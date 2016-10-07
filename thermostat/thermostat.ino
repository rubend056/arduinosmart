#include <RF24Network.h>
#include <RF24.h>
#include <EEPROM.h>
#include <SPI.h>
#include <Wire.h>
#include <dht.h>
#include <LiquidCrystal_I2C.h>


enum SCREEN {Menu, Welcome, Settings, Test, lowerB, upperB};
LiquidCrystal_I2C lcd(0x27, 16, 2);
dht DHT;
SCREEN scr = Menu;

unsigned int waitAfterOn = 720;
unsigned int waitAfterOff = 360;


uint8_t on[8]  = {0x0, 0xe, 0x11, 0x15, 0x15, 0x11, 0xe, 0x0};
uint8_t off[8]  = {0x0, 0xe, 0x11, 0x11, 0x11, 0x11, 0xe, 0x0};
byte connOn[8] = {
  B00000,
  B00100,
  B01100,
  B11111,
  B01100,
  B00100,
  B00000,
};
byte straight[8] = {
  B00000,
  B00000,
  B00000,
  B11111,
  B00000,
  B00000,
  B00000,
};
byte connOff[8] = {
  B00000,
  B00000,
  B10000,
  B10001,
  B10000,
  B00000,
  B00000,
};


#define coolPin 6
#define fanPin 5
#define backPin 9

#define APin 3
#define BPin 4
#define buttonPin 2
#define dhtPin A0
#define lightPin A1



int upperBack = 120, lowerBack = 25;
int back = upperBack;
unsigned long upTimer;
unsigned long downTimer;



int desTemp, lastDesTemp = 0;
unsigned long tempTime;
int temp, lastTemp = 0;
bool button = HIGH, lastButton = HIGH;
int tempA[30];
int average, lastAverage = 0;
int count = 0;
int hum, lastHum = 0;
char tempValue = 'C';
bool al, bl;
unsigned long bTimer;
bool fanAuto = HIGH, showHum = HIGH, autoBack = HIGH;
unsigned long backTime;
unsigned long fanTime;
unsigned long acTimer;
unsigned long coolingTime;
//unsigned long iconTime;
//int iconCount = 10;
int sPos = 1;
int scroll = 1;
int encoderLast = 0;
bool backCheck, connectionl, connection = LOW;


RF24 radio(7, 8);                   // nRF24L01(+) radio attached using Getting Started board
RF24Network network(radio);          // Network uses that radio
const uint16_t this_node = 01;        // Address of our node in Octal format
const uint16_t parent_node = 00;       // Address of the other node in Octal format
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
enum conn {
  set,
  request,
  info
};
enum valType {
  desiredTemperature,
  temperatureValue,
  automaticFan,
  upTime,
  downTime
};

conn type;
int devId;
valType valueType;
int values[6] = {0};
const int ammount = 6;

void setup() {

  lcd.init();
  lcd.backlight();


  lcd.createChar(0, off);
  lcd.createChar(1, on);
  lcd.createChar(2, connOn);
  lcd.createChar(3, connOff);
  lcd.createChar(4, straight);

  pinMode(coolPin, OUTPUT);
  pinMode(fanPin, OUTPUT);
  pinMode (backPin, OUTPUT);

  pinMode (APin, INPUT);
  pinMode (BPin, INPUT);
  pinMode (buttonPin, INPUT);
  pinMode (dhtPin, INPUT);
  pinMode (lightPin, INPUT);

  analogWrite(backPin, back);
  backTime = millis();

  bTimer = millis();
  upTimer = millis();
  downTimer = millis();
  scr = Welcome;
  lcdWrite(scr);
  delay(1000);
  scr = Menu;
  fanTime = millis();
  acTimer = millis();  

  DHT.read11(dhtPin);
  temp = readTemp(tempValue);
  hum = readHum();
  int newTemp = readTemp(tempValue);
  desTemp = newTemp + 1;
  for (int i = 0; i < 29; i++)tempA[i] = newTemp;
  average = tempAverage();
  tempTime = millis();
  
  //EEPROM implementation
  int r; 
  EEPROM.get(0,r);
  if((r > 0) && (r<200))desTemp = r;
  
  lcd.clear();
  lcdWrite(scr);

  SPI.begin();
  radio.begin();
  network.begin(/*channel*/ 85, /*node address*/ this_node);

  report();
}

