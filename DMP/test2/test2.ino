/**
 * BUGGISH after some time. Output should be:
 * 1 start
 * 3 end
 * 2 start
 * 1 end
 * 3 start
 * 2 end
 */

const int phase1pin = 14;
const int phase2pin = 15;
const int phase3pin = 16;
float holdTime = 50000; // microsecs
const unsigned long minHoldTime = 1300;
 
unsigned long p1start,
              p1end,
              p2start,
              p2end,
              p3start,
              p3end;
 
void setup(){
  pinMode(phase1pin, OUTPUT);
  pinMode(phase2pin, OUTPUT);
  pinMode(phase3pin, OUTPUT);
  p1start = micros();
  digitalWrite(phase1pin, HIGH);
  Serial.begin(9600);
}
 
 
void chkP1(){
  unsigned long currentTime = micros();
  unsigned long td = currentTime - p1start;
  unsigned long refractory = 2.25*holdTime;
  if(p1start > p1end){
    if(td > holdTime){
      Serial.println("1 end");
      digitalWrite(phase1pin, LOW);
      p1end = currentTime;
    }
  }else if(td > refractory){
    Serial.println("1 start");
    digitalWrite(phase1pin, HIGH);
    p1start = currentTime;
  }
}
 
void chkP2(){
  unsigned long currentTime = micros();
  unsigned long td = currentTime - p1start;
  if(p2start > p2end){
    if(td > 1.75*holdTime || td < 0.75*holdTime){
      Serial.println("2 end");
      digitalWrite(phase2pin, LOW);
      p2end = currentTime;
    }
  }else if(td > 0.75*holdTime && td < 1.75*holdTime){
    Serial.println("2 start");
    digitalWrite(phase2pin, HIGH);
    p2start = currentTime;
  }
}
 
void chkP3(){
  unsigned long currentTime = micros();
  unsigned long td = currentTime - p1start;
  if(p3start > p3end) {
    if(td > 0.25*holdTime && p3start < p1start){
      Serial.println("3 end");
      digitalWrite(phase3pin, LOW);
      p3end = currentTime;
    }
  }else if(td > 1.5*holdTime){
    Serial.println("3 start");
    digitalWrite(phase3pin, HIGH);
    p3start = currentTime;
  }
}
 
void loop(){
  chkP1();
  chkP2();
  chkP3();
  delayMicroseconds(100);
  if(holdTime >= minHoldTime){
    holdTime -= 0.5;
  }
}
