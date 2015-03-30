
/*
 * CMD_RADIO
 *      cmd[1]|type[1]|pin1[1]|pin2[1]      - radio init
 *      cmd[1]|type[1]|addr[5]              - radio writing pipe
 *      cmd[1]|type[1]|numpipe[1]|addr[5]   - radio reading pipe
 *      cmd[1]|type[1]                      - radio change keys
 *      cmd[1]|type[1]|len[1]|data[n]       - radio send data
 * 
 * 
 * 
 * 
 * output:
 *      cmd[1]|type[1]|pipe[1]|page[1]|data[8]      - radio responce
 *      cmd[1]|type[1]|page[1]|data[8]              - radio K128
 * 
 */


#include <Domo_Pin.h>

#include <SPI.h>
#include "nRF24L01.h"
#include "RF24.h"

#include "AES.h"

#include "Domo_Pin_RF24.h"
#include "CryptoDH.h"



byte Domo_Pin::cmd_Radio(byte *buf, byte len){
    byte val = 0;
    
    if(buf[1] == CMD_TYPE_RADIO_INIT){
        radio = new RF24(buf[2], buf[3]);
        dradio = new Domo_Pin_RF24();
        
        CryptoDH *dh = new CryptoDH();
        ((Domo_Pin_RF24 *)dradio)->init(this, (RF24 *)radio, dh);
        
        ((RF24 *)radio)->startListening();
        
    } else if(buf[1] == CMD_TYPE_RADIO_WPIPE){
        uint64_t addr = 0;
        memcpy(&addr, buf+2, 5);
        ((RF24 *)radio)->openWritingPipe(addr);
    } else if(buf[1] == CMD_TYPE_RADIO_RPIPE){
        uint64_t addr = 0;
        memcpy(&addr, buf+3, 5);
        ((RF24 *)radio)->openReadingPipe(((int)buf+2), addr);
    } else if(buf[1] == CMD_TYPE_RADIO_CHKEYS){
        ((Domo_Pin_RF24 *)dradio)->changeKeys();
    } else if(buf[1] == CMD_TYPE_RADIO_SEND){
        ((RF24 *)radio)->startListening();
        ((RF24 *)radio)->stopListening();
        ((Domo_Pin_RF24 *)dradio)->sendCrypt(buf+3, buf[2]);
        ((RF24 *)radio)->startListening();
    }
    
    return val;
}

void Domo_Pin::radio_recieve(byte pipe_num, byte *message, byte length){

    if(message[0] == CMD_RADIO && message[1] == CMD_TYPE_RADIO_RESPONCE){    // это ответ
        byte bufc[32];
        for(int i=0; i<sizeof(bufc); i++) bufc[i]=0;
        memcpy(bufc, message, length);
        bufc[2] = pipe_num;
        Serial.write(bufc, sizeof(bufc));
    } else{
        
        // выполняем команду и отправляем результат обратно
        byte bufc[19] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
        bool r = process(message, length);
        if(r == true){
            Serial.write(message, length);
            
            ((RF24 *)radio)->startListening();
            ((RF24 *)radio)->stopListening();
            
            bufc[0] = CMD_RADIO;
            bufc[1] = CMD_TYPE_RADIO_RESPONCE;
            bufc[2] = 0;        // резерв для pipe_num
            memcpy(bufc+3, message, length);
            
            ((Domo_Pin_RF24 *)dradio)->sendCrypt(bufc, sizeof(bufc));
            
            ((RF24 *)radio)->startListening();
            
        }
        
        
        
        // TODO: переделать
        /*
        bool r = process(message, length);
        if(r == true){
            Serial.write(message, length);

            ((RF24 *)radio)->startListening();
            ((RF24 *)radio)->stopListening();

            bufc[0] = CMD_RADIO;
            bufc[1] = CMD_TYPE_RADIO_RESPONCE;
            bufc[2] = pipe_num;
            bufc[3] = 0;
            memcpy(bufc+4, message, 8);
            // send data
            ((Domo_Pin_RF24 *)dradio)->sendCrypt(buf, sizeof(buf));
            
            bufc[3] = 1;
            memcpy(bufc+4, message+8, 8);
            // send data
            ((Domo_Pin_RF24 *)dradio)->sendCrypt(buf, sizeof(buf));


            ((RF24 *)radio)->startListening();
            
        }
        */
    }
    
    
    
    
    
//    byte bufc[16] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
//    memcpy(((byte *)bufc)+2, message, length<sizeof(bufc)?length:sizeof(bufc));
//
//    bool r = process(bufc, sizeof(bufc));
//
//    if(r == true){
//        Serial.write(bufc, sizeof(bufc));
//    }
    
    
//    Serial.println("recieve message: ");
//    Serial.println((char *)message);
//    Serial.print("recieve message length: ");
//    Serial.println(length, DEC);
    free(message);
    
}



