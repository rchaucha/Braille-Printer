#ifndef TSERIAL_H
#define TSERIAL_H

#include <stdio.h>
#include <windows.h>
#include <string>


enum serial_parity  { spNONE,    spODD, spEVEN };


class Tserial
{
protected:
    int               rate;                          // baudrate
    serial_parity     parityMode;
    HANDLE            serial_handle;                 // ...

public:
                  Tserial();
                 ~Tserial();
    int           connect          (const unsigned char port_arg, int rate_arg, serial_parity parity_arg);
    bool          sendChar         (char c);
    bool          sendArray        (char *buffer, int len);
    char          getChar          (void);
    int           getArray         (char *buffer, int len);
    int           getNbrOfBytes    (void);
    void          disconnect       (void);

private:
    std::wstring sToWs(const std::string& s);
};

#endif TSERIAL_H


