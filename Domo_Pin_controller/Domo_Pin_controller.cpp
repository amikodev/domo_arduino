
/*
 * 
 * CMD_CONTROLLER: 
 *      cmd[1]                  - read controller ID
 * 
 * 
 * output:
 *      cmd[1]|id[2]
 * 
 */


#include <Domo_Pin.h>

int Domo_Pin::cmd_Controller(byte *buf, byte len){
    
    int id = getControllerID();
    
    buf[1] = id&0xFF;
    buf[2] = (id>>8)&0xFF;
    
    return id;
}

