#define rfTransmitPin 15  //RF Transmitter pin = digital pin 4
#define ledPin 13        //Onboard LED = digital pin 13

long writeDelay = 10000;
long startTime;
void setup() {
  pinMode(rfTransmitPin, OUTPUT);
  pinMode(ledPin, OUTPUT);
  startTime = micros();
  Serial.begin(9600);
}

void writeData(int data) {
  Serial.print("Sending data: ");
  Serial.println(data, DEC);
  
  for (int i = 0; i < 3; i++) {
    digitalWrite(rfTransmitPin, HIGH);
    digitalWrite(ledPin, HIGH);
    delayMicroseconds(writeDelay);
  }
  for (int i = 0; i < data; i++) {
    digitalWrite(rfTransmitPin, LOW);
    digitalWrite(ledPin, LOW);
    delayMicroseconds(writeDelay);

    digitalWrite(rfTransmitPin, HIGH);
    digitalWrite(ledPin, HIGH);
    delayMicroseconds(writeDelay);
  }
  for (int i = 0; i < 3; i++) {
    digitalWrite(rfTransmitPin, LOW);
    digitalWrite(ledPin, LOW);
    delayMicroseconds(writeDelay);
  }
}

void loop() {
  digitalWrite(rfTransmitPin, HIGH);
  digitalWrite(ledPin, HIGH);
  delayMicroseconds(writeDelay);

  digitalWrite(rfTransmitPin, LOW);
  digitalWrite(ledPin, LOW);
  delayMicroseconds(writeDelay);

  int readData = (analogRead(16) - 150) / 59;
  Serial.print("I received: ");
  Serial.println(readData, DEC);

  writeData(readData);
}
