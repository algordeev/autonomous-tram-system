#include <Servo.h>
#include <SPI.h>
#include <MFRC522.h> // RFID library
#define SS_PIN 10
#define RST_PIN 9
MFRC522 mfrc522(SS_PIN, RST_PIN);
unsigned long uidDec, uidDecTemp;  // to store tag number in dec format
Servo servo;
void setup() {
 //Serial.begin(9600);
  SPI.begin();  // init SPI bus.
  mfrc522.PCD_Init();     // init MFRC522 card.
  servo.attach(3);
  servo.write(110);  // set servo in closed state
}



void loop() {
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


// conditions

//Serial.println(uidDec);
  
  if ((uidDec == 2305050007)||(uidDec ==3294433023)) // we compare the UID of the tag
  {
    
    servo.write(57); // if it is equal to the specified one, the servo drive opens the switch
   
  }

  
if ((uidDec == 2464178048)||(uidDec ==1269352972)) //we compare the UID of the tag
  {
    
    servo.write(127);  // if it is equal to the specified one, the servo drive closes the switch
   
  }

  //servo.write(0);  // set servo
}

