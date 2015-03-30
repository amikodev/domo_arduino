
#ifndef DS2413_h
#define DS2413_h

class DS2413{
    
public:
    static void digitalRead(int pin, byte addr[], byte& a, byte& b);
    static void digitalWrite(int pin, byte addr[], boolean a, boolean b);
    
};

#endif


