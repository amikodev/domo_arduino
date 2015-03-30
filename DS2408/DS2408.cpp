
#include <OneWire.h>

#include "DS2408.h"

void DS2408::digitalRead(int pin, byte addr[], byte& data){
    OneWire ds(pin);
    ds.reset();
    ds.select(addr);
    ds.write(0xF5, 1);
    data = ds.read();
    
    ds.reset();
}

void DS2408::digitalWrite(int pin, byte addr[], byte data){
    //byte cmd;
    
    OneWire ds(pin);
    ds.reset();
    ds.select(addr);
    ds.write(0x5A, 1);
    ds.write(data, 1);
    ds.write(~data, 1);

    
    byte ack = ds.read();
    if(ack == 0xAA){
        ds.read();
    }
    
    ds.reset();
}







