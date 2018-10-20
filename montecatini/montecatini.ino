/*
PIN ARDUINO    DESCRIZIONE FUNZIONAMENTO
RESET   RESET ESTERNO
D2    DIGITAL OUTPUT BUZZER
D3    DIGITAL INPUT COM
D4    DIGITAL INPUT  TEMPO 1
D5    DIGITAL INPUT TEMPO 2
D7    DIGITAL INPUT TEMPO3
D8    DIGITAL OUTPUT LED ATTESA
D9    DIGITAL OUTPUT LED PARTENZA
D10   DIGITAL INPUT SICUREZZA
D11    DIGITAL INPUT SENSE
D12   DIGITAL OUTPUT MOC 3010
A0    ANALOG INPUT TRIMMER 1 SCHEDA
A1    ANALOG INPUT TRIMMER 2 SCHEDA
A2    ANALOG INPUT TRIMMER 3 BORDO MACHINA 

Cosa fa:
Sono disponibili tre pulsanti TEMPO1/TEMPO2/TEMPO3, ognuno collegato ad un tempo di azionamento di un unico relè di uscita.
Premendo contemporanemanete uno dei pulsanti assieme al pulsante COM (per avere entrambe le mani impegnate), il relè si chiude per un
tempo prefissato attraverso uno dei trimmer TRIMMER1/TRIMMER2/TRIMMER3.
0. Il tempo è aggiustabile da 0.7 a 3.9s in 64 step da 50ms.
0.a - tempo aggiustabile da 0.5 a 6s (richiesta del 26/12/16)
0.b - tempo aggiustabile da 0 a 2s (richiesta del 25/02/17)

1. - All'accensione si ha un breve beep e il led verde (ATTESA) avvisa che il controllo è pronto a operare.
2. - Per avviare l'azione occorre che una coppia di pulsanti sia premuta contemporaneamente per almeno 200ms.
2.a - i due pulsante non possono essere premuti a distanza superiore a 200ms (precisazione del 26/12/16)
3. - Viene prodoto un beep (BUZZER), il led verde (ATTESA) si spegne, si accende quello rosso (PARTENZA) e il relè è azionato per il tempo impostato.
3.a - Viene prodoto un beep (BUZZER), il led verde (ATTESA) si spegne, si accende quello rosso (PARTENZA) e il relè è azionato fino a che il SENSE non è a 1. (richiesta del 25/02/2017)
    Quando SENSE = 1 parte il tempo.
4. - Alla fine del tempo il relè si apre e il led rosso si spegne. Occorre che entrambi i pulsanti siano stati premuti per l'intero tempo di chiusura del relè, possono essere
     rilasciati quando il relè si apre. L'evento è segnalato da un beep.
5. - Un nuovo ciclo è avviabile solo dopo aver rilasciato tutti i pulsanti.
6. - Dopo un tempo di sicurezza di 4s si accende il led verde (ATTESA) e può ripartire un nuovo ciclo.
6.a - tempo di sicurezza portato a 3s (richiesta del 26/12/16)
6.a - tempo di sicurezza portato a 1s (richiesta del 25/02/16)
7. - Se i pulsanti sono rilasciati durante il tempo di lavoro, il relè è immediatamente rilasciato (MOC3010), led rosso lampeggia (PARTENZA) e viene generato un beep da 2s.
7.a funzione SE esecuzione ciclo solo per ingresso chiuso SICUREZZA (richiesta del 26/12/16)

Il ciclo riprende dal punto 5.
I potenziometri sono letti quando si accende il led verde.
Il tasto reset può essere usato come emergenza per bloccare il ciclo e rilasciare immediatamente il relè. Se non si usa, può essere omesso, lasciando un pull-up su RB3.

  CREATO IL 14 DICEMBRE 2016
  AGGIORNATO AL 20/10/2018
  by MICHELE BIANCALANI
 */

