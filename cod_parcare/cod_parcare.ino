#include <Servo.h>
#include <LiquidCrystal.h>

#define RS 12
#define E 11
#define D4 2
#define D5 3
#define D6 4
#define D7 5
#define MAX_LOCURI_PARCARE 2
#define PARCARE_PLINA 0

int nrLocuriParcare = MAX_LOCURI_PARCARE;

Servo barieraIntrare, barieraIesire;

LiquidCrystal lcd(RS, E, D4, D5, D6, D7);

const int echoPin = 10 ;
const int triggerPin = 9;

const int ledVerde = 6;
const int ledRosu = 7;

int senzorIntrare = A0; 
int senzorIesire = A1; 
int senzorLoc1 = A5;
int senzorLoc2 = A4; 

unsigned long startStationare1 = 0, startStationare2=0;
unsigned int suma1 = 0, suma2=0;
int locOcupat1 = false, locOcupat2 = false;

//functie responsabila pentru comutarea barierei
void ridicaBariera(Servo servo){
    servo.write(0);
    delay(3000);
    servo.write(90);
}

//functie responsabila cu citirea starii senzorului cu ultrasunete
long citesteDistantaSenzor() {
    long duration, distance;

    digitalWrite(triggerPin, LOW);
    delayMicroseconds(2);
    digitalWrite(triggerPin, HIGH);
    delayMicroseconds(10);
    digitalWrite(triggerPin, LOW);

    duration = pulseIn(echoPin, HIGH);
    distance = (duration / 2) / 29.1;

    return distance;
}

//functie responsabila cu gestionarea senzorului de la intrare
void gestioneazaIntrare() {
    long distanta= citesteDistantaSenzor();
    int stareIntrare = digitalRead(senzorIntrare);

    if (stareIntrare == LOW && nrLocuriParcare > PARCARE_PLINA && nrLocuriParcare <= MAX_LOCURI_PARCARE && distanta<8) {
        ridicaBariera(barieraIntrare);
        nrLocuriParcare--;
    }
}

//functie responsabila cu gestionarea senzorului de la iesire
void gestioneazaIesire() {
    int stareIesire = digitalRead(senzorIesire);

    if (stareIesire == LOW && nrLocuriParcare < MAX_LOCURI_PARCARE) {
        ridicaBariera(barieraIesire);
        nrLocuriParcare++;
    }
}

//functie responsabila cu afisarea pe LCD a numarului de locuri
void afiseazaNumarLocuri() {
    lcd.setCursor(8, 0);
    lcd.print("     ");
    lcd.setCursor(8, 0);
    lcd.print(nrLocuriParcare);
}

//functie responsabila cu aprinderea celor 2 leduri 
void gestioneazaLeduri(){
    if (nrLocuriParcare == 0) {
      digitalWrite(ledVerde, LOW);
      digitalWrite(ledRosu, HIGH);
    } else {
      digitalWrite(ledVerde, HIGH);
      digitalWrite(ledRosu, LOW);
    }
}

//functie responsabila cu afisarea sumei de plata in Serial Monitor
void afisareSumaPlata(int sumaAfisare){
  Serial.print("Aveti de plata suma de: ");
  Serial.print(sumaAfisare);
  Serial.println(" lei");
  Serial.println('\n');
  Serial.println("Va rugam introduceti suma de plata!");
}

//functie responsabila cu plata sumei din Serial Monitor
void calculeazaSuma(int suma){
  afisareSumaPlata(suma);
      
  while (Serial.available() == 0) {
  }
  
  int numar = Serial.parseInt();
  if (suma == numar) {
    Serial.print("Suma a fost achitata, va multumim si o zi frumoasa ");
  }

  while (numar < suma) {
    suma = suma - numar;
    Serial.print("Mai aveti de introdus: ");
    Serial.print(suma);
    Serial.println(" lei");
    numar = Serial.parseInt();
  }

  if (numar > suma) {
    Serial.print("Va rugam ridicati restul: ");
    Serial.print(numar - suma);
    Serial.println(" lei");
  }
  suma = 0;
  Serial.print("Suma a fost achitata, va multumim si o zi frumoasa! ");
  Serial.println('\n');
}

//functie responsabila cu gestionarea senzorilor de pe fiecare loc de parcare
void gestioneazaLocParcare(int senzorLoc, int &locOcupat, unsigned long &startStationare, unsigned int &suma) {
  int numar = 0;
  int stareLocParcare = digitalRead(senzorLoc);
  if (stareLocParcare == LOW && !locOcupat) {
    locOcupat = true;
    startStationare = millis();
    Serial.println("Masina a intrat in locul de parcare");
  } 
  else if (stareLocParcare == HIGH && locOcupat) {
    locOcupat = false;
    unsigned int suma = calculTimpParcare(startStationare);
    if(suma > 0){
      calculeazaSuma(suma);
    }
  }
}

//functie responsabila cu, calculul timpului petrecut in parcare pe fiecare loc
unsigned int calculTimpParcare(unsigned long start){
  unsigned long stopStationare = millis();
  unsigned long timpParcare = stopStationare - start;
  
  Serial.print("Masina a stat in parcare pentru: ");
  Serial.print(timpParcare / 1000);
  Serial.println(" secunde");
  
  return timpParcare / 1000 * 2;
}


void setup() {
  Serial.begin(9600);
  barieraIntrare.attach(8);
  barieraIesire.attach(13);

  pinMode(echoPin, INPUT);
  pinMode(triggerPin, OUTPUT);

  pinMode(ledVerde, OUTPUT);
  pinMode(ledRosu, OUTPUT);
  
  lcd.begin(16, 2);
  lcd.print("Locuri: ");
  lcd.setCursor(8, 0);
  lcd.print(MAX_LOCURI_PARCARE);

  digitalWrite(ledVerde, HIGH);
  digitalWrite(ledRosu, LOW);

  pinMode(senzorIntrare, INPUT); 
  pinMode(senzorIesire, INPUT); 

  pinMode(senzorLoc1, INPUT);
  pinMode(senzorLoc2, INPUT);

  barieraIntrare.write(90);
  barieraIesire.write(90);
}

void loop() {
    gestioneazaLeduri();
    gestioneazaIntrare();
    gestioneazaIesire();
    afiseazaNumarLocuri();

    if(nrLocuriParcare>=PARCARE_PLINA && nrLocuriParcare<MAX_LOCURI_PARCARE){
      gestioneazaLocParcare(senzorLoc1, locOcupat1, startStationare1, suma1);
      gestioneazaLocParcare(senzorLoc2, locOcupat2, startStationare2, suma2);
    }
}