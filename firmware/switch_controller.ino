#include <Servo.h>
#include <SPI.h>
#include <MFRC522.h> // библиотека "RFID".
#define SS_PIN 10
#define RST_PIN 9
MFRC522 mfrc522(SS_PIN, RST_PIN);
unsigned long uidDec, uidDecTemp;  // для храниения номера метки в десятичном формате
Servo servo;
void setup() {
 //Serial.begin(9600);
  SPI.begin();  //  инициализация SPI / Init SPI bus.
  mfrc522.PCD_Init();     // инициализация MFRC522 / Init MFRC522 card.
  servo.attach(3);
  servo.write(110);  // устанавливаем серву в закрытое сосотояние
}



void loop() {
  // Поиск новой метки
  if ( ! mfrc522.PICC_IsNewCardPresent()) {
    return;
  }
  // Выбор метки
  if ( ! mfrc522.PICC_ReadCardSerial()) {
    return;
  }
  uidDec = 0;
  // Выдача серийного номера метки.
  for (byte i = 0; i < mfrc522.uid.size; i++)
  {
    uidDecTemp = mfrc522.uid.uidByte[i];
    uidDec = uidDec * 256 + uidDecTemp;
  }


//условия

//Serial.println(uidDec);
  
  if ((uidDec == 2305050007)||(uidDec ==3294433023)) // Сравниваем Uid метки, если он равен заданому то серва открывает.
  {
    
    servo.write(57); // Поворациваем серву на угол 90 градусов(Отпираем какой либо механизм: задвижку, поворациваем ключ и т.д.)
   
  }

  
if ((uidDec == 2464178048)||(uidDec ==1269352972)) // Сравниваем Uid метки, если он равен заданому то серва открывает.
  {
    
    servo.write(127); // Поворациваем серву на угол 90 градусов(Отпираем какой либо механизм: задвижку, поворациваем ключ и т.д.)
   
  }

  //servo.write(0);  // устанавливаем серву в закрытое сосотояние
}
