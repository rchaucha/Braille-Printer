#include "ArduinoConnectionManager.h"

#include <string>
#include <iostream>
#include <thread>
#include <QDebug>
#include "tserial.h"

using namespace std;

ArduinoConnectionManager::ArduinoConnectionManager()
{
	_serial_com_ptr = new Tserial();
}

bool ArduinoConnectionManager::connect(const unsigned char com_port)
{
    bool is_connected = _startDevice(com_port, 9600); //On ouvre la connection avec l'Arduino branché sur le port COM défini par l'utilisateur à 9600 bits / sec
	
	return is_connected;
}

void ArduinoConnectionManager::disconnect()
{
    sendBitSet(bitset<12>("000010000010"));

    this_thread::sleep_for(std::chrono::milliseconds(50));

    sendBitSet(bitset<12>("000100000100")); //EndOfTransmission x2

    this_thread::sleep_for(std::chrono::milliseconds(50));

    sendChar(1);

    this_thread::sleep_for(std::chrono::milliseconds(10));

    sendChar(1);

    _serial_com_ptr->disconnect();

    delete _serial_com_ptr;
    _serial_com_ptr = nullptr;
}

bool ArduinoConnectionManager::sendBitSet(bitset<12> full_bitset)
{
    bitset<6> first_bitset_part;
	bitset<6> second_bitset_part;

    for (unsigned char i = 0; i < 6; i++)           //On sépare le bitset<12> en deux bitset<6> pour
    {                                               //les envoyer par octet
        first_bitset_part[i] = full_bitset[i];
        second_bitset_part[i] = full_bitset[i + 6];
	}

    unsigned char first_bitset_part_c = static_cast<unsigned char>(first_bitset_part.to_ulong());   //conversion de bitset -> char (=octet)
    unsigned char second_bitset_part_c = static_cast<unsigned char>(second_bitset_part.to_ulong());

    bool success1 = _serial_com_ptr->sendChar(first_bitset_part_c);     //on envoie les deux octets, et on récupère le résultat des opérations

	Sleep(7);

    bool success2 = _serial_com_ptr->sendChar(second_bitset_part_c);

    return (success1 && success2);          //on fait un ET sur les deux booléens pour savoir si une des opérations a échoué ou non
}

void ArduinoConnectionManager::sendChar(char data)
{
	_serial_com_ptr->sendChar(data);
}

bool ArduinoConnectionManager::_startDevice(const unsigned char port, const int speed)
{
	bool is_connected = true;

    if (_serial_com_ptr != 0)
	{
		int error = _serial_com_ptr->connect(port, speed, spNONE);
		if (error) //si ERROR est différent de 0 (Si il y a une erreur) 
		{
			printf("La connection a échouée.\n");

			printf("Erreur : %i\n", error); //on affiche l'erreur

            qDebug() << "Erreur : " << error << " :)" << endl;

			is_connected = false;
		}
	}
    else
	{
        is_connected = false;

        qDebug() << "Erreur : " << '9' << " :)" << endl;
	}

	return is_connected;
}
