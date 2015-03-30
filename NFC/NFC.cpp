
#include "NFC.h"

u8 hextab[17] = "0123456789ABCDEF";
u8 ack[6] = {0x00, 0x00, 0xFF, 0x00, 0xFF, 0x00};
u8 nfc_version[6] = {0x00, 0xFF, 0x06, 0xFA, 0xD5, 0x03};

u8 nfc_buf[NFC_CMD_BUF_LEN];

NFC_Module::NFC_Module(void){
    
}

void NFC_Module::begin(void){
    Wire.begin();
}

u32 NFC_Module::get_version(void){
    u32 version;
    
    nfc_buf[0] = PN532_COMMAND_GETFIRMWAREVERSION;
    if(!write_cmd_check_ack(nfc_buf, 1)){
        return 0;
    }
    wait_ready();
    read_dt(nfc_buf, 12);
    if(nfc_buf[5] != 0xD5){
        return 0;
    }
    // check some basic stuff
	if (0 != strncmp((char *)nfc_buf, (char *)nfc_version, 6)) {
		return 0;
	}
    
	version = nfc_buf[7];
	version <<= 8;
	version |= nfc_buf[8];
	version <<= 8;
	version |= nfc_buf[9];
	version <<= 8;
	version |= nfc_buf[10];
    
	return version;
}

u8 NFC_Module::SAMConfiguration(u8 mode, u8 timeout, u8 irq){
	nfc_buf[0] = PN532_COMMAND_SAMCONFIGURATION;
	nfc_buf[1] = mode; // normal mode;
	nfc_buf[2] = timeout; // timeout 50ms * 20 = 1 second
	nfc_buf[3] = irq; // use IRQ pin!
    
    if(!write_cmd_check_ack(nfc_buf, 4)){
        return 0;
    }
    
	// read data packet
	read_dt(nfc_buf, 8);
    
	return  (nfc_buf[6] == PN532_COMMAND_SAMCONFIGURATION);
}

u8 NFC_Module::InListPassiveTarget(u8 *uid, u8 *uidLength, u8 brty, u8 maxtg, u8 *idata){
    nfc_buf[0] = PN532_COMMAND_INLISTPASSIVETARGET;
    nfc_buf[1] = maxtg;
    nfc_buf[2] = brty;
    
    if(!write_cmd_check_ack(nfc_buf, 3)){
        return 0;
    }
    
    /** "Waiting for IRQ (indicates card presence)" */
    wait_ready();
    read_dt(nfc_buf, 20);
    if(nfc_buf[NFC_FRAME_ID_INDEX] != (PN532_COMMAND_INLISTPASSIVETARGET+1)){
        return 0;
    }
    if(nfc_buf[NFC_FRAME_ID_INDEX+1]!=1){
        Serial.println(nfc_buf[NFC_FRAME_ID_INDEX+1],DEC);
        return 0;
    }
    
    *uidLength = nfc_buf[12];
    for(u8 i=0; i<nfc_buf[12]; i++){
        uid[i] = nfc_buf[13+i];
    }
    return 1;
}

u8 NFC_Module::MifareAuthentication(u8 type, u8 block, u8 *uuid, u8 uuid_len, u8 *key){
    u8 i;
    nfc_buf[0] = PN532_COMMAND_INDATAEXCHANGE;
    nfc_buf[1] = 1; // logical number of the relevant target
    nfc_buf[2] = MIFARE_CMD_AUTH_A+type;
    nfc_buf[3] = block;
    
    for(i=0; i<6; i++){
        nfc_buf[4+i] = key[i];
    }
    for(i=0; i<uuid_len; i++){
        nfc_buf[10+i] = uuid[i];
    }
    
    if(!write_cmd_check_ack( nfc_buf, (10+uuid_len) )){
        return 0;
    }
    
    wait_ready();
    read_dt(nfc_buf, 8);
    if(nfc_buf[NFC_FRAME_ID_INDEX] != (PN532_COMMAND_INDATAEXCHANGE+1)){
        return 0;
    }
    if(nfc_buf[NFC_FRAME_ID_INDEX+1]){
        return 0;
    }
    return 1;
}


u8 NFC_Module::MifareReadBlock(u8 block, u8 *buf){
    nfc_buf[0] = PN532_COMMAND_INDATAEXCHANGE;
    nfc_buf[1] = 1; // logical number of the relevant target
    nfc_buf[2] = MIFARE_CMD_READ;
    nfc_buf[3] = block;
    
    if(!write_cmd_check_ack(nfc_buf, 4)){
        return 0;
    }
    wait_ready();
    read_dt(nfc_buf, 26);
    if(nfc_buf[NFC_FRAME_ID_INDEX] != (PN532_COMMAND_INDATAEXCHANGE+1)){
        return 0;
    }
    if(nfc_buf[NFC_FRAME_ID_INDEX+1]){
        return 0;
    }
    
    memcpy(buf, nfc_buf+8, 16);
    
    return 1;
}



u8 NFC_Module::MifareReadPage(u8 page, u8 *buffer){
    if (page >= 64){
        return 0;
    }
    
    nfc_buf[0] = PN532_COMMAND_INDATAEXCHANGE;
    nfc_buf[1] = 1; // logical number of the relevant target
    nfc_buf[2] = MIFARE_CMD_READ;
    nfc_buf[3] = page;
    
    if(!write_cmd_check_ack(nfc_buf, 4)){
        return 0;
    }
    wait_ready();
    read_dt(nfc_buf, 26);
    
    if (nfc_buf[7] == 0x00){
        memcpy(buffer, nfc_buf+8, 4);
    } else{
        return 0;
    }
    
    return 1;
}

