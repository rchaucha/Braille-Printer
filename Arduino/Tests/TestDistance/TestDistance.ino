#include "MeOrion.h"

#define MILLIMETRES 200
#define LOW_SPEED  800
#define HIGH_SPEED 50

enum MotorsList {X, Y};

int dirPinX = mePort[PORT_1].s1;  //the direction pin connect to Base Board PORT1 SLOT1
int stpPinX = mePort[PORT_1].s2;  //the Step pin connect to Base Board PORT1 SLOT2

int dirPinY = mePort[PORT_2].s1;  //the direction pin connect to Base Board PORT1 SLOT1
int stpPinY = mePort[PORT_2].s2;  //the Step pin connect to Base Board PORT1 SLOT2

MeLimitSwitch limitSwitchY1(PORT_3, 1),
              limitSwitchY2(PORT_3, 2),
              limitSwitchX1(PORT_6, 1),
              limitSwitchX2(PORT_6, 2);

bool makeStepsFromMm(MotorsList motor, bool dir, float mm, unsigned int delayInMicroseconds);
void makeStep(int pin, unsigned int delayInMicroseconds);
void clearSerialBuffer();

void setup()
{
  Serial.begin(9600);
  
  pinMode(dirPinX, OUTPUT);
  pinMode(stpPinX, OUTPUT); 
  
  pinMode(dirPinY, OUTPUT);
  pinMode(stpPinY, OUTPUT);
}

void loop()
{
  Serial.println("Entrez le chiffre correspondant a l'action a effectuer:");
  Serial.println("1: Moteur en X, rapide");
  Serial.println("2: Moteur en X, lent");
  Serial.println("3: Moteur en Y, rapide");
  Serial.println("4: Moteur en Y, lent");

  clearSerialBuffer();	//on s'assure qu'aucun caractère n'est resté dans le buffer du Serial d'Arduino

  bool is_action_performed = true;
  
  while(!is_action_performed)	//tant que le nombre saisi par l'utilisateur est incorrect, on ne fait rien
  {
    if(Serial.available() > 0)	//l'utilisateur a entré un caractère
    {
      char choice = Serial.read() - '0';	//on le stocke dans la variable choice

        switch(choice)	//on effectue l'action correspondante
        {
        case 1:
          makeStepsFromMm(MotorsList::X, 0, MILLIMETRES, HIGH_SPEED);
          break;
        case 2:
          makeStepsFromMm(MotorsList::X, 0, MILLIMETRES, LOW_SPEED);
          break;
        case 3:
          makeStepsFromMm(MotorsList::Y, 0, MILLIMETRES, HIGH_SPEED);
          break;
        case 4:
          makeStepsFromMm(MotorsList::Y, 0, MILLIMETRES, LOW_SPEED);
          break;
        default:
          Serial.println("Veuillez choisir une action correcte"); 
          is_action_performed = false;
          break;
        }
      }
    }
    delay(10);
  }
}

bool makeStepsFromMm(MotorsList motor, bool dir, float mm, unsigned int delayInMicroseconds)
{
  /* Retourne true si le nombre de mm entré en argument a pu être effectué, false sinon */

  Serial.println("La tete d'impression se deplacera de %i mm.", mm);

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
    if(!(limitSwitchY1.touched() || limitSwitchY2.touched() || limitSwitchX1.touched() || limitSwitchX2.touched())) //si aucun capteur fin de course n'est atteint
      makeStep(stpPin, delayInMicroseconds);
    else
      return false;
  }
  return true;
}

void makeStep(int pin, unsigned int delayInMicroseconds)
{
  if(delayInMicroseconds < 50)
    delayInMicroseconds = 50;
  
  digitalWrite(pin, HIGH);
  delayMicroseconds(delayInMicroseconds);
  digitalWrite(pin, LOW);
  delayMicroseconds(delayInMicroseconds); 
}

void clearSerialBuffer()
{
  while(Serial.available() > 0)
    Serial.read();
}
