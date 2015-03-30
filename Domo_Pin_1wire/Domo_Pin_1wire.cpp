
/*
 * 
 * CMD_1WIRE: 
 *      cmd[1]|type[1]|pin[1]                               - 1wire count, list
 *      cmd[1]|type[1]|pin[1]|addr[8]                       - 1wire read
 *      cmd[1]|type[1]|pin[1]|addr[8]|val[n]                - 1wire write
 * 
 * output:
 *      cmd[1]|type[1]|pin[1]|val[1]                        - 1wire count
 *      cmd[1]|type[1]|pin[1]|val[8] {n}                    - 1wire list
 *      cmd[1]|type[1]|pin[1]|addr[8]|val[n]                - 1wire value
 * 
 */


#include <Domo_Pin.h>

#include <OneWire.h>
#include <DS18B20.h>
#include <DS2413.h>
#include <DS2408.h>

byte Domo_Pin::cmd_1Wire(byte *buf, byte len){
    byte val = 0;
    

    byte pin = buf[2];

    byte addr[8];
    OneWire net(pin);
    
    if(buf[1] == CMD_TYPE_1WIRE_COUNT){
        byte nind = 0;
        
        while(net.search(addr)){
            nind++;
        }
        val = nind;
        buf[3] = val;
        
    } else if(buf[1] == CMD_TYPE_1WIRE_LIST){
        // processed in Domo_Pin::process
        
    } else if(buf[1] == CMD_TYPE_1WIRE_READ){
        
        memcpy(addr, buf+3, 8);
        
        if(OneWire::crc8(addr, 7) != addr[7]){      // crc not valid
            for(int i=11; i<len; i++) buf[i] = 0xFF;
        } else{
            if(addr[0] == 0x28){                // DS18B20
                float value = DS18B20::getTemperature(pin, addr);
                value = DS18B20::getTemperature(pin, addr);
                curValueSize = sizeof(float);
                memcpy(curValue, (byte *)&value, curValueSize);
                memcpy(buf+11, (byte *)&value, curValueSize);
                buf[1] = CMD_TYPE_1WIRE_VALUE;
            } else if(addr[0] == 0x3A){         // DS2413                
                byte a, b;
                DS2413::digitalRead(pin, addr, a, b);
                val = (b<<1) | a;
                buf[1] = CMD_TYPE_1WIRE_VALUE;
                buf[11] = val;
            } else if(addr[0] == 0x29){         // DS2408
                byte data;
                DS2408::digitalRead(pin, addr, data);
                val = data;
                buf[1] = CMD_TYPE_1WIRE_VALUE;
                buf[11] = data;
            } else{

            }
        }
        
    } else if(buf[1] == CMD_TYPE_1WIRE_WRITE){
        
        memcpy(addr, buf+3, 8);
        
        if(OneWire::crc8(addr, 7) != addr[7]){      // crc not valid
            for(int i=11; i<len; i++) buf[i] = 0xFF;
        } else{
            if(addr[0] == 0x3A){                // DS2413                
                byte values = buf[11];
                DS2413::digitalWrite(pin, addr, values&0x1, (values>>1)&0x1);

                byte a, b;
                DS2413::digitalRead(pin, addr, a, b);
                val = (b<<1) | a;
                buf[1] = CMD_TYPE_1WIRE_VALUE;
                buf[11] = val;
                
            } else if(addr[0] == 0x29){         // DS2408
                byte data = buf[11];
                
                DS2408::digitalWrite(pin, addr, data);
                
                byte val = 0;
                DS2408::digitalRead(pin, addr, val);
                //byte val = data;
                
                buf[1] = CMD_TYPE_1WIRE_VALUE;
                buf[11] = val;
            } else{
                
            }
        }
    }
    
    return val;
}

