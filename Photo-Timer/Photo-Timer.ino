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


byte pinArray[] = {2,3,4,5,6,7,8}; // Segmente A,B,C,D,E,F,G
byte decimalPoint = 9;
byte einerStelle = 12;
byte zehnerStelle = 13;
byte relayPin = A5;

byte buttonArray[] = {16,17,18}; //DOWN A2, UP A3, START A4
unsigned long lastButtonTime[] = {0,0,0}; //Die Zeiten, wann welcher knop das letzte mal gedrückt wurde..
bool buttonPressed[] = {false,false,false};
bool justStarted = false;
bool justStopped = false;

#define ButtonDOWN 0
#define ButtonUP 1
#define ButtonSTART 2
int debounce = 20; //

//somit sollte ich doch mit buttonArray(ButtonDOWN) an den Pin kommen oder?


int TimeToDisplay = 0; //wert in Sec *10 ->> zentel Sekunden sind per integer darstellbar
int TimeSet = 25; // muss long sein, sonst läuft waitTime über.. warum?
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
 for (int i = 0 ; i < 3; i++) {
  pinMode(buttonArray[i],INPUT); //button pins as input
  digitalWrite(buttonArray[i],HIGH); //button pins enable Pullup
 }
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
/* startStop ?? state = IDLING;
*
 * if ButtonSTART und ich bin nicht in Idling -> idling. sonst verlasse Idling
 */
} 
//ende Loop

void starting(void){
  justStarted = true; //um es nur einmal zu tun.. wird zurückgesetzt, wenn der kopf losgelassn wird.?
  lastStartTime = currentMillis;
  waitStartTime = currentMillis;
  waitTime = TimeSet * 100L; //axo auf ms umrechenn :-/
  state = WAITING;
  digitalWrite(relayPin,HIGH);
}

void stopping(void) {
 justStopped = true;   
 lastStopTime = currentMillis;
 state = IDLING;
 digitalWrite(relayPin,LOW);
}


void waiting(void){
  //waitTime ist gesetzt.. jetzt zählen wir halt in 100ms Schritten von Wert auf 0...
  if (currentMillis - lastDisplayTime >= updateInterval) {  
    TimeToDisplay = TimeSet - ((currentMillis - waitStartTime)/100) -1 ; //besser mit der gesamt different arbeiten.. die 100ms werden ja nicht genau getroffen.
    lastDisplayTime = currentMillis;
  }
  
  if (currentMillis - waitStartTime >= waitTime){
    state = STOPPING;
    }
  if (!justStarted && buttonPressed[ButtonSTART] ) {//wenn StartStopp gedrückt -> gehe in stopping
    state = STOPPING;
    TimeSet = TimeToDisplay; // um das abzuspeichern
  }
}

void idling(void) {
  //
  int incrementDelay;
  if (buttonPressed[ButtonUP]){
    incrementDelay = calcIncDelay(currentMillis, buttonPressed[ButtonUP],lastButtonTime[ButtonUP]); //ms verzögerung
  }
  if (buttonPressed[ButtonDOWN]){
    incrementDelay = calcIncDelay(currentMillis, buttonPressed[ButtonDOWN],lastButtonTime[ButtonDOWN]); //ms verzögerung
    }
  
  if (buttonPressed[ButtonSTART] && !justStopped) { 
    state = STARTING;
    return;
    }
    int increment = 1;
    
  if (currentMillis - TimeSetLastChanged > incrementDelay) {
    if ((buttonPressed[ButtonUP] && !buttonPressed[ButtonDOWN])&&(TimeSet < 990)) {
      if (TimeSet >=100)
        increment = 10;
      TimeSet += increment;
      TimeSetLastChanged = currentMillis;
      }
    else if ((buttonPressed[ButtonDOWN] && !buttonPressed[ButtonUP]) && (TimeSet > 0)) {
      if (TimeSet>110){
        increment=10;
        TimeSet -= increment;
      }
      else if (TimeSet > 100)
        TimeSet = 100;
      else
        TimeSet -= increment;
      TimeSetLastChanged = currentMillis;
      }
  }
  
  
  if (!buttonPressed[ButtonDOWN] && ! buttonPressed[ButtonUP]){
     TimeSetLastChanged = 0;
  }
  
  TimeToDisplay = TimeSet;
}

int calcIncDelay(unsigned long currentMillis, bool buttonPressed, unsigned long lastButtonTime){
    if ((currentMillis - lastButtonTime > 500) && buttonPressed ){
    if (TimeSet > 100)
      return 100; //wenn man sekunden verstellt nur noch mit 200ms weiter hochzählen
    else
     return 50; //erst langsam, dann schneller hihi
    }
    return 500;
  }


void button(void){ //checks if Buttons are pressed.
    for (int i = 0 ; i < 3 ; i++){
      bool buttonCheck = !digitalRead(buttonArray[i]); //pull-Up --> pressed führt zu low-Pegel
      if (buttonCheck && !buttonPressed[i]) {
        if (currentMillis - lastButtonTime[i] > debounce ){
          buttonPressed[i] = true;
          lastButtonTime[i] = currentMillis;
        }     
      }
      else if (!buttonCheck) {
        buttonPressed[i] = false;
      }  
    }

    //reset justStarted && justSTopped
    if (!buttonPressed[ButtonSTART]){
      if (justStarted && (currentMillis - lastStartTime > debounce )){
        justStarted = false;
        }
       if (justStopped && (currentMillis - lastStopTime > debounce)){
        justStopped = false; 
       }
    }  
}


void anzeige(int wert){
  byte einer,zehner;
  bool doDecimal;
  if (wert > 99) {
    doDecimal = false;
    wert = wert / 10;}
  else
    doDecimal = true;

  
  zehner = int(wert/10);
  einer = wert - zehner *10; 
  digitalWrite(zehnerStelle,HIGH); //abschalten
  digitalWrite(einerStelle,LOW);
  digitalWrite(zehnerStelle,HIGH);
  ansteuerung(einer,!doDecimal);
  digitalWrite(zehnerStelle,LOW);
  digitalWrite(einerStelle,HIGH);
  ansteuerung(zehner,doDecimal);  
  }

void ansteuerung(int a,bool doDecimalPoint){
  for(int j = 6 ; j >= 0 ; j--){
    digitalWrite(pinArray[6-j], bitRead(segmente[a],j) == 1?LOW:HIGH);
  }
  digitalWrite(decimalPoint, doDecimalPoint?LOW:HIGH); //geht, man weiss aber nicht warum.
  delay(1); //sorgt dafür, dass die Elemente eine Weile an sind..
  for (int j = 6; j >=0 ;j--){
    digitalWrite(pinArray[6-j], HIGH);
    }
    digitalWrite(decimalPoint, HIGH);
  //delay(2);
}