// COSTANTI PROGRAMMA
const int BUZZER = 2;
const int TEMPO0Pin = 3; //com    
const int TEMPO1Pin = 4;
const int TEMPO2Pin = 5;
const int TEMPO3Pin = 7;
const int LEDATTESA = 8;
const int LEDPARTENZA = 9;
const int SICUREZZA = 10;
const int SENSE = 11;
const int MOC3010 = 12;
const int suono=4978;
const int attesapressione=200;
int TRM1 = A0;
int TRM2 = A1;
int TRM3 = A2;
int tempo1=0;
int tempo2=0;
int tempo3=0;


boolean TEMPO0State; //com
boolean TEMPO1State; 
boolean TEMPO2State;
boolean TEMPO3State;
boolean SENSEState;

boolean SICUREZZAState;

boolean oTEMPO0State; //com
boolean oTEMPO1State;
boolean oTEMPO2State;
boolean oTEMPO3State;


unsigned long timeMOC;
long timeX;
unsigned long timeX0;
unsigned long timeX1;
unsigned long timeX2;
unsigned long timeX3;

void setup() {
  pinMode(TEMPO0Pin, INPUT_PULLUP);
  pinMode(TEMPO1Pin, INPUT_PULLUP);
  pinMode(TEMPO2Pin, INPUT_PULLUP);
  pinMode(TEMPO3Pin, INPUT_PULLUP);
  pinMode(BUZZER, OUTPUT);
  pinMode(LEDATTESA, OUTPUT);
  pinMode(LEDPARTENZA, OUTPUT);
  pinMode(MOC3010, OUTPUT);
  pinMode(SICUREZZA, INPUT_PULLUP);
  pinMode(SENSE, INPUT_PULLUP);
  digitalWrite(MOC3010, LOW);
    Serial.println("MOC_OFF");
  Serial.begin(9600);

  tone(BUZZER, suono/3, 1000);
  digitalWrite(LEDATTESA, HIGH);
  digitalWrite(LEDPARTENZA, LOW);
}

void loop() {
  LETTURA();
  STATUS();
//SICUREZZAState=1;
  timeX = timeX1-timeX0;
  if (timeX<0) timeX=timeX*-1;  
  if (timeX<=attesapressione && timeX1>0 && timeX0>0 && !SICUREZZAState) { 
    Serial.print("1ES");
    ESEGUI(1);
    RESET();
    return;
  }
  timeX = timeX2-timeX0;
  if (timeX<0) timeX=timeX*-1;  
  if (timeX<=attesapressione && timeX2>0 && timeX0>0 && !SICUREZZAState) { 
    Serial.print("2ES");
    ESEGUI(2);
    RESET();
    return;
  }
  timeX = timeX3-timeX0;
  if (timeX<0) timeX=timeX*-1;  
  if (timeX<=attesapressione && timeX3>0 && timeX0>0 && !SICUREZZAState) { 
    Serial.print("3ES");
    ESEGUI(3);
    RESET();
    return;
  }
}

