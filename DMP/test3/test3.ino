const int pin1 = 14;
const int pin2 = 15;
const int pin3 = 16;

unsigned long phaseStartTime;
unsigned long longPhaseTime = 40000;
const unsigned long minLongPhaseTime = 20000;
unsigned long shortPhaseTime = longPhaseTime / 4;
unsigned long lastPhaseTimeChangeTime;
const unsigned long phaseTimeChangeInterval = 4000;
char phase;

void setup() {
  Serial.begin(9600);
  pinMode(pin1, OUTPUT);
  pinMode(pin2, OUTPUT);
  pinMode(pin3, OUTPUT);
  digitalWrite(pin2, LOW);
  digitalWrite(pin3, LOW);

  phase = 1;
  lastPhaseTimeChangeTime = phaseStartTime = micros();
  digitalWrite(pin1, HIGH);
}

void test() {  
  Serial.println("1");
  digitalWrite(pin1, HIGH);
  digitalWrite(pin2, LOW);
  digitalWrite(pin3, LOW);
  delay(1000);
  Serial.println("2");
  digitalWrite(pin1, LOW);
  digitalWrite(pin2, HIGH);
  digitalWrite(pin3, LOW);
  delay(1000);
  Serial.println("3");
  digitalWrite(pin1, LOW);
  digitalWrite(pin2, LOW);
  digitalWrite(pin3, HIGH);
  delay(1000);
}

void loop() {
  unsigned long currentTime = micros();
  switch (phase) {
    case 1:
      if (currentTime - phaseStartTime >= longPhaseTime) {
        Serial.println("Go!");
        phaseStartTime = currentTime;
        digitalWrite(pin2, HIGH);
        phase = 2;
      }
      break;
    case 2:
      if (currentTime - phaseStartTime >= shortPhaseTime) {
        phaseStartTime = currentTime;
        digitalWrite(pin1, LOW);
        phase = 3;
      }
      break;
    case 3:
      if (currentTime - phaseStartTime >= longPhaseTime) {
        phaseStartTime = currentTime;
        digitalWrite(pin3, HIGH);
        phase = 4;
      }
      break;
    case 4:
      if (currentTime - phaseStartTime >= shortPhaseTime) {
        phaseStartTime = currentTime;
        digitalWrite(pin2, LOW);
        phase = 5;
      }
      break;
    case 5:
      if (currentTime - phaseStartTime >= longPhaseTime) {
        phaseStartTime = currentTime;
        digitalWrite(pin1, HIGH);
        phase = 6;
      }
      break;
    case 6:
      if (currentTime - phaseStartTime >= shortPhaseTime) {
        phaseStartTime = currentTime;
        digitalWrite(pin3, LOW);
        phase = 1;
      }
      break;
  }

  if (longPhaseTime > minLongPhaseTime && currentTime - lastPhaseTimeChangeTime >= phaseTimeChangeInterval) {
    lastPhaseTimeChangeTime = currentTime;
    longPhaseTime -= 40;
    shortPhaseTime -= 10;
  }
}