void loop() {
  network.update();
  if (network.available()) {
    readRF24();
    if (type == set) {
      switch (valueType){
        case desiredTemperature:
          if ((values[0] <= temp + 10) and (values[0] >= temp - 10))desTemp = values[0];
          if (desTemp >= 100)lcd.setCursor(13, 0); else lcd.setCursor(14, 0);
          lcd.print(desTemp);
          break;
        case temperatureValue:
          if (values[0] == 1)tempValue = 'F'; else tempValue = 'C';
          setTemp();
          lcd.setCursor(15, 1);
          lcd.print(tempValue);
          break;
        case automaticFan:
          fanAuto = (bool)values[0];
          break;
        case upTime:
          if(values[0]>=0)waitAfterOn = values[0];
          break;
        case downTime:
          if(values[0]>=0)waitAfterOff = values[0];
          break;
      }
    }
    report();
  }

  backControl();



  if (millis() > tempTime + 2000)
  {
    DHT.read11(dhtPin);
    tempA[count] = readTemp(tempValue);
    if (count < 29)count++; else count = 0;
    tempTime = millis();
    temp = readTemp(tempValue);
    hum = readHum();
    average = tempAverage();
    //ledBlink();

  }
  if (millis() > 1500)button = digitalRead(buttonPin);
  updateButton();
  updateEncoder();
  if (millis() >= acTimer + 2000){
    acController();
    acTimer = millis();
  }
  menuUpdate();

  connectionl = connection;
  lastAverage = average;
  lastDesTemp = desTemp;
  lastTemp = temp;
  lastHum = hum;
  lastButton = button;
}

void menuUpdate() {
  if (scr == Menu) {
    if ((hum != lastHum) and (showHum)) {
      lcd.setCursor(0, 1);
      lcd.print(hum);
      lcd.print('%');
    }
    if (desTemp != lastDesTemp) {
      if (desTemp >= 100)lcd.setCursor(13, 0); else lcd.setCursor(14, 0);
      lcd.print(desTemp);
    }
    if (average != lastAverage) {
      lcd.setCursor(0, 0);
      lcd.print(average);
    }
    if (connection != connectionl) {
      lcd.setCursor(11, 1);
      if (connection)lcd.write(2); else lcd.write(3);
    }
  }
}

void readRF24() {
  lcd.setCursor(5, 1);
  lcd.print("X");
  RF24NetworkHeader header;
  payload_thermostat payload;
  network.read(header, &payload, sizeof(payload));

  type = payload.type;
  devId = header.from_node;
  valueType = payload.valueType;
  int payloadValues[] {payload.value, payload.value1,
                       payload.value2, payload.value3, payload.value4, payload.value5
                      };
  lcd.setCursor(5, 1);
  lcd.print(" ");

  for (int i = 0; i < ammount; i++) {
    values[i] = payloadValues[i];
  }

}

void updateValues() {
  values[0] = temp;
  values[1] = desTemp;
  values[2] = hum;
  if (fanAuto == HIGH)values[3] = 1; else values[3] = 0;
  if (tempValue == 'F')values[4] = 1; else values[4] = 0;
}

bool sendRF24(uint16_t whereTo) {
  lcd.setCursor(6, 1);
  lcd.print("X");
  payload_thermostat payload = { type, valueType , values[0], values[1], values[2], values[3], values[4], values[5]};
  RF24NetworkHeader header(whereTo);
  bool ok = network.write(header, &payload, sizeof(payload));

  delay(150);
  
  lcd.setCursor(6, 1);
  lcd.print(" ");
  
  return ok;
}

void backControl() {
  if (millis() - backTime > 10000) {
    if (back > lowerBack)back--;
    if (scr != Menu)scr = Menu;
    if (backCheck) {
      backCheck = LOW;
      EEPROM.put(0,desTemp);
      lcdWrite(scr);
    }
  } else {
    if (back < upperBack)back++; else if (back > upperBack)back--;
    backCheck = HIGH;
  }
  analogWrite(backPin, back);
}