void RESET() {
  tempo1=0;
  tempo2=0;
  tempo3=0;
  TEMPO0State=1; 
  TEMPO1State=1; 
  TEMPO2State=1;
  TEMPO3State=1;
  SICUREZZAState=1;
  SENSEState=1;
  oTEMPO0State=1;
  oTEMPO1State=1;
  oTEMPO2State=1;
  oTEMPO3State=1;
  timeMOC=0;
  timeX=10000;
  timeX0=0;
  timeX1=10000;
  timeX2=10000;
  timeX3=10000;  
}
//LETTURA SENSORI
void LETTURA() {
  tempo1 = analogRead(TRM1);
  tempo2 = analogRead(TRM2);
  tempo3 = analogRead(TRM3);
  TEMPO0State = digitalRead(TEMPO0Pin);
  TEMPO1State = digitalRead(TEMPO1Pin);
  TEMPO2State = digitalRead(TEMPO2Pin);
  TEMPO3State = digitalRead(TEMPO3Pin);  
  SICUREZZAState = digitalRead(SICUREZZA);  
  SENSEState = digitalRead(SENSE);
  Serial.print(TEMPO0State);
  Serial.print(TEMPO1State);
  Serial.print(TEMPO2State);
  Serial.print(TEMPO3State);
  Serial.print(SICUREZZAState);
  Serial.print(SENSEState);
  Serial.print("!");
  Serial.print(tempo1,DEC);
  Serial.print("!");
  Serial.print(tempo2,DEC);
  Serial.print("!");
  Serial.println(tempo3,DEC);
}
void STATUS() {
  if (TEMPO0State==oTEMPO0State & TEMPO1State==oTEMPO1State & TEMPO2State==oTEMPO2State & TEMPO3State==oTEMPO3State) {
    return;
  }
  if (TEMPO0State==0) {
    oTEMPO0State=0;
    if (timeX0==0) timeX0=millis(); 
  } else {
    oTEMPO0State=1;
    timeX0=0;
  }
  if (TEMPO1State==0) {
    oTEMPO1State=0;
    if (timeX1==0) timeX1=millis(); 
  } else{
    oTEMPO1State=1;
    timeX1=0;     
  }
  if (TEMPO2State==0) {
    oTEMPO2State=0;
    if (timeX2==0) timeX2=millis(); 
  } else{
    oTEMPO2State=1;
    timeX2=0;     
  }
  if (TEMPO3State==0) {
    oTEMPO3State=0;
    if (timeX3==0) timeX3=millis(); 
  } else{
    oTEMPO3State=1;
    timeX3=0;     
  }
//  Serial.print(TEMPO0State);
//  Serial.print(TEMPO1State);
//  Serial.print(TEMPO2State);
//  Serial.print(TEMPO3State);
//  Serial.print(oTEMPO0State);
//  Serial.print(oTEMPO1State);
//  Serial.print(oTEMPO2State);
//  Serial.print(oTEMPO3State);
//  Serial.print(timeX);
//  Serial.print("!");
//  Serial.print(timeX0);
//  Serial.print("!");
//  Serial.print(timeX1);
//  Serial.print("!");
//  Serial.print(timeX2);
//  Serial.print("!");
//  Serial.println(timeX3);
}

//CALCOLO DEL TEMPO DI ESECUZIONE RELè
int CALCT (int tempo) {
      int t = map(tempo, 0, 1024, 000, 2050);
      int x=t % 50;
      return t-x;
}

