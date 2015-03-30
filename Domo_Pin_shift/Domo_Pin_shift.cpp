
/*
 * 
 * CMD_SHIFT:
 *      cmd[1]|type[1]|dataPin[1]|clockPin[1]|latchPin[1]|size[1]|data[n]   - shift write (0<=n<=10)
 *      cmd[1]|type[1]|dataPin[1]|clockPin[1]|latchPin[1]|size[1]           - shift read
 * 
 * output:
 *      cmd[1]|type[1]|dataPin[1]|clockPin[1]|latchPin[1]|size[1]|data[n]   - shift value (0<=n<=10)
 * 
 */


#include <Domo_Pin.h>

byte Domo_Pin::cmd_Shift(byte *buf, byte len){
    byte val = 0;
    
    if(buf[1] == CMD_TYPE_SHIFT_READ){
        byte dataPin = buf[2];
        byte clockPin = buf[3];
        byte latchPin = buf[4];
        byte size = buf[5];

        pinMode(dataPin, INPUT);
        pinMode(clockPin, OUTPUT);
        pinMode(latchPin, OUTPUT);

        digitalWrite(latchPin, LOW);

        for(byte i=0; i<size; i++){
            byte dat = shiftIn(dataPin, clockPin, MSBFIRST);
            buf[6+i] = dat;
        }
        
        digitalWrite(latchPin, HIGH);
        
        buf[1] = CMD_TYPE_SHIFT_VALUE;
        
        curValueSize = size;
        memcpy(curValue, buf+6, curValueSize);
    } else if(buf[1] == CMD_TYPE_SHIFT_WRITE){
        byte dataPin = buf[2];
        byte clockPin = buf[3];
        byte latchPin = buf[4];
        byte size = buf[5];

        pinMode(dataPin, OUTPUT);
        pinMode(clockPin, OUTPUT);
        pinMode(latchPin, OUTPUT);
        
        digitalWrite(latchPin, LOW);

        for(byte i=0; i<size; i++){
            byte dat = buf[5+size-i];
            shiftOut(dataPin, clockPin, MSBFIRST, dat);
        }
        
        digitalWrite(latchPin, HIGH);
        
        buf[1] = CMD_TYPE_SHIFT_VALUE;

        curValueSize = size;
        memcpy(curValue, buf+6, curValueSize);
    }
    
    return val;
}

