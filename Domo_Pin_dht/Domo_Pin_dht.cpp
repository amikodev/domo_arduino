
/*
 * CMD_DHT:
 *      cmd[1]|type[1]|pin[1]           - dht read
 * 
 * 
 * output:
 *      cmd[1]|type[1]|pin[1]|temp[1]|hum[1]    - dht value
 *      cmd[1]|type[1]|pin[1]|code[1]           - dht error
 * 
 */


#include <Domo_Pin.h>

#include "DHT.h"

byte Domo_Pin::cmd_Dht(byte *buf, byte len){
    byte val = 0;
    
    byte pin = buf[2];
    if(buf[1] == CMD_TYPE_DHT_READ){

        DHT *sensor = new DHT();
        sensor->attach(A0+pin);
        
        sensor->update();
        
        byte err = sensor->getLastError();
        if(err == 0){
            buf[1] = CMD_TYPE_DHT_VALUE;
            buf[3] = sensor->getTemperatureInt();
            buf[4] = sensor->getHumidityInt();
        } else{
            buf[1] = CMD_TYPE_DHT_ERROR;
            buf[3] = err;
        }
        
    }
    
    return val;
}

