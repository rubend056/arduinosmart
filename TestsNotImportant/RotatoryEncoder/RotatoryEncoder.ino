int a = 4;
int b = 3;
int c = 7;
bool stateC;
bool lastC = LOW;
int pos = 0;
bool al = LOW;
bool n = LOW; 


void setup()
{
  pinMode(c,INPUT);
  pinMode(a,INPUT);
  pinMode(b,INPUT);
  Serial.begin(115200);
}

void loop()
{
  stateC = digitalRead(c);
  if ((stateC == LOW) && (lastC == HIGH)){Serial.println("pressed");}else 
  if ((stateC == HIGH) && (lastC == LOW))Serial.println("released");
  n = digitalRead(a);
  
  
  if ((al == LOW) && (n == HIGH))
  {
    
    if (digitalRead(b) == LOW)
    {
      pos--;
    }
    else
    {
      pos++;
    }
  
  Serial.print ("EncoderPos: ");
  Serial.println (pos);
  }
  
  al = n;
  lastC = stateC;
  delay(1.5);
  
}
