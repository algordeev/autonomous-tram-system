#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>
#include <VL53L0X.h>
#include <Wire.h>

VL53L0X sensor; 

const int a1=4;
const int a2=3;
const int pwmA=5;
#define gerk A2
#define gerkGnd A3
int pwm = 0;
const uint64_t pipe = 0xE8E8F0F0E1LL;
RF24 radio(7,8);
int joystick[4];
void setup()
{

  Wire.begin();

  sensor.init();
  sensor.setTimeout(500);

  sensor.setSignalRateLimit(0.1);
  sensor.setVcselPulsePeriod(VL53L0X::VcselPeriodPreRange, 18);
  sensor.setVcselPulsePeriod(VL53L0X::VcselPeriodFinalRange, 14);

  sensor.setMeasurementTimingBudget(20000);
  
  delay(1000);
 
  radio.begin();
  radio.openReadingPipe(1,pipe);
  radio.startListening();;

  pinMode(2, OUTPUT);
  pinMode(a1, OUTPUT);
    pinMode(a2, OUTPUT);
      pinMode(pwmA, OUTPUT);
        pinMode(gerk, INPUT);
         pinMode(gerkGnd, OUTPUT);
  digitalWrite(gerkGnd,LOW);
}


void loop()
{

  
  
  if ( radio.available() )
  {
   
    bool done = false;
    while (!done)
    {
      done = radio.read( joystick, sizeof(joystick));

if(joystick[2]==0)
{
     
    if(!joystick[1]) // autopilot
    {
      
    if(joystick[0]==0)
{    digitalWrite(a1,LOW);
    digitalWrite(a2,LOW);
}
if(joystick[3])
{  pwm = map(joystick[0],1,1024,0,170);
    analogWrite(pwmA,pwm);
    digitalWrite(a1,LOW);
    digitalWrite(a2,HIGH);
   
}
if(!joystick[3])
{  pwm = map(joystick[0], 1,1024,0,170);
    analogWrite(pwmA,pwm);
    digitalWrite(a2,LOW);
    digitalWrite(a1,HIGH);
    
}


     
    }  
    else // autopilot
    { 
      
  if(sensor.readRangeSingleMillimeters()>70) // laser rangefinder
  {
analogWrite(pwmA,55);
    digitalWrite(a2,LOW);
    digitalWrite(a1,HIGH);
  }
  else
  {
    digitalWrite(a1,LOW);
    digitalWrite(a2,LOW);
  }

 
  if(analogRead(gerk) == 0)// reed switch
     {
      analogWrite(pwmA,20);
    digitalWrite(a2,LOW);
    digitalWrite(a1,HIGH);
    delay(2000);
      digitalWrite(a1,LOW);
    digitalWrite(a2,LOW);
    delay(5000);
     }
}

}
else
{
 digitalWrite(a1,LOW);
    digitalWrite(a2,LOW); 
}









    
  }
  }
  

  }

