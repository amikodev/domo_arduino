#ifndef __DomoRF24_h__
#define __DomoRF24_h__

#include <SPI.h>
#include "nRF24L01.h"
#include "RF24.h"

#include "AES.h"

#include "CryptoDH.h"



#define RF24_CHK_P 1
#define RF24_CHK_G 2
#define RF24_CHK_A 3
#define RF24_CHK_B 4
#define RF24_CRYPTED_TEXT 5


typedef void (*funcRF24Resp)(byte *message, byte length);


struct StateDataByte{
    byte state;
    byte data[16];
};
struct StateDataInt{
    byte state;
    unsigned int data[8];
};
struct StateDataLong{
    byte state;
    unsigned long data[4];
};

struct RadioData{
    byte page;
    byte pagesCount;
    byte length;
    byte data[29];
};


class DomoRF24{
    
public:

    funcRF24Resp funcResp;
    
    void init(RF24 *radio, CryptoDH *dh);
    bool send(byte* buf, uint8_t len);
    bool sendCrypt(byte* buf, uint8_t len);
    void dump(byte* buf, uint8_t len, const char* title);
    void print_value(char * str, byte * a, int bits);
    void process();
    
    void changeKeys();
    
private:
    
    RF24 *_radio;
    CryptoDH *_dh;

    unsigned int p128[8], g128[8];
    unsigned long K128[4];

    //boolean 
    
    byte *fullMessage;
    byte fullMessageLength;
    int dataType;
    
    

    
};

#endif

