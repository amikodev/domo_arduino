
#ifndef DS2408_h
#define DS2408_h

class DS2408{
    
public:
    static void digitalRead(int pin, byte addr[], byte& data);
    static void digitalWrite(int pin, byte addr[], byte data);
    
};

#endif


