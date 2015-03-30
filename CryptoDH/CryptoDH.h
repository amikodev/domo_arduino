#ifndef __CryptoDH_H__
#define __CryptoDH_H__

#if ARDUINO >= 100
#include "Arduino.h"
#else
#include "WProgram.h"
#endif


class CryptoDH{
    
  
public:
    
    void initParams(unsigned int _p, unsigned int _g);
    void initParams128(unsigned int* _p128, unsigned int* _g128);
    void randomParams();
    void randomParams128();
    unsigned int randomSecureKey();
    unsigned int& randomSecureKey128();
    unsigned long getPublicKey();
    void getPublicKey128(unsigned long* A128);
    unsigned long modexp(unsigned long a, unsigned long b, unsigned long n);

    void setRemotePublicKey(unsigned long _B);
    void setRemotePublicKey128(unsigned long* _B128);
    unsigned long getRemotePublicKey();
    unsigned long getPrivateKey();
    unsigned long calcPrivateKey128();
    unsigned long getPrivateKey128(unsigned long* _K128);
    unsigned long setPrivateKey128(unsigned long* _K128);
    
    void getParams(unsigned int& _p, unsigned int& _g);
    void getParams128(unsigned int* _p128, unsigned int* _g128);
        
    
private:
    
    unsigned int p, g;
    unsigned int a;
    unsigned long B;
    unsigned long K;
    
    unsigned int p128[8], g128[8];
    unsigned int a128[8];
    unsigned long B128[4];
    unsigned long K128[4];
    
    
};

#endif
