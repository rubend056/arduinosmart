#include <RF24Network.h>
#include <RF24.h>
#include <SPI.h>
String inputString = "";  // whether the string is complete

const int ledPin = 5;

RF24 radio(7, 8);                   // nRF24L01(+) radio attached using Getting Started board
RF24Network network(radio);          // Network uses that radio

const uint16_t this_node = 00;        // Address of our node in Octal format


struct payload_thermostat {                  // Structure of our payload
  int type;
  int valueType;
  int value;
  int value1;
  int value2;
  int value3;
  int value4;
  int value5;
};

struct payload_flow {          // Structure of our payload
  int type;
  int valueType;
  int value;
};

enum conn {
  set,
  request,
  info,
  identify
};

enum devices {
  base,
  thermostat,
  waterFlow
};

conn type;
devices devId;
int valueType;
int values[6];
const int ammount = 6;
int count;


void setup() {
  Serial.begin(9600);

  pinMode(ledPin, OUTPUT);
  SPI.begin();
  radio.begin();
  network.begin( 85, this_node);
  digitalWrite(ledPin, LOW);
  ledBlink(10, 1000);
}

void loop() {

  if (serial()) {
    bool c = sendRF24(devId);
    if (c)digitalWrite(ledPin, LOW); else digitalWrite(ledPin, HIGH);
    if ((type == request) and (c)) {

      c = waitRF24(500);
      if (!(c)) {
        Serial.println("none");
      }
    }
  }


  
  network.update();                          // Check the network regularly
  if (network.available()) {
    readRF24();
    if (type == info) {
      Serial.println(rebuild());
    }
  }
}

void readRF24() {
  digitalWrite(ledPin, !(bitRead(PORTD, ledPin)));
  RF24NetworkHeader header;
  network.peek(header);
  devId = header.from_node;
  
  if(devId == thermostat){
      payload_thermostat payload;
      network.read(header, &payload, sizeof(payload));
      int payloadValues[] = {payload.value, payload.value1,payload.value2, payload.value3, payload.value4, payload.value5};
      for (int i = 0; i < ammount; i++){
        values[i] = payloadValues[i];
      }
      type = payload.type;
      valueType = payload.valueType;
  } else if (devId == waterFlow){
      payload_flow payload;
      network.read(header, &payload, sizeof(payload));
      values[0] = payload.value;
      for (int i = 1; i < ammount; i++){
        values[i] = 0;
      }
      type = payload.type;
      valueType = payload.valueType;
  }
  
  digitalWrite(ledPin, !(bitRead(PORTD, ledPin)));
}

bool sendRF24(uint16_t whereTo) {
  digitalWrite(ledPin, !(bitRead(PORTD, ledPin)));
  bool ok;
  RF24NetworkHeader header(whereTo);
  
  if (whereTo == waterFlow){
    payload_flow payload = { type, valueType , values[0]};
    ok = network.write(header, &payload, sizeof(payload));
  } else if (whereTo == thermostat){
    payload_thermostat payload = { type, valueType , values[0], 0, 0, 0, 0, 0};
    ok = network.write(header, &payload, sizeof(payload));
  }
  
  digitalWrite(ledPin, !(bitRead(PORTD, ledPin)));
  return ok;
}

void ledBlink(int count, int del) {
  del = (del*1000)/(count*2);
  for (int i = 0; i < count; i++) {
    digitalWrite(ledPin, !(bitRead(PORTD, ledPin)));
    delayMicroseconds(del);
    digitalWrite(ledPin, !(bitRead(PORTD, ledPin)));
    delayMicroseconds(del);
  }
}

String rebuild () {
  String here = "";

  here += String(type) + ',';
  here += String(devId) + ',';
  here += String(valueType) + ',';
  
  for (int i = 0; i<ammount;i++){
    here += String(values[i]) + ',';
  }
  here.remove(here.length()-1);

  return here;
}

bool serial() {
  bool hey = false;
  if (Serial.available()) {
    char y = (char)Serial.read();
    if (y == '\n') {
      count = decipher(inputString);
      inputString = "";
      hey = true;
    } else {
      inputString += y;
    }
  }
  return hey;
}

int decipher(String toDec) {
  String output;
  int c = 1;
  int i = 0;
  do {
    output = "";
    do {
      output += toDec[i];
      i++;
    } while ((i < toDec.length()) and !(toDec[i] == ','));

    int val = output.toInt();
    if (c == 1)type = val;
    if (c == 2)devId = val;
    if (c == 3)valueType = val;
    if (c >= 4)values[c-4] = val;

    c++;
    i++;

  } while (i < toDec.length());
  c--;
  return c;

}

bool waitSerial(int wait) {
  bool c = true;
  unsigned long responseTime = millis();
  while (!(serial())) {
    if (responseTime + wait < millis()) {
      c = false;
      break;
    }
  }
  return c;
}

bool waitRF24(int wait) {
  bool c = true;
  unsigned long responseTime = millis();
  while (!(network.available())) {
    network.update();
    if (responseTime + wait < millis()) {
      c = false;
      break;
    }
  }
  return c;
}
