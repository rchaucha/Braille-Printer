#include "MeOrion.h"

// Me_LimitSwitch module can only be connected to PORT_3, PORT_4, PORT_6, PORT_7, 
// PORT_8 of base shield or from PORT_3 to PORT_8 of baseboard.

MeLimitSwitch limitSwitchY1(PORT_3, 1),
              limitSwitchY2(PORT_3, 2),
              limitSwitchX1(PORT_6, 1),
              limitSwitchX2(PORT_6, 2);

int dirPinX = mePort[PORT_1].s1;  //the direction pin connect to Base Board PORT1 SLOT1
int stpPinX = mePort[PORT_1].s2;  //the Step pin connect to Base Board PORT1 SLOT2

int dirPinY = mePort[PORT_2].s1;  //the direction pin connect to Base Board PORT1 SLOT1
int stpPinY = mePort[PORT_2].s2;  //the Step pin connect to Base Board PORT1 SLOT2


bool isTouched()
{
  return (limitSwitchY1.touched() || limitSwitchY2.touched() || limitSwitchX1.touched() || limitSwitchX2.touched());
}

void makeStep(int pin, unsigned int delayInMicroseconds)
{
  digitalWrite(pin, HIGH);
  delayMicroseconds(delayInMicroseconds);
  digitalWrite(pin, LOW);
  delayMicroseconds(delayInMicroseconds); 
}

void countSteps()
{
  while(!Serial.available() > 0){}
  
  digitalWrite(dirPinX, 0);
  delay(5);

  unsigned int stepsX = 13646,  //valeurs expérimentales : calcul du nbr de pas pour faire la longueur totale, puis divisés par 2
               stepsY = 17011;
  
  while(stepsX > 0 && !isTouched()) //tant qu'on n'a pas atteint le bord gauche, on fait un pas
  {
    stepsX--;
    makeStep(stpPinX, 50);
  }
  
  digitalWrite(dirPinY, 1);
  delay(5);
  
  while(stepsY > 0 && !isTouched()) //tant qu'on n'a pas atteint le bord supérieur, on fait un pas
  {
    stepsY--;
    makeStep(stpPinY, 50);
  }
}

void setup()
{
  Serial.begin(9600);
  Serial.println("Start");

  pinMode(dirPinX, OUTPUT);
  pinMode(stpPinX, OUTPUT); 
  
  pinMode(dirPinY, OUTPUT);
  pinMode(stpPinY, OUTPUT);
  
  countSteps();
}

void loop()
{
  if(isTouched())      // If the limit switch is touched, the  return value is true.
  {
    Serial.println("State: DOWN.");
    delay(1);
    while(isTouched())
    {
      ;// Repeat check the switch state, until released.
    }
    delay(2);
  }
  if(!isTouched())
  {
    Serial.println("State: UP.");
    delay(1);
    while(!isTouched())
    {
      ;
    }
    delay(2);
  }
}
