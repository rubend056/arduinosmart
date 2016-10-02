/*
** Example Arduino sketch for SainSmart I2C LCD Screen 16x2
 ** based on https://bitbucket.org/celem/sainsmart-i2c-lcd/src/3adf8e0d2443/sainlcdtest.ino
 ** by
 ** Edward Comer
 ** LICENSE: GNU General Public License, version 3 (GPL-3.0)
 
 ** This example uses F Malpartida's NewLiquidCrystal library. Obtain from:
 ** https://bitbucket.org/fmalpartida/new-liquidcrystal
 
 ** Modified - Ian Brennan ianbren at hotmail.com 23-10-2012 to support Tutorial posted to Arduino.cc
 
 ** Written for and tested with Arduino 1.0
 **
 ** NOTE: Tested on Arduino Uno whose I2C pins are A4==SDA, A5==SCL
 
 */
#include <Wire.h>
#include <LCD.h>
#include <LiquidCrystal_I2C.h>

#define I2C_ADDR    0x27 // <<----- Add your address here.  Find it from I2C Scanner
#define BACKLIGHT_PIN     3
#define En_pin  2
#define Rw_pin  1
#define Rs_pin  0
#define D4_pin  4
#define D5_pin  5
#define D6_pin  6
#define D7_pin  7

long n = 0;
int ledPin = 13;
String inputString;
int check = 1;

LiquidCrystal_I2C	lcd(I2C_ADDR,En_pin,Rw_pin,Rs_pin,D4_pin,D5_pin,D6_pin,D7_pin);
String inString = "";
void setup()
{
  lcd.begin (16,2); //  <<----- My LCD was 16x2
  Serial.begin(9600);
  pinMode(ledPin,INPUT);
  // Switch on the backlight
  lcd.setBacklightPin(BACKLIGHT_PIN,POSITIVE); 
  lcd.setBacklight(HIGH);

  inputString.reserve(300);

  lcd.home (); // go home
  lcd.clear(); 
  lcd.print("hey there");

}

void loop()
{
  // Backlight on/off every 3 seconds
  /*lcd.setCursor (0,1);        // go to start of 2nd line
   lcd.print(n++,DEC);*/
  //delay(50);
  /*lcd.setBacklight(LOW);      // Backlight off
   delay(3000);
   lcd.setBacklight(HIGH);     // Backlight on
   delay(3000);*/
  switch (check) {
  case 1:
    Serial.println("name");
    break;
  case 2:
    Serial.println("altitude");
    break;

  }
  n = millis();
  wait();
  if (Serial.available()>0)
  {
    serialCommand();
    if (check == 1){
      lcdClear(0,0,15,0);
      lcd.setCursor(0,0); 
      lcd.print(inString);
    }
    if (check == 2){
      lcdClear(0,1,15,1);
      lcd.setCursor(0,1); 
      lcd.print(inString);
    }


  }


  else{
    lcd.clear();
    lcd.home();
    lcd.print("no input");
  }




  if (check == 1)check = 2;
  else check =1;
  /*if (stringComplete) {
   Serial.println(inputString); 
   // clear the string:
   inputString = "";
   stringComplete = false;
   }*/
}


void serialCommand(){
  //Serial.println(Serial.available());
  inString = "";

  while(Serial.available()>0){
    inString += (char)Serial.read();
  }



}
void wait(){

  while(Serial.available()==0)
  {
    if ((millis()-n)>5000)break;
  }
  ledBlink();
}

void ledBlink(){
  digitalWrite(ledPin,HIGH);
  delay(25);
  digitalWrite(ledPin,LOW);
}
/*void serialEvent() {
 while (Serial.available()) {
 // get the new byte:
 char inChar = (char)Serial.read(); 
 // add it to the inputString:
 inputString += inChar;
 // if the incoming character is a newline, set a flag
 // so the main loop can do something about it:
 if (inChar == '\n') {
 stringComplete = true;
 } 
 }
 }*/

void lcdClear(int from1,int from2,int to1,int to2)
{
  int e;
  int f;
  int rem = abs((from1 + (from2*16)) - (to1 + (to2*16)))+1;
  for (int i = 0; i<rem; i++)
  {

    if (from2 == 0)
    {
      if (from1+i > 15)
      {
        e = (i+from1)-16;
        f = 1;
      }
      else
      {
        e = i+from1;
        f = 0;
      }
    }
    else
    {
      e = i+from1;
      f = 1;
    }

    lcd.setCursor(e,f);
    lcd.print(" ");

  }

}


