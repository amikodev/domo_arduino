
#include <OneWire.h>

#include "DS2413.h"

void DS2413::digitalRead(int pin, byte addr[], byte& a, byte& b){
    OneWire ds(pin);
    ds.reset();
    ds.select(addr);
    ds.write(0xF5);
    byte data = ds.read();
    
    a = data & 0x1;
    b = (data & 0x4) >> 2;
    
    ds.reset();
}

void DS2413::digitalWrite(int pin, byte addr[], boolean a, boolean b){
    byte cmd;
    cmd = 0xFC;
    if(a) cmd |= 0x1;
    if(b) cmd |= 0x2;
    
    OneWire ds(pin);
    ds.reset();
    ds.select(addr);
    ds.write(0x5A);
    ds.write(cmd);
    ds.write(~cmd);
    
    byte ack = ds.read();
    if(ack == 0xAA){
        ds.read();
    }
    
    ds.reset();
}