//ESEGUE rele
void ESEGUI(byte vv) {
    oTEMPO0State=1;
    timeX0=0;
    timeX1=0;     
    timeX2=0;     
    timeX3=0;     
    int ciclo=0;
    boolean TS=0;
    boolean TC=0;
    boolean TX=0;
    tone(BUZZER, suono/2, 500);
    if (vv==1) {
      oTEMPO1State=1;
      timeX1=0;     
      //calcolo del tempo di attesa
      ciclo = CALCT(tempo1); 
    }
    if (vv==2) {
      oTEMPO2State=1;
      timeX2=0;     
      //calcolo del tempo di attesa 
      ciclo = CALCT(tempo2); 
    }
    if (vv==3) {
      oTEMPO3State=1;
      timeX3=0;     
      //calcolo del tempo di attesa 
      ciclo = CALCT(tempo3); 
    }
    digitalWrite(LEDATTESA, LOW);
    digitalWrite(LEDPARTENZA, HIGH);
    //accendo relè fino al completamento del ciclo    
    digitalWrite(MOC3010, HIGH);
    Serial.println("MOC_ON");

    //noTone(BUZZER);
//    Serial.println(ciclo);
    while (1) {
      if (digitalRead(SENSE)==0) break;
      TX = digitalRead(SICUREZZA);
      TC = digitalRead(TEMPO0Pin);
      if (vv==1) {
        TS = digitalRead(TEMPO1Pin);
      }
      if (vv==2) {
        TS = digitalRead(TEMPO2Pin);
      }
      if (vv==3) {
        TS = digitalRead(TEMPO3Pin);
      }
      // se vengono mollati i pulsanti disattivo relè e suono e lampeggio per due secondi
      if (TC==1 || TS==1 || TX==1) {
//        Serial.println(millis());
        digitalWrite(MOC3010, LOW);
    Serial.println("MOC_OFF");
        tone(BUZZER, suono/2, 2000);
        digitalWrite(LEDPARTENZA, LOW);
        delay(250);
        digitalWrite(LEDPARTENZA, HIGH);
        delay(250);
        digitalWrite(LEDPARTENZA, LOW);
        delay(250);
        digitalWrite(LEDPARTENZA, HIGH);
        delay(250);
        digitalWrite(LEDPARTENZA, LOW);
        delay(250);
        digitalWrite(LEDPARTENZA, HIGH);
        delay(250);
        digitalWrite(LEDPARTENZA, LOW);
        delay(250);
        digitalWrite(LEDPARTENZA, HIGH);
        delay(250);
        digitalWrite(LEDPARTENZA, LOW);
        digitalWrite(LEDATTESA, HIGH);
        return;
      }
    } 
    timeMOC = millis()+ciclo;
    Serial.println(timeMOC);
    while (1) {
      TC = digitalRead(TEMPO0Pin);
      TX = digitalRead(SICUREZZA);
      if (vv==1) {
        TS = digitalRead(TEMPO1Pin);
      }
      if (vv==2) {
        TS = digitalRead(TEMPO2Pin);
      }
      if (vv==3) {
        TS = digitalRead(TEMPO3Pin);
      }
      // se vengono mollati i pulsanti disattivo relè e suono e lampeggio per due secondi
      if (TC==1 || TS==1 || TX==1) {
//        Serial.println(millis());
        digitalWrite(MOC3010, LOW);
    Serial.println("MOC_OFF");
        tone(BUZZER, suono/2, 2000);
        digitalWrite(LEDPARTENZA, LOW);
        delay(250);
        digitalWrite(LEDPARTENZA, HIGH);
        delay(250);
        digitalWrite(LEDPARTENZA, LOW);
        delay(250);
        digitalWrite(LEDPARTENZA, HIGH);
        delay(250);
        digitalWrite(LEDPARTENZA, LOW);
        delay(250);
        digitalWrite(LEDPARTENZA, HIGH);
        delay(250);
        digitalWrite(LEDPARTENZA, LOW);
        delay(250);
        digitalWrite(LEDPARTENZA, HIGH);
        delay(250);
        digitalWrite(LEDPARTENZA, LOW);
        digitalWrite(LEDATTESA, HIGH);
        return;
      }
      if (millis()>=timeMOC) {
        digitalWrite(MOC3010, LOW);
    Serial.println("MOC_OFF");
        break;
      }
    } 
    digitalWrite(LEDPARTENZA, LOW);
    //suono di successo per 500 secondi
    tone(BUZZER, suono/3, 60);
    delay(60);
    noTone(BUZZER);
    delay(60);
    tone(BUZZER, suono/3, 60);
    delay(60);
    noTone(BUZZER);
    delay(60);
    tone(BUZZER, suono/3, 60);
    delay(60);
    noTone(BUZZER);
    delay(60);
    tone(BUZZER, suono/2, 60);
    delay(60);
    noTone(BUZZER);
    delay(60);
    //attendo che tutti i pulsanti siano a 0
    while (1) {
      LETTURA();      
      if (TEMPO0State==1 & TEMPO1State==1 & TEMPO2State==1 & TEMPO3State==1 & SENSEState==1) break;
    }
    tone(BUZZER, suono/3, 500);
    digitalWrite(LEDATTESA, HIGH);
}
