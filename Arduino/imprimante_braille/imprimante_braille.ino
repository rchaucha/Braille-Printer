#include <Servo.h>
#include <StandardCplusplus.h>
#include <vector>
#include <algorithm>
#include "MeOrion.h"

using namespace std;

#define DEFAULT_DELAY 800
#define INIT_VALUE    127     //127(dec) = 111 1111(bin), impossible à obtenir car les données reçues sont codées sur 6 bits.
#define SERVO_HAUT    130
#define SERVO_BAS     100

enum MotorsList {X, Y};

int dirPinX = mePort[PORT_1].s1;  //the direction pin connect to Base Board PORT1 SLOT1
int stpPinX = mePort[PORT_1].s2;  //the Step pin connect to Base Board PORT1 SLOT2

int dirPinY = mePort[PORT_2].s1;  //the direction pin connect to Base Board PORT1 SLOT1
int stpPinY = mePort[PORT_2].s2;  //the Step pin connect to Base Board PORT1 SLOT2

MePort servo_port(PORT_7);
Servo servo;
int16_t servo_pin = servo_port.pin2();

MeLimitSwitch limitSwitchY1(PORT_3, 1),
              limitSwitchY2(PORT_3, 2),
              limitSwitchX1(PORT_6, 1),
              limitSwitchX2(PORT_6, 2);

vector<int> all_characters;

///FONCTIONS///
 void clearSerialBuffer();
 void getHome();
 void connection();
 void centerSystem();
 void waitForTransmission();
 void receiveMessage();
 void receiveMessageDimensions(char (&msg_dimensions)[2]);
 void initPosition(char const (&msg_dimensions)[2]);
 void printMessage();
 int  bytesCombiner(unsigned char part1, unsigned char part2);
 bool isFirstPartNull(int char_to_study);
 bool makeStepsFromMm(MotorsList motor, bool dir, float mm, unsigned int delayInMicroseconds);
 void makeStep(int pin, unsigned int delayInMicroseconds);
 void servoCommand(int current_int, short i);
 void lineWrapping(char last_line_length);
 void abortPrinting();
///////////////

void setup()
{
  Serial.begin(9600);

  servo.attach(servo_pin);
  servo.write(SERVO_BAS);
  
  pinMode(dirPinX, OUTPUT);
  pinMode(stpPinX, OUTPUT);
  
  pinMode(dirPinY, OUTPUT);
  pinMode(stpPinY, OUTPUT);
}

void loop()
{    
  all_characters.clear();
  clearSerialBuffer();
  
  getHome();
  
  connection();
    
  centerSystem();
  
  waitForTransmission();
  
  receiveMessage();

  int msg_dimensions[2];
  receiveMessageDimensions(msg_dimensions);

  initPosition(msg_dimensions);
  
  printMessage();
}

void clearSerialBuffer()
{
  while(Serial.available() > 0)
    Serial.read();
}

void getHome()
{
  digitalWrite(dirPinX, 0);
  delay(5);
  
  while(!limitSwitchX2.touched()) //tant qu'on n'a pas atteint le bord droite, on fait un pas
  {
    makeStep(stpPinX, 50);
  }
  
  digitalWrite(dirPinY, 0);
  delay(5);
  
  while(!limitSwitchY2.touched()) //tant qu'on n'a pas atteint le bord supérieur, on fait un pas
  {
    makeStep(stpPinY, 50);
  }
}

void connection()
{
  char connection1 = INIT_VALUE,
       connection2 = INIT_VALUE;
	   
  bool is_connection_established = false;
	
  while(!is_connection_established)
  {
    if (Serial.available() > 0) //si un caratère est reçu
    {
      if(connection1 == INIT_VALUE) //si c'est le 1er
        connection1 = Serial.read();
      else
      {
        connection2 = Serial.read();

        if((connection1 == B111111) && (connection2 == B111111))   //si le caractère reçu est le caratère 111111 on engage la connection
           is_connection_established = true;
        else
        {
          connection1 = INIT_VALUE;
          connection2 = INIT_VALUE;
        }
      }
    }
    delay(100);
  }
  clearSerialBuffer();

  for(int i = 0; i < 6; i++)
  {
    servoCommand(B111111, i);

    unsigned int stepsY = 100;
    
    digitalWrite(dirPinY, 1);
    delay(5);
    
    while(stepsY > 0) //tant qu'on n'a pas atteint le milieu, on fait un pas
    {
      stepsY--;
      makeStep(stpPinY, 50);
    }

    makeStepsFromMm(MotorsList::Y, 1, 2, DEFAULT_DELAY);
  }
}

