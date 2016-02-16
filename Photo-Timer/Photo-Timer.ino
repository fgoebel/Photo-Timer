byte segmente[10] = { 
  B01111110, //0  B0ABCDEFG
  B00110000, //1
  B01101101, //2
  B01111001, //3
  B00110011, //4
  B01011011, //5
  B01011111, //6
  B01110000, //7
  B01111111, //8
  B01111011};//9
  //B00000001 Decimal Point
  /*wenn man jetzt also den Decimal Point zu den Restlichen Bytes addiert hab ich den Punkt auch an..
   * so far so good
   */

enum states {WAITING, IDLING, STARTING, STOPPING};
states state = IDLING;

enum  motorStates {WAITING_FOR_HIGH, WAITING_FOR_LOW};
motorStates motorState = WAITING_FOR_HIGH; //bei IR HIGH sind wir auf der Scheibe und können dann gezielt auf low warten.


byte pinArray[] = {16,15,10,11,9,17,18}; // Segmente A,B,C,D,E,F,G
byte decimalPoint = 12;
byte einerStelle = 14; //A0
byte zehnerStelle = 13;
byte relayPin = 4;

byte buttonPin = 7; 
bool isPressed = false; //wenn buttonPin gedrückt ist
bool clickPress = false;
bool longPress = false; 
bool longPressDisable = false;
#define longPressDuration 1000

unsigned long buttonPressTime = 0;
unsigned long lastReleaseTime = 0;
unsigned long pressedTime = 0;

bool justStarted = false;
bool justStopped = false;

#include <Encoder.h>
Encoder myEnc(2, 3); //besser später selbst implementieren. 
                     // oder auch nicht.. das bounced wie hölle
int encoder = 0;
#define debounce 20 //


int TimeToDisplay = 0; //wert in Sec *10 ->> zentel Sekunden sind per integer darstellbar
int TimeSet = 100; //also 10s, Berechnung long = int * int geht nicht.. auf long = int * long geändert.
int TimeSave = 0; //um nach ablauf auf eingestellten Wert zurückzusetzen.
int LedTime; //zeit in µs welche die LEDs an sind..
int maxLedTime = 2000;
byte brightness = 100;
unsigned long waitTime = 0; //ms Verzögerung Motor 
int updateInterval = 100; //ms um display upzudaten.
unsigned long currentMillis;
unsigned long previousMillis = 0;
unsigned long waitStartTime = 0;
unsigned long lastDisplayTime = 0 ;
unsigned long TimeSetLastChanged = 0 ;
unsigned long lastStartTime = 0;
unsigned long lastStopTime = 0;




//SETUP//
void setup() {
  for(int i = 0; i < 7; i++)
  pinMode(pinArray[i], OUTPUT);
  pinMode(decimalPoint, OUTPUT);
  pinMode(einerStelle, OUTPUT);
  pinMode(zehnerStelle, OUTPUT);
  pinMode(relayPin, OUTPUT);
  digitalWrite(relayPin,LOW);
  //Serial.begin(9600);
  TimeToDisplay = TimeSet;

//init Inputs
  pinMode(buttonPin,INPUT_PULLUP); //button pins as input with Pullup

brightness = constrain(brightness, 0, 100);
LedTime = map(brightness,0,100,0,maxLedTime);
 
}
// END SETUP //
void loop() {
currentMillis = millis();

switch (state) {
  case WAITING:
    waiting();
    break;
  case IDLING:
    idling();
    break;
  case STARTING:
    starting();
    break;
  case STOPPING:
    stopping();
    break;
  }

//jeder Loop zu tun
button();
anzeige(TimeToDisplay); //wert der Angezeigt werden soll  
//Serial.println(encoder);
/* startStop ?? state = IDLING;
*
 * if ButtonSTART und ich bin nicht in Idling -> idling. sonst verlasse Idling
 */
//ende Loop
}


void starting(void){
  justStarted = true; //um es nur einmal zu tun.. wird nach "debounce" ms zurückgesetzt.
  lastStartTime = currentMillis;
  waitStartTime = currentMillis;
  waitTime = TimeSet * 100L; //axo auf ms umrechenn :-/
  state = WAITING;
  digitalWrite(relayPin,HIGH);
  myEnc.write(0);
}
//ENDE starting(void)

