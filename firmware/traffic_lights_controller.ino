#include <SPI.h>
#include <MFRC522.h> //  "RFID" library
#define RED1 5
#define ORG1 3
#define GRN1 4
#define RED2 8
#define ORG2 6
#define GRN2 7


#define SS_PIN 10
#define RST_PIN 9
MFRC522 mfrc522(SS_PIN, RST_PIN);
unsigned long uidDec, uidDecTemp;  // to store tag number in dec format

// delay in sec between traffic light states
int pause = 1000;

void setup() {
  //Serial.begin(9600);
  SPI.begin();  //  init SPI bus.
  mfrc522.PCD_Init();     // init MFRC522 card.
   pinMode(RED1, OUTPUT);
   pinMode(ORG1, OUTPUT);
   pinMode(GRN1, OUTPUT);
   pinMode(RED2, OUTPUT);
   pinMode(ORG2, OUTPUT);
   pinMode(GRN2, OUTPUT);

 

}

void loop() {
  // first phase
   digitalWrite(RED1, HIGH);
   digitalWrite(ORG1, LOW);
   digitalWrite(GRN1, LOW);
   digitalWrite(RED2, LOW);
   digitalWrite(ORG2, LOW);
   digitalWrite(GRN2, HIGH);


// new tag search
  if ( ! mfrc522.PICC_IsNewCardPresent()) {
    return;
  }
  // selecting a tag
  if ( ! mfrc522.PICC_ReadCardSerial()) {
    return;
  }
  uidDec = 0;
  // issuing a serial number for the tag
  for (byte i = 0; i < mfrc522.uid.size; i++)
  {
    uidDecTemp = mfrc522.uid.uidByte[i];
    uidDec = uidDec * 256 + uidDecTemp;
  }

  
  //Serial.print(uidDec);
if ((uidDec == 2305050007)||(uidDec == 2464178048)||(uidDec ==1269352972)){
     // second phase
      digitalWrite(RED1, HIGH);
      digitalWrite(ORG1, HIGH);
      digitalWrite(GRN1, LOW);
      digitalWrite(RED2, LOW);
      digitalWrite(ORG2, HIGH);
      digitalWrite(GRN2, LOW);
      delay(pause);
     // third phase
      digitalWrite(RED1, LOW);
      digitalWrite(ORG1, LOW);
      digitalWrite(GRN1, HIGH);
      digitalWrite(RED2, HIGH);
      digitalWrite(ORG2, LOW);
      digitalWrite(GRN2, LOW);
      delay(10000);
     // fourth phase
      digitalWrite(RED1, LOW);
      digitalWrite(ORG1, HIGH);
      digitalWrite(GRN1, LOW);
      digitalWrite(RED2, HIGH);
      digitalWrite(ORG2, HIGH);
      digitalWrite(GRN2, LOW);
      delay(pause);
   }
}

