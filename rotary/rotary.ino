#include <Encoder.h>

Encoder myEnc(2,3);

int counter = 100;
int diff = 0;

void setup() {
  // put your setup code here, to run once:

 Serial.begin(9600);
}

long oldPosition  = 0;

void loop() {
  long newPosition = (myEnc.read()/2);
  if (newPosition != oldPosition) {
    diff = newPosition - oldPosition; 
    counter = counter + diff;
    diff = 0;
    oldPosition = newPosition;
    if (counter >=990) {
       counter = 990;
       }
    if (counter <= 2) {
      counter = 2;
    }
    
    Serial.println(counter);
  }
}
