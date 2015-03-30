
#include <Domo_Pin.h>

#include <stdarg.h>


#ifdef DOMO_MODULE_CONTROLLER
#include <Domo_Pin_controller.cpp>
#endif

#ifdef DOMO_MODULE_PIN
#include <Domo_Pin_pin.cpp>
#endif

#ifdef DOMO_MODULE_1WIRE
#include <DS18B20.h>
#include <DS2413.h>
#include <Domo_Pin_1wire.cpp>
#endif

#ifdef DOMO_MODULE_NFC
#include <Domo_Pin_nfc.cpp>
#endif

#ifdef DOMO_MODULE_RADIO
#include <Domo_Pin_radio.cpp>
#endif

#ifdef DOMO_MODULE_DHT
#include <Domo_Pin_dht.cpp>
#endif

#ifdef DOMO_MODULE_SHIFT
#include <Domo_Pin_shift.cpp>
#endif




/*
#ifdef DOMO_MODULE_DS18B20
#include "DS18B20.h"
#endif

#ifdef DOMO_MODULE_DS2413
#include "DS2413.h"
#endif
*/



bool Domo_Pin::process(byte *buf, byte len){
    
    bool ret = false;
    
    curValueSize = 0;
    
    byte command = buf[0];
    if(command == CMD_CONTROLLER){
#ifdef DOMO_MODULE_CONTROLLER
        int value = cmd_Controller(buf, len);
        curValueSize = sizeof(int *);
        memcpy(curValue, (byte *)value, curValueSize);
        ret = true;
#endif
    } else if(command == CMD_PIN){
#ifdef DOMO_MODULE_PIN
        byte value = cmd_Pin(buf, len);
        curValueSize = sizeof(byte *);
        memcpy(curValue, (byte *)value, curValueSize);
        ret = true;
#endif
    } else if(command == CMD_1WIRE){
#ifdef DOMO_MODULE_1WIRE
        
        if(buf[1] == CMD_TYPE_1WIRE_LIST){
            byte addr[8];
            byte pin = buf[2];
            OneWire net(pin);

            byte nind = 0;

            while(net.search(addr)){
                memcpy(buf+3, addr, 8);
                Serial.write(buf, 16);
                nind++;
            }
            
            ret = true;
        } else{
            byte value = cmd_1Wire(buf, len);
            curValueSize = sizeof(byte *);
            memcpy(curValue, (byte *)value, curValueSize);
            ret = true;
        }
        
#endif
    } else if(command == CMD_NFC){
#ifdef DOMO_MODULE_NFC
        byte value = cmd_Nfc(buf, len);
        curValueSize = sizeof(byte *);
        memcpy(curValue, (byte *)value, curValueSize);
        ret = true;
#endif
    } else if(command == CMD_RADIO){
#ifdef DOMO_MODULE_RADIO
        byte value = cmd_Radio(buf, len);
        //curValueSize = sizeof(byte *);
        //memcpy(curValue, (byte *)value, curValueSize);
        ret = true;
#endif
    } else if(command == CMD_DHT){
#ifdef DOMO_MODULE_DHT
        byte value = cmd_Dht(buf, len);
        curValueSize = sizeof(byte *);
        memcpy(curValue, (byte *)value, curValueSize);
        ret = true;
#endif
    } else if(command == CMD_SHIFT){
#ifdef DOMO_MODULE_SHIFT
        byte value = cmd_Shift(buf, len);
        //curValueSize = sizeof(byte *);
        //memcpy(curValue, (byte *)value, curValueSize);
        ret = true;
#endif
    } else if(command == CMD_LISTEN){
        
        byte i = 15;
        for(i=15; i>0; i--){
            if(buf[i] != 0x00) break;
        }
        
        void *nlisten = malloc(i+1);
        ((byte *)nlisten)[0] = i;
        memcpy(nlisten+1, buf+1, i);
        listens[listenCount] = (byte *)nlisten;

        byte cmd2 = buf[1];
        byte valueSize = 0;
        if(cmd2 == CMD_CONTROLLER) valueSize = sizeof(int *);
        else if(cmd2 == CMD_PIN) valueSize = sizeof(byte *);
        else if(cmd2 == CMD_1WIRE) valueSize = sizeof(byte *);

        //if(valueSize > 0){
        void *nval = malloc(valueSize+1);
        ((byte *)nval)[0] = valueSize;
        for(int i=0; i<valueSize; i++) ((byte *)nval)[i+1] = 0;
        //}
        listenValues[listenCount] = (byte *)nval;
        
        listenCount++;

        ret = false;
    } else if(command == CMD_FREERAM){
        int value = cmd_FreeRam(buf, len);
        curValueSize = sizeof(int *);
        memcpy(curValue, (byte *)value, curValueSize);
        ret = true;
    } else{     // ошибочная команда
        for(int i=1; i<len; i++){
            buf[i] = 0xFF;
        }
        ret = true;
    }
    
    return ret;
    
}

bool Domo_Pin::process(byte n, ...){
    
    byte bufc[16] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

    va_list args;
    va_start(args, n);
    
    int p;
    for(int i=0; i<n && i<sizeof(bufc); i++){
        p = va_arg(args, int);
        bufc[i] = (byte) p;
    }
    va_end(args);
    
    bool r = process(bufc, sizeof(bufc));
    if(r == true){
        Serial.write(bufc, sizeof(bufc));
    }    
    return r;
}




void Domo_Pin::run(){
    
    if(Serial.available() > 0){

        for(int i=0; i<sizeof(bufc); i++) bufc[i] = 0;
        Serial.readBytes((char *)bufc, 16);

        bool r = process(bufc, sizeof(bufc));

        if(r == true){
            Serial.write(bufc, sizeof(bufc));
        } else{
            //for(int i=1; i<sizeof(bufc); i++) bufc[i] = 0xFF;
            //Serial.write(bufc, sizeof(bufc));
        }
        
        
        
    }

#ifdef DOMO_MODULE_RADIO
    if(dradio){
        ((Domo_Pin_RF24 *)dradio)->process();
    }
#endif
    
#ifdef DOMO_MODULE_NFC
    if(nfc){
        process_nfc();
    }
#endif    
    
    
    processListens();
    
}




