#include "MeOrion.h"

int dirPin = mePort[PORT_1].s1;//the direction pin connect to Base Board PORT1 SLOT1
int stpPin = mePort[PORT_1].s2;//the Step pin connect to Base Board PORT1 SLOT2

void step(boolean dir,int steps);


bool sens = 1;

void setup()
{
  Serial.begin(9600);
  
  pinMode(dirPin, OUTPUT);
  pinMode(stpPin, OUTPUT);
}

void step_in_cm(boolean dir,int cm)
{
  digitalWrite(dirPin,dir);
  delay(50);

  int steps = cm * 112000/128;      //35/128 : valeur expérimentale converstion cm -> tours * 3200 pour tours -> steps

  Serial.println(cm, DEC);
  Serial.println(steps, DEC);
   
  for(int i=0;i<steps;i++)
  {
    digitalWrite(stpPin, HIGH);
    delayMicroseconds(800);
    digitalWrite(stpPin, LOW);
    delayMicroseconds(800); 
  }
}

void loop()
{
  int cm = 20;

  step_in_cm(sens, cm);
  
  sens = !sens;
  delay(2000);
}
