#include <Encoder.h>

Encoder myEnc(2,3);

int counter = 0;


void setup() {
  // put your setup code here, to run once:

 Serial.begin(9600);
}

long oldPosition  = -999;

void loop() {
  long newPosition = (myEnc.read()/3);
  if ((newPosition != oldPosition) && (newPosition >= 0)) {
    
    oldPosition = newPosition;
    Serial.println(newPosition);
  }
}
