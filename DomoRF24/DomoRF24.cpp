#include "DomoRF24.h"

void DomoRF24::init(RF24 *radio, CryptoDH *dh){
    
    _radio = radio;
    _dh = dh;
    
    _radio->begin();
    _radio->setChannel(0x23);
    _radio->enableAckPayload();
    _radio->setRetries(15, 15);
    _radio->setDataRate(RF24_250KBPS);        // Установка минимальной скорости;
    _radio->setPALevel(RF24_PA_MAX);          // Установка максимальной мощности;
    _radio->setAutoAck(1);                    // Установка режима подтверждения приема;
    _radio->setCRCLength(RF24_CRC_16);

    dataType = 0;
    
}

bool DomoRF24::send(byte* buf, uint8_t len){
    bool ret = true;
    if(len > 0){
        RadioData rd;

        uint8_t dataLen = sizeof(rd.data);
        
        rd.pagesCount = len/dataLen;
        if(len % dataLen > 0)
            rd.pagesCount++;

        for(int i=0; i<rd.pagesCount; i++){
            rd.page = i+1;
            rd.length = rd.page<rd.pagesCount || (len % dataLen == 0) ? dataLen : len % dataLen;
            memcpy((byte *)rd.data, buf+i*dataLen, rd.length);
            for(int j=rd.length; j<dataLen; j++){
                rd.data[j] = 0;
            }
            
            dump((byte *)&rd, sizeof(rd), "rd");
            
            byte tryCount = 5;
            bool success;
            do{
                success = _radio->write(&rd, sizeof(rd));
            } while(!success && tryCount-- > 0);
            if(!success){
                ret = false;
            }
            
            dump((byte *)&success, sizeof(success), "success");
            dump((byte *)&tryCount, sizeof(tryCount), "tryCount");
        }
    }
    return ret;
}

bool DomoRF24::sendCrypt(byte* buf, uint8_t len){
    bool ret = true;
    if(len > 0){
        int nlen = len + (len%N_BLOCK > 0 ? N_BLOCK-(len%N_BLOCK) : 0);

        byte bufc[nlen+1];
        bufc[0] = RF24_CRYPTED_TEXT;
        memcpy(bufc+1, buf, len);
        
        for(int i=len+1; i<nlen+1; i++){
            bufc[i] = 0;
        }
        
        AES aes;
        aes.set_key( (byte*)K128, 128 );

        for(int i=0; i<nlen/N_BLOCK; i++){
            aes.encrypt(bufc+i*N_BLOCK+1, bufc+i*N_BLOCK+1);
        }
        
        ret = send((byte *)bufc, nlen+1);
    }
    return ret;
}


void DomoRF24::process(){
    uint8_t pipe_num;
    if(_radio->available(&pipe_num)){
        bool done = false;
        //int dataType = 0;
        while(!done){
            RadioData rd;
            done = _radio->read(&rd, sizeof(rd));
            dump((byte *)&rd, sizeof(rd), "rd");
            
            /*
             * rd.data[0]
             *      1 - change keys: P
             *      2 - change keys: G
             *      3 - change keys: A
             *      4 - change keys: B
             *      5 - crypted text
            */
            
            
            if(rd.page == 1){
                dataType = rd.data[0];
            }
            
            if(rd.page == 1 && rd.pagesCount == 1){
                if(dataType == RF24_CHK_P){
                    memcpy(&p128, rd.data+1, sizeof(p128));
                } else if(dataType == RF24_CHK_G){
                    memcpy(&g128, rd.data+1, sizeof(g128));
                    _dh->initParams128(p128, g128);
                    _dh->randomSecureKey128();
                } else if(dataType == RF24_CHK_A){
                    _dh->setRemotePublicKey128((unsigned long *)(rd.data+1));

                    // send B to first node
                    _radio->startListening();
                    _radio->stopListening();

                    StateDataLong stB;
                    _dh->getPublicKey128(stB.data);

                    bool success;

                    stB.state = 4;
                    success = send((byte *)&stB, sizeof(stB));

                    _radio->startListening();


                    _dh->calcPrivateKey128();
                    _dh->getPrivateKey128(K128);

                    dump((byte *)&K128, sizeof(K128), "K128");
//                    Serial.print("; K128: ");
//                    for(int i=0; i<4; i++){
//                        Serial.print(K128[i], DEC);
//                        Serial.print(" ");
//                    }
//                    Serial.print("; \r\n");


                } else if(dataType == RF24_CHK_B){
                    _dh->setRemotePublicKey128((unsigned long *)(rd.data+1));
                    _dh->calcPrivateKey128();
                    _dh->getPrivateKey128(K128);

                    dump((byte *)&K128, sizeof(K128), "K128");
//                    Serial.print("; K128: ");
//                    for(int i=0; i<4; i++){
//                        Serial.print(K128[i], DEC);
//                        Serial.print(" ");
//                    }
//                    Serial.print("; \r\n");

                }
            }


            if(rd.page == 1){
                
                if(dataType == RF24_CRYPTED_TEXT){        // crypted text
                    int nlen = rd.pagesCount*rd.length;
                    nlen += nlen%N_BLOCK > 0 ? N_BLOCK-(nlen%N_BLOCK) : 0;
                    fullMessage = (byte *)malloc(nlen);
                    fullMessageLength = rd.length-1;
                    memcpy(fullMessage, rd.data+1, sizeof(rd.data)-1);
                }
            } else{
                memcpy(fullMessage+fullMessageLength, rd.data, rd.length);
                fullMessageLength += rd.length;
            }
            
            if(rd.page == rd.pagesCount && dataType == RF24_CRYPTED_TEXT){
                AES aes;
                aes.set_key( (byte*)K128, 128 );

                int nlen = fullMessageLength + (fullMessageLength%N_BLOCK > 0 ? N_BLOCK-(fullMessageLength%N_BLOCK) : 0);
                for(int i=0; i<nlen/N_BLOCK; i++){
                    aes.decrypt(fullMessage+i*N_BLOCK, fullMessage+i*N_BLOCK);
                }
                
                if(funcResp != NULL){
                    funcResp(fullMessage, fullMessageLength);
                }
            }
        }
    }
    
    
}

/**
 * Обмен ключами
 */
void DomoRF24::changeKeys(){

    StateDataLong stA;
    StateDataInt stP, stG;

    _dh->randomParams128();
    _dh->randomSecureKey128();
    _dh->getPublicKey128(stA.data);
    
    
    _radio->startListening();
    _radio->stopListening();

    bool success;

    _dh->getParams128(stP.data, stG.data);
    
    stP.state = 1;
    stG.state = 2;
    stA.state = 3;

    success = send((byte *)&stP, sizeof(stP));
    success = send((byte *)&stG, sizeof(stG));
    success = send((byte *)&stA, sizeof(stA));

    _radio->startListening();
    
    
    
}


void DomoRF24::dump(byte* buf, uint8_t len, const char* title){

//#ifdef DOMO_RADIO_DEBUG
    Serial.print(title);
    Serial.print(": ");
    for(int i=0; i<len; i++){
        Serial.print(buf[i], DEC);
        Serial.print(" ");
    }
    Serial.println();
//#endif
    
}


void DomoRF24::print_value(char * str, byte * a, int bits){

//#ifdef DOMO_RADIO_DEBUG
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
//#endif
    
}
    
    