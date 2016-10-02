String test = "32,21";

enum conn {
  set,
  request,
  info
};

conn type;
int devId;
int valueType;
int value;
int value1;
int value2;
int value3;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  int hey = decipher(test);
  Serial.println(test);
  Serial.println(rebuild());
  Serial.println(hey);
}

void loop() {
  // put your main code here, to run repeatedly:
  
}

int decipher(String toDec) {
  String output;
  int count = 1;
  int i = 0;
  do {
    output = "";
    do {
      output += toDec[i];
      i++;
    } while ((i < toDec.length()) and !(toDec[i]==','));

    int val = output.toInt();
    if (count == 1)type = val;
    if (count == 2)devId = val;
    if (count == 3)valueType = val;
    if (count == 4)value = val;
    if (count == 5)value1 = val;
    if (count == 6)value2 = val;
    if (count == 7)value3 = val;
    
    count++;
    i++;

  } while (i < toDec.length());
  count--;
  return count;

}

String rebuild () {
  String here = "";

  here += String(type) + ',';
  here += String(devId) + ',';
  here += String(valueType) + ',';
  here += String(value) + ',';
  here += String(value1) + ',';
  here += String(value2) + ',';
  here += String(value3);

  return here;
}
