#include <StackArray.h>
#include <RF24Network.h>
#include <RF24.h>
#include <SPI.h>

/*
  Liquid flow rate sensor -RubenD started at 10/1/16 16:38:50

  Measure the liquid/water flow rate using this code.
  Connect Vcc and Gnd of sensor to arduino, and the
  signal line to arduino digital pin 2.

*/

const int ledPin = 5;
byte sensorInterrupt = 0;
byte sensorPin       = 2;

RF24 radio(7, 8);                   // nRF24L01(+) radio attached using GettingStarted
RF24Network network(radio);          // Network uses that radio

const uint16_t this_node = 02;        // Address of our node in Octal format
const uint16_t other_node = 00;       // Address of the other node in Octal format


float calibrationFactor = 4.5;

volatile byte pulseCount;

float flowRate;
unsigned int flowMilliLitres;
unsigned long totalMilliLitres;

unsigned long oldTime;

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

const devices myDeviceId = waterFlow;

conn type;
devices devId;
int valueType;
int values[6];
const int ammount = 6;
int count;


void setup()
{

  // Initialize a serial connection for reporting values to the host
  //Serial.begin(9600);

  // Set up the status LED line as an output
  pinMode(ledPin, OUTPUT);
  //digitalWrite(ledPin, HIGH);  // We have an active-low LED attached

  pinMode(sensorPin, INPUT);
  digitalWrite(sensorPin, HIGH);

  pulseCount        = 0;
  flowRate          = 0.0;
  flowMilliLitres   = 0;
  totalMilliLitres  = 0;
  oldTime           = 0;

  
  radio.begin();
  network.begin( 85, this_node);

  digitalWrite(ledPin,LOW);
  //ledBlink(5,500);
  // The Hall-effect sensor is connected to pin 2 which uses interrupt 0.
  // Configured to trigger on a FALLING state change (transition from HIGH
  // state to LOW state)
  attachInterrupt(sensorInterrupt, pulseCounter, FALLING);
}





void loop()
{
  network.update();
  if (network.available()){
    readRF24();
    if (type == request){
      report();
    }
  }
  
  if ((millis() - oldTime) > 1000)   // Only process counters once per second
  {
    // Disable the interrupt while calculating flow rate and sending the value to
    // the host
    detachInterrupt(sensorInterrupt);

    flowRate = ((1000.0 / (millis() - oldTime)) * pulseCount) / calibrationFactor;

    oldTime = millis();

    flowMilliLitres = (flowRate / 60) * 1000;

    totalMilliLitres += flowMilliLitres;

    unsigned int frac;
    
    //Serial.print("Flow rate: ");
    //Serial.print(int(flowRate));  // Print the integer part of the variable
    //Serial.print(".");             // Print the decimal point

    frac = (flowRate - int(flowRate)) * 10;
    //Serial.print(frac, DEC) ;      // Print the fractional part of the variable
    //Serial.print("L/min");

    //Serial.print("  Current Liquid Flowing: ");             // Output separator
    //Serial.print(flowMilliLitres);
    //Serial.print("mL/Sec");

    //Serial.print("  Output Liquid Quantity: ");             // Output separator
    //Serial.print(totalMilliLitres);
    //Serial.println("mL");

    pulseCount = 0;

    // Enable the interrupt again now that we've finished sending output
    attachInterrupt(sensorInterrupt, pulseCounter, FALLING);
  }
}



void readRF24() {
  digitalWrite(ledPin, !(bitRead(PORTD, ledPin)));
  RF24NetworkHeader header;
  payload_flow payload;
  network.read(header, &payload, sizeof(payload));
  digitalWrite(ledPin, !(bitRead(PORTD, ledPin)));
  type = payload.type;
  devId = header.from_node;
  valueType = payload.valueType;
  values[0] = payload.value;
}

bool sendRF24(uint16_t whereTo) {
  digitalWrite(ledPin, !(bitRead(PORTD, ledPin)));
  payload_flow payload = { type, valueType, values[0]};
  RF24NetworkHeader header(whereTo);
  bool ok = network.write(header, &payload, sizeof(payload));
  digitalWrite(ledPin, !(bitRead(PORTD, ledPin)));
  return ok;
}

void report() {
  delay(10);
  type = info;
  values[0] = int(totalMilliLitres / 1000);
  sendRF24(0);
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

void pulseCounter()
{
  pulseCount++;
}
