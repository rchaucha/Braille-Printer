#include "MeOrion.h"
#include <Servo.h>

MePort port(PORT_7);
Servo servo;  // create servo object to control a servo
int16_t servopin =  port.pin2();//attaches the servo on PORT_3 SLOT2 to the servo object
int16_t potpin = A1;//  // analog pin used to connect the potentiometer
int16_t val;    // variable to read the value from the analog pin

void setup()
{
  Serial.begin(9600);
  servo.attach(servopin);  // attaches the servo on servopin
  servo.write(130);
}

void servoCommand(int current_int, short i)
{
  if(bitRead(current_int, i)) //1
  {
    Serial.println("130");
    servo.write(130);

    delay(200);
    
    Serial.println("92");
    servo.write(92);
  }
  else
  {
    Serial.println("88");
    servo.write(90);
  }

  delay(500);
}

void loop()
{
  /*int current_int = 5;

  for(short i = 0; i < 3; i++)
    servoCommand(current_int, i);*/
}