u8 NFC_Module::MifareWriteBlock(u8 block, u8 *buf)
{
    nfc_buf[0] = PN532_COMMAND_INDATAEXCHANGE;
    nfc_buf[1] = 1; // logical number of the relevant target
    nfc_buf[2] = MIFARE_CMD_WRITE;
    nfc_buf[3] = block;
    
    memcpy(nfc_buf+4, buf, 16);
    
    if(!write_cmd_check_ack(nfc_buf, 20)){
        return 0;
    }
    wait_ready();
    read_dt(nfc_buf, 26);
    if(nfc_buf[NFC_FRAME_ID_INDEX] != (PN532_COMMAND_INDATAEXCHANGE+1)){
        return 0;
    }
    if(nfc_buf[NFC_FRAME_ID_INDEX+1]){
        return 0;
    }
    return 1;
}


u8 NFC_Module::MifareWritePage(u8 page, u8 *buffer){
    if (page >= 64){
        return 0;
    }
    
    nfc_buf[0] = PN532_COMMAND_INDATAEXCHANGE;
    nfc_buf[1] = 1; // logical number of the relevant target
    nfc_buf[2] = MIFARE_CMD_ULTRALIGHT_WRITE;
    nfc_buf[3] = page;
    
    memcpy(nfc_buf+4, buffer, 4);
    
    if(!write_cmd_check_ack(nfc_buf, 8)){
        return 0;
    }
    wait_ready();
    read_dt(nfc_buf, 26);
    if(nfc_buf[NFC_FRAME_ID_INDEX] != (PN532_COMMAND_INDATAEXCHANGE+1)){
        return 0;
    }
    return 1;
    
}


u8 NFC_Module::SetParameters(u8 para){
    nfc_buf[0] = PN532_COMMAND_SETPARAMETERS;
    nfc_buf[1] = para;
    
    if(!write_cmd_check_ack(nfc_buf, 2)){
        return 0;
    }
    read_dt(nfc_buf, 8);
    if(nfc_buf[NFC_FRAME_ID_INDEX] != (PN532_COMMAND_SETPARAMETERS+1)){
        return 0;
    }
    return 1;
}

u8 NFC_Module::write_cmd_check_ack(u8 *cmd, u8 len){
    write_cmd(cmd, len);
    wait_ready();
    
	// read acknowledgement
	if (!read_ack()) {
		return false;
	}
    
	return true; // ack'd command
}

inline u8 NFC_Module::send(u8 data){
#if ARDUINO >= 100
    return Wire.write((u8)data);
#else
    return Wire.send((u8)data);
#endif
}

inline u8 NFC_Module::receive(void){
#if ARDUINO >= 100
    return Wire.read();
#else
    return Wire.receive();
#endif
}

void NFC_Module::puthex(u8 data){
    Serial.write(hextab[(data>>4)&0x0F]);
    Serial.write(hextab[data&0x0F]);
    Serial.write(' ');
}

void NFC_Module::puthex(u8 *buf, u32 len){
    u32 i;
    for(i=0; i<len; i++){
        Serial.write(hextab[(buf[i]>>4)&0x0F]);
        Serial.write(hextab[buf[i]&0x0F]);
        Serial.write(' ');
    }
}

void NFC_Module::write_cmd(u8 *cmd, u8 len){
    uint8_t checksum;
    len++;
    
    delay(2);     // or whatever the delay is for waking up the board
    
    // I2C START
    Wire.beginTransmission(PN532_I2C_ADDRESS);
    checksum = PN532_PREAMBLE + PN532_PREAMBLE + PN532_STARTCODE2;
    send(PN532_PREAMBLE);
    send(PN532_PREAMBLE);
    send(PN532_STARTCODE2);
    
    send(len);
    send(~len + 1);
    
    send(PN532_HOSTTOPN532);
    checksum += PN532_HOSTTOPN532;
    
    for (uint8_t i=0; i<len-1; i++){
        if(send(cmd[i])){
            checksum += cmd[i];
        } else{
            i--;
            delay(1);
        }
    }
    
    send(~checksum);
    send(PN532_POSTAMBLE);
    
    // I2C STOP
    Wire.endTransmission();
    
}

void NFC_Module::read_dt(u8 *buf, u8 len){
    delay(2);
    
    Wire.requestFrom((u8)PN532_I2C_ADDRESS, (u8)(len+2));
    // Discard the leading 0x01
    receive();
    for (u8 i=0; i<len; i++){
        delay(1);
        buf[i] = receive();
        if((len!=6)&&i==3&&(buf[0]==0)&&(buf[1]==0)&&(buf[2]==0xFF)){
            len = buf[i]+6;
        }
    }
}

u8 NFC_Module::read_ack(void){
    u8 ack_buf[6];
    read_dt(ack_buf, 6);
    return (0 == strncmp((char *)ack_buf, (char *)ack, 6));
}

u8 NFC_Module::read_sta(void){
    delay(NFC_WAIT_TIME);
    return PN532_I2C_READY;
}

u8 NFC_Module::wait_ready(u8 ms){
    delay(ms);
    return PN532_I2C_READY;
}
