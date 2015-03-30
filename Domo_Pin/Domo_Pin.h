
#include "Arduino.h"

#include <Wire.h>
#include <stdlib.h>

#ifndef Domo_Pin_h
#define Domo_Pin_h

/*

config.h :

#define DOMO_MODULE_CONTROLLER
#define DOMO_MODULE_PIN
#define DOMO_MODULE_1WIRE
#define DOMO_MODULE_NFC
#define DOMO_MODULE_RADIO
#define DOMO_MODULE_DHT
#define DOMO_MODULE_SHIFT

*/

#define CMD_CONTROLLER 0x01
#define CMD_PIN 0x02
#define CMD_1WIRE 0x03
#define CMD_REGIN 0x04
#define CMD_REGOUT 0x05
#define CMD_NFC 0x06
#define CMD_RADIO 0x07
#define CMD_DHT 0x08
#define CMD_SHIFT 0x09

#define CMD_LISTEN 0x80
#define CMD_FREERAM 0x81


#define CMD_TYPE_PIN_MODE_INPUT 0x01
#define CMD_TYPE_PIN_MODE_OUTPUT 0x02
#define CMD_TYPE_PIN_READ 0x03
#define CMD_TYPE_PIN_WRITE 0x04
#define CMD_TYPE_PIN_VALUE 0x05

#define CMD_TYPE_1WIRE_COUNT 0x01
#define CMD_TYPE_1WIRE_LIST 0x02
#define CMD_TYPE_1WIRE_READ 0x03
#define CMD_TYPE_1WIRE_WRITE 0x04
#define CMD_TYPE_1WIRE_VALUE 0x05

#define CMD_TYPE_NFC_INIT 0x01
#define CMD_TYPE_NFC_FIRMWARE 0x02
#define CMD_TYPE_NFC_ERROR 0x03
#define CMD_TYPE_NFC_RECIEVE 0x04
#define CMD_TYPE_NFC_WRITE 0x05

#define CMD_ERROR_NFC_NONE 0x00
#define CMD_ERROR_NFC_DEVICENOTFOUND 0x01
#define CMD_ERROR_NFC_CARDNOTVALID 0x02
#define CMD_ERROR_NFC_WRONGDATA 0x03
#define CMD_ERROR_NFC_NOTAUTH 0x04
#define CMD_ERROR_NFC_NOTWRITE 0x05

#define CMD_TYPE_RADIO_INIT 0x01
#define CMD_TYPE_RADIO_WPIPE 0x02
#define CMD_TYPE_RADIO_RPIPE 0x03
#define CMD_TYPE_RADIO_CHKEYS 0x04
#define CMD_TYPE_RADIO_SEND 0x05
#define CMD_TYPE_RADIO_RESPONCE 0x06
#define CMD_TYPE_RADIO_K128 0x07

#define CMD_TYPE_DHT_READ 0x01
#define CMD_TYPE_DHT_VALUE 0x02
#define CMD_TYPE_DHT_ERROR 0x03

#define CMD_TYPE_SHIFT_READ 0x01
#define CMD_TYPE_SHIFT_WRITE 0x02
#define CMD_TYPE_SHIFT_VALUE 0x03


class Domo_Pin{
    
private:
    int controllerID;
    
    byte *listens[16];
    byte *listenValues[16];
    int listenCount;
    
    byte curValue[8];   // double size
    int curValueSize = 0;
    
    
    byte bufc[16];
    byte bufCmd[16];
    byte bufCmdLen = 0;
    
    void *radio;
    void *dradio;
    void *nfc;
    
public:
    Domo_Pin();

    void run();
    bool process(byte *buf, byte len);
    bool process(byte n, ...);
    
    void processListens();
    void setControllerID(int id);
    int getControllerID();
    String float_to_string(double value);
    void print_value(char * str, byte * a, int bits);

    int freeRam();
    
    int  cmd_Controller(byte *buf, byte len);
    byte cmd_Pin(byte *buf, byte len);
    byte cmd_1Wire(byte *buf, byte len);
    byte cmd_Nfc(byte *buf, byte len);
    byte cmd_Radio(byte *buf, byte len);
    byte cmd_Dht(byte *buf, byte len);
    byte cmd_Shift(byte *buf, byte len);
    
    int  cmd_FreeRam(byte *buf, byte len);
    
    void radio_recieve(byte pipe_num, byte *message, byte length);
    void process_nfc();
    
};

#endif