void stopping(void) {
 justStopped = true;   
 lastStopTime = currentMillis;
 state = IDLING;
 digitalWrite(relayPin,LOW);
 myEnc.write(0);
}
//ENDE stopping(void)


void waiting(void){
  //waitTime ist gesetzt.. jetzt zählen wir halt in 100ms Schritten von Wert auf 0...
  if (currentMillis - lastDisplayTime >= updateInterval) {  
    TimeToDisplay = TimeSet - ((currentMillis - waitStartTime)/100) -1 ; //besser mit der gesamt different arbeiten.. die 100ms werden ja nicht genau getroffen.
    lastDisplayTime = currentMillis;
  }
  
  if (currentMillis - waitStartTime >= waitTime){
    state = STOPPING;
    if (TimeSave != 0 ){
    TimeSet = TimeSave;
    TimeSave = 0;
      }
    }
  if (!justStarted && clickPress ) {//durch Flag "klick" ersetzen.. //wenn StartStopp gedrückt -> gehe in stopping
    clickPress = false;
    state = STOPPING;
    if (TimeSave == 0 ){
    TimeSave = TimeSet; 
    }
    TimeSet = TimeToDisplay; // um das abzuspeichern
  }
}
//ENDE waiting(void)
void idling(void) {
  //

    if (clickPress && !justStopped) { 
      state = STARTING; //umschreiben auf Trigger durch release nach min 20ms
      clickPress = false;
      return;
      }
  if (TimeSave == 0){ //verstellen nur, wenn wir nicht pausiert haben...
      encoder = myEnc.read()/2;
    if (encoder != 0) {
      if (TimeSet >= 100) {
        TimeSet = TimeSet + encoder*10; 
      }
      else {
        TimeSet = TimeSet + encoder*5; 
      }
      myEnc.write(0);// ich will ja nur relative änderung haben..
    }
    //TimeSet Limitieren
    if (TimeSet >= 990) {
      TimeSet = 990;
      }
    if (TimeSet <= 5 ) {
      TimeSet = 5;  
    }
  }
  else {
    //es wurde also pausiert!
      if (longPress) {
      longPress = false;
      TimeSet = TimeSave;
      TimeSave = 0;
      }
    }
    TimeToDisplay = TimeSet;
  //nur wenn TimeSave nicht gesetzt ist..
}
//ENDE idling(void)

void button(void){ //checks if Buttons are pressed.
      bool buttonCheck = !digitalRead(buttonPin); //pull-Up --> pressed führt zu low-Pegel
      
    //buttonCheck = true, aber !isPressed und (lastReleaseTime ist mehr als debounce ms) her!
    //also jetzt isPressed setzen! und buttonPressTime
    if ( buttonCheck && !isPressed && (currentMillis - lastReleaseTime > debounce)){
      isPressed = true;
      buttonPressTime = currentMillis;
    }

    //isPressed und buttonCheck und Zeit >2s
    //--> longPress
    if (!longPressDisable && !longPress && isPressed && buttonCheck && (currentMillis - buttonPressTime > longPressDuration )){
      //nur setzen, wenn wir den knopf zwischen durch mal losgelassen haben...
      longPress = true;
      longPressDisable = true;
      }

    //isPressed und !buttonCheck
    if (isPressed && !buttonCheck){
      pressedTime = currentMillis - buttonPressTime;
      isPressed = false;
      longPressDisable = false;
      //zeit < 20ms oder > 2s 
        // --> setze isPressed = false sonst nix..
        //---> lastReleaseTime = curren
     if ((pressedTime < 20) ||(pressedTime > longPressDuration) ) {
      lastReleaseTime = currentMillis;
     }
      //zeit > 20ms und <2s
        // trigger clickPress
        //last releaseTime ==current
     else if ( !((pressedTime < 20) ||(pressedTime > longPressDuration)) && !clickPress) {
      clickPress = true;
      lastReleaseTime = currentMillis;
       }

    }
    //die flags zurücksetzen!


    //reset justStarted && justSTopped
    if (!isPressed){
      if (justStarted && (currentMillis - lastStartTime > debounce )){
        justStarted = false;
        }
       if (justStopped && (currentMillis - lastStopTime > debounce)){
        justStopped = false; 
       }
    }  
}
//ENDE button(void)

