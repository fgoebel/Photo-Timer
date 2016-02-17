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
  delayMicroseconds(LedTime); //sorgt dafür, dass die Elemente eine Weile an sind..
  for (int j = 6; j >=0 ;j--){
    digitalWrite(pinArray[6-j], HIGH);
    }
    digitalWrite(decimalPoint, HIGH);
  delayMicroseconds(maxLedTime - LedTime);
}

