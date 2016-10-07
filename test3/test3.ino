int desTemp = 3;
int average = 4;

const int fanPin = 5;
const int coolPin = 6;
bool fanAuto = HIGH;
int c = 10;

unsigned long fanTime;
unsigned long acTimer;
unsigned int waitAfterOn = 120;
unsigned int waitAfterOff = 20;

void setup() {
  Serial.begin(9600);



  fanTime = millis();
  acTimer = millis();
  Serial.println("Begin!!");
}

void loop() {

  if (millis() >= acTimer + 4000) {
    acController();
    acTimer = millis();
    Serial.println(String(average) + "  " + String(desTemp));
  }



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
        if (after(fanTime, 2))digitalWrite(coolPin, HIGH);
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

  /*if (checkFan() == LOW) {
    if (fanAuto == HIGH) {
      if ((shouldBe() == HIGH) && after(fanTime,waitAfterOff)) {
        digitalWrite(fanPin, HIGH);
        Serial.println()
        fanTime = millis();
      }
    } else digitalWrite(fanPin, HIGH);
    } else if ((fanAuto == HIGH) && (shouldBe() == LOW) && after(fanTime, waitAfterOn)) {
    digitalWrite(fanPin, LOW);
    }

    if ((checkFan() == HIGH)  && (checkCool() == LOW) && (shouldBe() == HIGH) &&  after(fanTime, 5)) {
    digitalWrite(coolPin, HIGH);
    fanTime = millis();
    //coolingTime = millis();
    } else if ((shouldBe() == LOW) && (checkCool() == HIGH) && after(fanTime, waitAfterOn)) {
    digitalWrite(coolPin, LOW);
    fanTime = millis();
    }*/

}

void serialEvent() {
  char t = Serial.read();
  switch (t) {
    case '\n':
      Serial.println("GotIt!");
      break;
    case '1':
      c = 1;
      break;
    case '2':
      c = 2;
      break;
    default:
      if ((int)t > 2) {
        if (c == 1)average = (int)t - 48;
        if (c == 2)desTemp = (int)t - 48;
      }
      c = 10;
      break;
  }

}

