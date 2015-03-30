

#include "Domo_Pin.h"



Domo_Pin::Domo_Pin(){
    listenCount = 0;
    //outputType = DP_OUTPUT_SERIAL;
}

void Domo_Pin::setControllerID(int id){
    controllerID = id;
}
int Domo_Pin::getControllerID(){
    return controllerID;
}


void Domo_Pin::processListens(){
    for(int i=0; i<listenCount; i++){
        byte buf[16];
        void *listen = listens[i];
        void *listenValue = listenValues[i];
        
        for(int j=0; j<sizeof(buf); j++) buf[j] = 0;
        memcpy(buf, listen+1, ((byte *)listen)[0]);
        
        bool r = process(buf, sizeof(buf));
        
        if(r == true && curValueSize > 0 && ((byte *)listenValue)[0] == curValueSize){
            if(memcmp(listenValue+1, curValue, curValueSize) != 0){
                // вывод изменившегося значения
                // output();
                
                Serial.write(buf, sizeof(buf));
                memcpy(listenValue+1, curValue, curValueSize);
            }
        }
    }
}


int Domo_Pin::cmd_FreeRam(byte *buf, byte len){
    int value = freeRam();
    memcpy(buf+1, (byte *)value, sizeof(int));
    return value;
}


String Domo_Pin::float_to_string(double value){
    char str2[32];
    double val2 = value-(int)value;
    val2=val2*10000;
    sprintf(str2,"%d.%04d",(int)value,(int)val2);
    return str2;
}

void Domo_Pin::print_value(char * str, byte * a, int bits){
    char * hex = "0123456789abcdef";
    Serial.print(str);
    bits >>= 3 ;
    for(int i = 0; i < bits; i++){
        byte b = a[i];
        Serial.print(hex [b >> 4]);
        Serial.print(hex [b & 15]);
        Serial.print(" ");
    }
    Serial.println();
}

int Domo_Pin::freeRam(){
    extern int __heap_start, *__brkval; 
    int v; 
    return (int) &v - (__brkval == 0 ? (int) &__heap_start : (int) __brkval); 
}

