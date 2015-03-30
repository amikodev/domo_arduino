
/*
 * 
 * CMD_PIN: 
 *      cmd[1]|type[1]|pin[1]           - pin mode (input, output), read
 *      cmd[1]|type[1]|pin[1]|val[1]    - pin write
 * 
 * 
 * output:
 *      cmd[1]|type[1]|pin[1]|val[1]    - pin mode (input, output), value
 * 
 */


#include <Domo_Pin.h>

byte Domo_Pin::cmd_Pin(byte *buf, byte len){
    byte val = 0;
    
    byte pin = buf[2];
    if(buf[1] == CMD_TYPE_PIN_MODE_INPUT){
        pinMode(pin, INPUT);
    } else if(buf[1] == CMD_TYPE_PIN_MODE_OUTPUT){
        pinMode(pin, OUTPUT);
    } else if(buf[1] == CMD_TYPE_PIN_READ){
        val = digitalRead(pin);
        buf[1] = CMD_TYPE_PIN_VALUE;
        buf[3] = val;
    } else if(buf[1] == CMD_TYPE_PIN_WRITE){
        digitalWrite(pin, buf[3]);
        val = digitalRead(pin);
        buf[1] = CMD_TYPE_PIN_VALUE;
        buf[3] = val;
    }
    
    return val;
}

