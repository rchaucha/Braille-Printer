#ifndef DEF_ARDUINOCONNECTIONMANAGER
#define DEF_ARDUINOCONNECTIONMANAGER

#include <map>
#include <bitset>

class Tserial;

class ArduinoConnectionManager
{
public:
	ArduinoConnectionManager();

    bool connect(const unsigned char com_port);
	void disconnect();
    bool sendBitSet(std::bitset<12> bitset);
	void sendChar(char data);

private:
    bool _startDevice(const unsigned char port, const int speed);

	Tserial* _serial_com_ptr;
};

#endif
