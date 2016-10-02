/*
 * Displays text sent over the serial port (e.g. from the Serial Monitor) on
 * an attached LCD.
 */
#include <Wire.h> 
#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x27,16,2);  

void setup()
{
  lcd.init();                      
  lcd.backlight();
  Serial.begin(9600);
}

void loop()
{
  
  if (Serial.available()) {
    
    delay(100);
    
    lcd.clear();
    
    while (Serial.available() > 0) {
     
      lcd.write(Serial.read());
    }
  }
}