void lcdWrite (int value) {
  lcd.clear();
  int p, page = ((sPos - 1) / 2) + 1;

  switch (value) {
    case Menu:
      lcd.setCursor(0, 0);
      lcd.print(average);
      //lcd.print("  ");
      if (desTemp >= 100)lcd.setCursor(13, 0); else lcd.setCursor(14, 0);
      lcd.print(desTemp);

      if (showHum == HIGH) {
        lcd.setCursor(0, 1);
        lcd.print(hum);
        lcd.print('%');
      }

      lcd.setCursor(10, 1);
      lcd.write(4);
      lcd.setCursor(11, 1);
      if (connection)lcd.write(2); else lcd.write(3);
      lcd.setCursor(12, 1);
      lcd.write(4);

      lcd.setCursor(15, 1);
      lcd.print(tempValue);
      break;
    case Welcome:
      lcd.setCursor(3, 0);
      lcd.print("Welcome!");
      break;
    case Settings:
      if (sPos % 2 == 0)p = 1; else p = 0;
      lcd.setCursor(15, 0);
      lcd.print(page);
      lcd.setCursor(0, p);
      lcd.write('>');
      switch (page) {
        case 1:
          lcd.setCursor(1, 0);
          lcd.print("TUnit");
          lcd.setCursor(8, 0);
          lcd.print(tempValue);
          lcd.setCursor(1, 1);
          lcd.print("ShowHum");
          lcd.setCursor(9, 1);
          if (showHum)lcd.write(1); else lcd.write(0);
          break;
        case 2:
          lcd.setCursor(1, 0);
          lcd.print("fanAuto");
          lcd.setCursor(9, 0);
          if (fanAuto)lcd.write(1); else lcd.write(0);
          lcd.setCursor(1, 1);
          lcd.print("autoBack");
          lcd.setCursor(9, 1);
          if (autoBack)lcd.write(1); else lcd.write(0);
          break;
        case 3:
          lcd.setCursor(1, 0);
          lcd.print("lowerBack");
          if (lowerBack >= 100)lcd.setCursor(11, 0); else lcd.setCursor(12, 0);
          lcd.print(lowerBack);
          lcd.setCursor(1, 1);
          lcd.print("upperBack");
          if (upperBack >= 100)lcd.setCursor(11, 1); else lcd.setCursor(12, 1);
          lcd.print(upperBack);
          break;
        case 4:
          lcd.setCursor(1, 0);
          lcd.print("NONE");
          lcd.setCursor(1, 1);
          lcd.print("Exit");
      }
      break;
    case lowerB:
      lcd.setCursor(1, 0);
      lcd.print("lowerBack");
      if (lowerBack >= 100)lcd.setCursor(13, 0); else lcd.setCursor(14, 0);
      lcd.print(lowerBack);
      break;
    case upperB:
      lcd.setCursor(1, 0);
      lcd.print("upperBack");
      if (upperBack >= 100)lcd.setCursor(13, 0); else lcd.setCursor(14, 0);
      lcd.print(upperBack);
      break;

  }



}

void updateEncoder() {
  int a = digitalRead(APin);
  int b = digitalRead(BPin);
  bool update = LOW;

  if ((al == LOW) && (a == HIGH)) {
    if (back == upperBack) {
      if (b == LOW) {
        if (!(bl == HIGH) and (millis() < bTimer + 400)) {
          switch (scr) {
            case Menu:
              if (average - 10 < desTemp)desTemp--;
              break;
            case Settings:
              if (sPos > 1)sPos--; else sPos = 8;
              break;
            case lowerB:
              lowerBack = down(lowerBack, 0);
              break;
            case upperB:
              upperBack = down(upperBack, lowerBack + 10);
              break;
          }
        }
      } else {
        if (!(bl == LOW) and (millis() < bTimer + 400)) {
          switch (scr) {
            case Menu:
              if (average + 10 > desTemp)desTemp++;
              break;
            case Settings:
              if (sPos < 8)sPos++; else sPos = 1;
              break;
            case lowerB:
              lowerBack = up(lowerBack, 245);
              break;
            case upperB:
              upperBack = up(upperBack, 255);
              break;
          }
        }
      }
    }
    if (scr != Menu) update = HIGH;
    backTime = millis();
    if (update) lcdWrite(scr);
  }


  bTimer = millis();
  al = a;
  bl = b;
}

int readTemp(char value) {

  int t = DHT.temperature;
  if (value == 'F')t = conversor(t, 'C', 'F');
  return t;
}

