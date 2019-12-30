#include <Servo.h>
#include "MeOrion.h"

enum MotorsList {X, Y};

int dirPinX = mePort[PORT_1].s1;  //the direction pin connect to Base Board PORT1 SLOT1
int stpPinX = mePort[PORT_1].s2;  //the Step pin connect to Base Board PORT1 SLOT2

int dirPinY = mePort[PORT_2].s1;  //the direction pin connect to Base Board PORT1 SLOT1
int stpPinY = mePort[PORT_2].s2;  //the Step pin connect to Base Board PORT1 SLOT2

MePort servo_port(PORT_7);
Servo servo;
int16_t servo_pin =  servo_port.pin2();

void makeStepsFromMm(MotorsList motor, bool dir, float mm);
void servoCommand(int current_int, short i);

void setup()
{
  Serial.begin(9600);
  
  servo.attach(servo_pin);
  servo.write(0);
  
  pinMode(dirPinX, OUTPUT);
  pinMode(stpPinX, OUTPUT);
  
  pinMode(dirPinY, OUTPUT);
  pinMode(stpPinY, OUTPUT);
}

void loop()
{
  if(Serial.available() > 0)
  {
    char servo_delay = Serial.read();
    
    Serial.write(servo_delay);
    
    int current_int = 2730;
    for(short i = 5; i >= 3; i--)          //points 1->3
    {
      if(i != 5)
        makeStepsFromMm(MotorsList::Y, 1, 2.5);
      servoCommand(current_int, i, servo_delay); 
    }
    makeStepsFromMm(MotorsList::X, 0, 2.5);
    
    for(short i = 0; i < 3; i++)           //points 6->4
    {
      if(i != 0)
        makeStepsFromMm(MotorsList::Y, 0, 2.5);
      servoCommand(current_int, i, servo_delay); 
    }
    makeStepsFromMm(MotorsList::X, 0, 3.5);
  
    for(short i = 11; i >= 9; i--)         //points 7->9
    {
      if(i != 11)
        makeStepsFromMm(MotorsList::Y, 1, 2.5);
      servoCommand(current_int, i, servo_delay);        
    }
    makeStepsFromMm(MotorsList::X, 0, 2.5);
    
    for(short i = 6; i < 9; i++)           //points 12->10
    {
      if(i != 6)
        makeStepsFromMm(MotorsList::Y, 0, 2.5);
      servoCommand(current_int, i, servo_delay); 
    }
    makeStepsFromMm(MotorsList::X, 0, 3.5);

    while(Serial.available() > 0) //clear buffer
      Serial.read();
  }
}

void servoCommand(int current_int, short i, char servo_delay)
{
  if(bitRead(current_int, i)) //1
    servo.write(90);

  Serial.write(servo_delay);

  servo_delay = 100;
  
  delay(servo_delay);

  servo.write(0);
}

void makeStepsFromMm(MotorsList motor, bool dir, float mm)
{
  int stpPin = 0;
  int dirPin = 0;
  
  switch(motor)
  {
  case MotorsList::X:
    stpPin = stpPinX;
    dirPin = dirPinX;
    break;
  case MotorsList::Y:
    stpPin = stpPinY;
    dirPin = dirPinY;
    break;
  default:break;
  }
  
  digitalWrite(dirPin, dir);
  delay(10);

  int steps = mm * 11200./128.;      //35/128 : valeur expérimentale converstion cm -> tours * 3200 pour tours -> steps

  for(int i = 0; i < steps; i++)
  {
    digitalWrite(stpPin, HIGH);
    delayMicroseconds(800);
    digitalWrite(stpPin, LOW);
    delayMicroseconds(800); 
  }
}
