String test = "5";

int storage[5][4] = {0};  // a storage to hold data
int ammount = 4; 

String inputString = "";

enum conn {
  set,
  request,
  info,
  requestOld
};

conn type;
int devId;
int valueType;
int value;
int value1;
int value2;
int value3;
int count;

int minimum = 13;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  int c = decipher(test);
  Serial.println(rebuild());
  Serial.println(c);
}

void loop() {
  // put your main code here, to run repeatedly:
  String toSend;
  bool c = true;
  c = waitSerial(2000);

  if ((c) and (count == 7)) {
    updateStorage();      // reads values and puts them in appropiate storage position
    toSend = getStorage(devId);    //equals string to appropiate device values from storage
  } else {
    toSend = "not available";
  }
  toSend += '\n';
  
  Serial.print(toSend);
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
    if (c == 4)value = val;
    if (c == 5)value1 = val;
    if (c == 6)value2 = val;
    if (c == 7)value3 = val;

    c++;
    i++;

  } while (i < toDec.length());
  c--;
  return c;

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
  if (type == info) {
    int i[] = {value, value1, value2, value3};
    for (int r = 0; r < 4; r++) {
      storage[devId][r] = i[r];
    }
  }
}
