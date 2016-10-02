#include <UIPEthernet.h>

EthernetUDP udp;
int myPort = 5000;

String received = "";
String inputString = "";

int ledPin = 5;

int storage[5][6];  // a storage to hold data
const int ammount = 6; //change this when you change ammount of storage;
int values[6];

enum conn {
  set,
  request,
  info,
  requestOld
};

conn type = 0;
int devId = 0;
int valueType = 0;


int count;

int minimum = 14;

void setup() {

  Serial.begin(9600);

  uint8_t mac[6] = {0x00, 0x01, 0x02, 0x03, 0x04, 0x05};
  IPAddress ip = {10, 0, 0, 147};
  pinMode(ledPin, OUTPUT);
  Ethernet.begin(mac, ip);

  received.reserve(30);
  udp.begin(myPort);
  digitalWrite(ledPin, HIGH);
}

void loop() {
  int toCheck = ammount + 3;
  
  if (serial()) {
    if (count == toCheck)updateStorage();
  }

  int size = udp.parsePacket();
  if (size > 0) {
    String toSend;
    readPacket(size);     //read ethernet packet and output to string received
    count = decipher(received);   //convert string to set values
    ledBlink(count, 250);  //blink LED
    //Serial.println(count);//*d**********

    if (type == set) {
      Serial.println(rebuild());
    } else if (type == request) {
      Serial.println(rebuild());
      bool c = true;
      c = waitSerial(1100);
      //Serial.println(count);//**************
      if ((c) and (count == toCheck)) {
        updateStorage();      // reads values and puts them in appropiate storage position
        toSend = getStorage(devId);    //equals string to appropiate device values from storage
      } else {
        toSend = "not available";
      }
      toSend += '\n';
      sendPacket(toSend, udp.remoteIP(), myPort);
    } else if (type == requestOld) {
      //Serial.println(received);//***********
      //Serial.println(devId);//***********
      //Serial.println(count);
      toSend = getStorage(devId);
      toSend += '\n';
      sendPacket(toSend, udp.remoteIP(), myPort);
    }

    udp.stop();
    udp.begin(myPort);
  }
}

void readPacket(int size) {
  digitalWrite(ledPin, !(bitRead(PORTD, ledPin)));
  received = "";
  do
  {
    char* msg = (char*)malloc(size + 1);
    int len = udp.read(msg, size + 1);
    msg[len] = 0;
    received += msg;
    free(msg);
  }
  while ((size = udp.available()) > 0);

  udp.flush();
  //Serial.println("'");

  received.trim();
  digitalWrite(ledPin, !(bitRead(PORTD, ledPin)));
}

bool sendPacket(String text, IPAddress ip, int port) {
  //Serial.println(ip);
  bool success = true;
  do
  {
    if (!udp.beginPacket(ip, port)) success = false;
  }
  while (!success);

  if (!udp.print(text)) success = false;
  if (!udp.endPacket()) success = false;

  return success;
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

void ledBlink(int count, int del) {
  del = (del * 1000) / (count * 2);
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

String getStorage(int id) {
  String toSend = "";
  for (int i = 0; i < ammount; i++) {
    toSend += String(storage[id][i]);
    toSend += ',';
  }
  toSend.remove(toSend.length() - 1);
  return toSend;
}

void updateStorage() {
  for (int r = 0; r < ammount; r++) {
    storage[devId][r] = values[r];
  }
}

bool waitSerial(int waitTime) {
  bool c = true;
  unsigned long responseTime = millis();
  while (!(serial())) {
    if (responseTime + waitTime < millis()) {
      c = false;
      break;
    }
  }
  return c;
}

