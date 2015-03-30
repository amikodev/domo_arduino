
/*
 * 
 * CMD_NFC:
 *      cmd[1]|type[1]                  - nfc init
 *      cmd[1]|type[1]|block[1]|data[4] - nfc write
 * 
 * 
 * output:
 *      cmd[1]|type[1]|data[4]          - nfc firmware
 *      cmd[1]|type[1]|code[1]          - nfc error code
 *      cmd[1]|type[1]|uid[7]|block[1]|data[16]           - nfc recieve
 * 
 * 
 * 
 */


#include <Domo_Pin.h>

#include <NFC.h>

byte Domo_Pin::cmd_Nfc(byte *buf, byte len){
    byte val = 0;
    
    if(buf[1] == CMD_TYPE_NFC_INIT){
        nfc = new NFC_Module();
        uint32_t versiondata;
        byte trycount = 5;
        do{
            versiondata = ((NFC_Module *)nfc)->get_version();
            if(versiondata){
                break;
            }
            delay(100);
        } while(!versiondata && trycount-- > 0);
        
        ((NFC_Module *)nfc)->SAMConfiguration();
        
        if(versiondata){
            buf[1] = CMD_TYPE_NFC_FIRMWARE;
            memcpy(buf+2, (byte *)&versiondata, sizeof(versiondata));
        } else{
            buf[1] = CMD_TYPE_NFC_ERROR;
            buf[2] = CMD_ERROR_NFC_DEVICENOTFOUND;
        }
    } else if(buf[1] == CMD_TYPE_NFC_WRITE){

        byte data[16] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
        uint8_t res;
        int block = buf[2];

        u8 uid[] = {0, 0, 0, 0, 0, 0, 0};
        u8 uidLength;
        u8 sta;
        sta = ((NFC_Module *)nfc)->InListPassiveTarget(&uid[0], &uidLength);
        if(sta){
            if(uidLength == 4){
                memcpy(&data, buf+3, 4);
                uint8_t keya[6] = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };
                if(((NFC_Module *)nfc)->MifareAuthentication(0, block, uid, uidLength, keya)){
                    res = ((NFC_Module *)nfc)->MifareWriteBlock(block, data);
                } else{         // not auth
                    res = -1;
                }
            } else if(uidLength == 7){
                memcpy(&data, buf+3, 4);
                res = ((NFC_Module *)nfc)->MifareWritePage(block, data);
            }
            
            if(res == -1){
                bufc[1] = CMD_TYPE_NFC_ERROR;
                bufc[2] = CMD_ERROR_NFC_NOTAUTH;
            } else if(res == 0){
                bufc[1] = CMD_TYPE_NFC_ERROR;
                bufc[2] = CMD_ERROR_NFC_NOTWRITE;
            } else if(res == 1){
                bufc[1] = CMD_TYPE_NFC_ERROR;
                bufc[2] = CMD_ERROR_NFC_NONE;
            }

        } else{
            bufc[1] = CMD_TYPE_NFC_ERROR;
            bufc[2] = CMD_ERROR_NFC_CARDNOTVALID;
        }
        
        for(int i=3; i<len; i++) bufc[i] = 0x00;
        
        
    }
    return val;
}

void Domo_Pin::process_nfc(){

    u8 uid[] = {0, 0, 0, 0, 0, 0, 0};
    u8 uidLength;
    u8 sta;
    sta = ((NFC_Module *)nfc)->InListPassiveTarget(&uid[0], &uidLength);
    if(sta){
        
        byte bufc[32];
        for(int i=0; i<sizeof(bufc); i++) bufc[i]=0;

        bufc[0] = CMD_NFC;
        bufc[1] = CMD_TYPE_NFC_RECIEVE;
        
        if(uidLength <= 7){
            
            // прочитаем данные с блока #4
            uint8_t data[16], res;
            int block = 4;

            if(uidLength == 4){
                uint8_t keya[6] = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };
                if(((NFC_Module *)nfc)->MifareAuthentication(0, block, uid, uidLength, keya)){
                    res = ((NFC_Module *)nfc)->MifareReadBlock(block, data);
                    if(res == 1){
                        memcpy(bufc+2, (byte *)&uid, 7);
                        bufc[9] = block;
                        memcpy(bufc+10, (byte *)&data, 16);
                    }
                } else{         // not auth
                    bufc[1] = CMD_TYPE_NFC_ERROR;
                    bufc[2] = CMD_ERROR_NFC_NOTAUTH;
                    res = -1;
                }

            } else if(uidLength == 7){
                res = ((NFC_Module *)nfc)->MifareReadPage(block, data);
                if(res == 1){
                    memcpy(bufc+2, (byte *)&uid, 7);
                    bufc[9] = block;
                    memcpy(bufc+10, data, 4);
                }
            }
            if(res == 0){       // wrong data
                bufc[1] = CMD_TYPE_NFC_ERROR;
                bufc[2] = CMD_ERROR_NFC_WRONGDATA;
            }
            
        } else{                 // not valid
            bufc[1] = CMD_TYPE_NFC_ERROR;
            bufc[2] = CMD_ERROR_NFC_CARDNOTVALID;
        }
        
        if(bufc[1] == CMD_TYPE_NFC_RECIEVE){
            Serial.write(bufc, 32);
        } else{
            Serial.write(bufc, 16);
        }
        
    }

}