void centerSystem()
{
  getHome(); //pour être certain que la tête est rangée avant d'être centrée
  
  unsigned int stepsX = 13646,  //valeurs expérimentales : calcul du nbr de pas pour faire la longueur totale, puis divisés par 2
               stepsY = 17011;
  
  digitalWrite(dirPinY, 1);
  delay(5);
  
  while(stepsY > 0) //tant qu'on n'a pas atteint le milieu, on fait un pas
  {
    stepsY--;
    makeStep(stpPinY, 50);
  }

  digitalWrite(dirPinX, 1);
  delay(5);
  
  while(stepsX > 0) //tant qu'on n'a pas atteint le milieu, on fait un pas
  {
    stepsX--;
    makeStep(stpPinX, 50);
  }
}

void waitForTransmission()
{
  char initialization1 = INIT_VALUE,
       initialization2 = INIT_VALUE;

  bool is_transmission_began = false;
  
  while(!is_transmission_began)     //En attente de l'initialisation de la transmission
  {
    if (Serial.available() > 0) //si un caratère est reçu
    {
      if(initialization1 == INIT_VALUE) //si c'est le 1er
        initialization1 = Serial.read();
      else
      {
        initialization2 = Serial.read();

        if((initialization1 == B0000010) && (initialization2 == B0000010))   //si le caractère reçu est le caratère SOT (start of text, 000010) on débute la transmission
           is_transmission_began = true;
        else
        {
          initialization1 = INIT_VALUE;
          initialization2 = INIT_VALUE;
        }
      }
    }
    delay(10);
  }
  clearSerialBuffer();
}

void receiveMessage()
{
  char part1 = INIT_VALUE,
       part2 = INIT_VALUE;
	   
  bool is_msg_entirely_received  = false;
	   
  while(!is_msg_entirely_received)
  {
    if (Serial.available() > 0) //si un caratère est reçu
    { 
      if(part1 == INIT_VALUE) //si c'est le 1er
      {
        part1 = Serial.read(); 
      }
      else
      {
        part2 = Serial.read();

        if((part1 == B000100) && (part2 == B000100))   //si le caractère reçu est le caratère EOT (end of transmission, 0000100) en deuxième caratère aussi, on arrête la transmission
        {
           is_msg_entirely_received = true;
        }
        else
        {                
          all_characters.push_back(bytesCombiner(part1, part2)); //on ajoute la combination des 2 parties du caractère converties en int dans le vector.
  
          part1 = INIT_VALUE;      //on réinitialise les 2 parties du caratère une fois traitées
          part2 = INIT_VALUE;
        }
      }
    }
    delay(5);
  }
  clearSerialBuffer();
}

void receiveMessageDimensions(char (&msg_dimensions)[2])
{
  while(Serial.available() <= 0){}
  msg_dimensions[0] = Serial.read();
  clearSerialBuffer();

  delay(5);

  while(Serial.available() <= 0){}
  msg_dimensions[1] = Serial.read();
  clearSerialBuffer();
}

void initPosition(char const (&msg_dimensions)[2])
{
  char longest_line = msg_dimensions[0],
       lines_nbr    = msg_dimensions[1];
  
  if(lines_nbr != 0 && longest_line != 0)
  {
    float heightInMm = 0,
          widthInMm  = 0;

    if(lines_nbr % 2 == 0)  //si on a un nombre de ligne pair
      heightInMm = 10 * (lines_nbr/2 - 1) + 7.5;
    else
      heightInMm =  5 * (lines_nbr - 1) + 2.5;
      
    if(longest_line % 2 == 0)  //si on a un nombre de caractère pair
      widthInMm = 5 * (longest_line/2 - 1) + 4.5;
    else
      widthInMm = 2.5 * (longest_line - 1) + 1.25;
  
    if (!makeStepsFromMm(MotorsList::Y, 0, heightInMm, 70)) 
      abortPrinting();
  
    if (!makeStepsFromMm(MotorsList::X, 1, widthInMm, 70))
      abortPrinting();
  }
}

