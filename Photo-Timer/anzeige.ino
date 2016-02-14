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