int readHum() {
  return DHT.humidity;
}

void updateButton() {
  if ((lastButton == LOW) && (button == HIGH)) {
    if (back == upperBack) {
      switch (scr) {
        case Menu:
          scr = Settings;
          sPos = 1;
          break;
        case Settings:
          switch (sPos) {
            case 1:
              if (tempValue == 'F')tempValue = 'C'; else tempValue = 'F';
              setTemp();
              break;
            case 2:
              showHum = !showHum;
              lcd.setCursor(9, 1);
              if (showHum)lcd.write(1); else lcd.write(0);
              break;
            case 3:
              fanAuto = !fanAuto;
              lcd.setCursor(9, 0);
              if (fanAuto)lcd.write(1); else lcd.write(0);
              break;
            case 4:
              autoBack = !autoBack;
              lcd.setCursor(9, 1);
              if (autoBack)lcd.write(1); else lcd.write(0);
              break;
            case 5:
              scr = lowerB;
              break;
            case 6:
              scr = upperB;
              break;
            case 7:
              break;
            case 8:
              scr = Menu;
              break;
          }
          break;
        case lowerB:
          scr = Settings;
          break;
        case upperB:
          scr = Settings;
          break;
      }
    }
    backTime = millis();
    /*if (!(scr == settings) and !((sPos == 2) or (sPos == 3) or (sPos == 4)))*/lcdWrite(scr);
  }
}

int tempAverage() {
  int f = 0;
  for (int i = -1; i < 29; i++)f += tempA[i];
  if (f % 30 >= 3)f = (f / 30) + 1; else f = f / 30;
  return f;
}

int conversor(int value, char temp, char otherTemp) {
  int output = value;
  if ((temp == 'C') && (otherTemp == 'F')) {
    output = output * 1.8;
    output += 32;
  }
  if ((temp == 'F') && (otherTemp == 'C')) {
    output -= 32;
    output = output / 1.8;
  }
  return output;
}

bool checkFan() {
  return bitRead(PORTD, fanPin);
}
bool checkCool() {
  return bitRead(PORTD, coolPin);
}
bool shouldBe() {
  if (average > desTemp)return HIGH; else return LOW;
}
bool after(unsigned long tim, unsigned int howLong) {
  bool toReturn = (millis() >= (tim + (howLong * 1000)));
  return toReturn;
}


void acController() {

  if (fanAuto == HIGH) {
    if (checkFan()) {
      if (shouldBe()) {
        if (after(fanTime, 10))digitalWrite(coolPin, HIGH);
      } else {
        if (after(fanTime, waitAfterOn)) {
          digitalWrite(fanPin, LOW);
          digitalWrite(coolPin, LOW);
          fanTime = millis();
          Serial.println("LOW");
        }
      }
    } else {
      if(after(fanTime,waitAfterOff)){
        if(shouldBe()){
          digitalWrite(fanPin,HIGH);
          fanTime = millis();
        }
      }
    }
  } else {
    digitalWrite(fanPin, HIGH);
    if (checkCool() == HIGH) {
      if (after(fanTime, waitAfterOn) == HIGH) {
        if(shouldBe() == LOW){
          digitalWrite(coolPin,LOW);
          fanTime = millis();
        }
        
      }
    } else {
      if (after(fanTime, waitAfterOff) == HIGH) {
        if(shouldBe()){
          digitalWrite(coolPin,HIGH);
          fanTime = millis();
        }
      }
    }
  }
}

void setTemp() {
  int newTemp = readTemp(tempValue);
  desTemp = newTemp;
  for (int i = 0; i < 29; i++)tempA[i] = newTemp;
  average = tempAverage();
}

int up(int value, int limit) {

  if (millis() <= (upTimer + 290)) {
    if (value + 10 < limit)value += 10; else value = limit;
  } else {
    if (value + 1 <= limit)value += 1;
  }
  upTimer = millis();
  return value;
}

int down(int value, int limit) {

  if (millis() <= (downTimer + 290)) {
    if (value - 10 > limit)value -= 10; else value = limit;
  } else {
    if (value - 1 >= limit)value -= 1;
  }
  downTimer = millis();
  return value;
}

void report() {
  delay(10);
  type = info;
  updateValues();
  connection = (sendRF24(parent_node));
}