void printMessage()
{
  char last_line_length = 0;

  for(int j = 0; j < all_characters.size(); j++)
  {    
    last_line_length++;
    
    int current_int = all_characters[j];

    if(current_int == 0) //si le caractère est une espace
    {
      makeStepsFromMm(MotorsList::X, 0, 6.0, DEFAULT_DELAY);
    }
    else if (current_int == 845) //si le caratère est un retour à la ligne : 845(10) = 001101001101(2)
    {
      last_line_length--; //on enlève le caractère \n du compte
      lineWrapping(last_line_length);
    }
    else
    {
      if(!isFirstPartNull(current_int)) //si la première partie du caractère ne vaut pas 000000, on l'imprime
      {
        for(byte i = 5; i >= 3; i--)          //points 1->3
        {
          if(i != 5)
            makeStepsFromMm(MotorsList::Y, 1, 2.5, DEFAULT_DELAY);
          servoCommand(current_int, i); 
        }
        makeStepsFromMm(MotorsList::X, 0, 2.5, DEFAULT_DELAY);
        
        for(byte i = 0; i < 3; i++)         //points 6->4
        {
          if(i != 0)
            makeStepsFromMm(MotorsList::Y, 0, 2.5, DEFAULT_DELAY);
          servoCommand(current_int, i); 
        }
        makeStepsFromMm(MotorsList::X, 0, 3.5, DEFAULT_DELAY);
      }
  
      for(byte i = 11; i >= 9; i--)            //points 7->9
      {
        if(i != 11)
          makeStepsFromMm(MotorsList::Y, 1, 2.5, DEFAULT_DELAY);
        servoCommand(current_int, i);   
      }
      makeStepsFromMm(MotorsList::X, 0, 2.5, DEFAULT_DELAY);
      
      for(byte i = 6; i < 9; i++)          //points 12->10
      {
        if(i != 6)
          makeStepsFromMm(MotorsList::Y, 0, 2.5, DEFAULT_DELAY);
        servoCommand(current_int, i); 
      }
      makeStepsFromMm(MotorsList::X, 0, 3.5, DEFAULT_DELAY);
    }
  }
}

int bytesCombiner(unsigned char part1, unsigned char part2)    //combine les 2 octets (ex: '1010 1010' et '1111 1111' => '1010 1010 1111 1111')
{
  int combined = part1;
  combined = combined << 6; //bits shift
  combined |= part2;        //OR gate
  
  return combined;
}

bool isFirstPartNull(int char_to_study)
{
  return !(bitRead(char_to_study, 0) || 
           bitRead(char_to_study, 1) || 
           bitRead(char_to_study, 2) || 
           bitRead(char_to_study, 3) || 
           bitRead(char_to_study, 4) || 
           bitRead(char_to_study, 5));    //si un des bits de la première partie vaut 1 on retourne FALSE
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

bool makeStepsFromMm(MotorsList motor, bool dir, float mm, unsigned int delayInMicroseconds)
{
  /* Retourne true si le nombre de mm entré en argument a pu être effectué, false sinon */

  int stpPin = 0;
  int dirPin = 0;
  
  switch(motor)       //on sélectionne le pin et le sens correspondant en fonction du moteur
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
  
  digitalWrite(dirPin, dir);  //on initialise le moteur
  delay(10);

  int steps = mm * 11200./128.;      //35/128 : valeur expérimentale conversion cm -> tours, puis * 3200 pour tours -> steps

  for(int i = 0; i < steps; i++)
  {
    if(!(limitSwitchY1.touched() || limitSwitchY2.touched() || limitSwitchX1.touched() || limitSwitchX2.touched())) //si aucun capteur fin de course n'est atteint
      makeStep(stpPin, delayInMicroseconds);
    else
      return false;
  }
  return true;
}

void servoCommand(int current_int, short i)
{
  if(bitRead(current_int, i))   //si le bit est à 1
  {
    servo.write(SERVO_HAUT);
    delay(150);
  }

  servo.write(SERVO_BAS);
  delay(200);
}

void lineWrapping(char last_line_length)
{
  if (!makeStepsFromMm(MotorsList::Y, 1, 10, 70)) //on descend d'une ligne
    abortPrinting();

  if (!makeStepsFromMm(MotorsList::X, 1, 6 * last_line_length, 70)) //on revient tout à gauche
    abortPrinting();
}

void abortPrinting()
{
  all_characters.clear();
}
