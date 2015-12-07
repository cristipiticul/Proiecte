const int PIN_1 = 14;
const int PIN_2 = 15;
const int PIN_3 = 16;
unsigned long stepDuration =80000; // microseconds
unsigned long stepStartTime;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  pinMode(PIN_1, OUTPUT);
  pinMode(PIN_2, OUTPUT);
  pinMode(PIN_3, OUTPUT);
  stepStartTime = micros();
}

void loop(){
  switchStep();
}

void switchStep(){
  unsigned long currentTime = micros();
  while (currentTime - stepStartTime >= stepDuration) {
    stepStartTime += stepDuration;
  }
  int stage = ((currentTime - stepStartTime) * 6 / stepDuration) + 1;
  switch(stage){
    case 1:
      digitalWrite(PIN_1, HIGH);
      digitalWrite(PIN_2, LOW);
      digitalWrite(PIN_3, LOW);
      break;
    case 2:
      digitalWrite(PIN_1, HIGH);
      digitalWrite(PIN_2, HIGH);
      digitalWrite(PIN_3, LOW);
      break;
    case 3:
      digitalWrite(PIN_1, LOW);
      digitalWrite(PIN_2, HIGH);
      digitalWrite(PIN_3, LOW);
      break;
    case 4:
      digitalWrite(PIN_1, LOW);
      digitalWrite(PIN_2, HIGH);
      digitalWrite(PIN_3, HIGH);
      break;
    case 5:
      digitalWrite(PIN_1, LOW);
      digitalWrite(PIN_2, LOW);
      digitalWrite(PIN_3, HIGH);
      break;
    case 6:
      digitalWrite(PIN_1, HIGH);
      digitalWrite(PIN_2, LOW);
      digitalWrite(PIN_3, HIGH);
      break;
  }
}
