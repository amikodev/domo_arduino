#include "CryptoDH.h"

/**
 * Протокол Диффи — Хеллмана
 * https://ru.wikipedia.org/wiki/%D0%9F%D1%80%D0%BE%D1%82%D0%BE%D0%BA%D0%BE%D0%BB_%D0%94%D0%B8%D1%84%D1%84%D0%B8_%E2%80%94_%D0%A5%D0%B5%D0%BB%D0%BB%D0%BC%D0%B0%D0%BD%D0%B0
 */

void CryptoDH::initParams(unsigned int _p, unsigned int _g){
    p = _p;
    g = _g;
}
void CryptoDH::initParams128(unsigned int* _p128, unsigned int* _g128){
    memcpy(p128, _p128, 16);
    memcpy(g128, _g128, 16);
}
    
void CryptoDH::randomParams(){
    randomSeed(micros());
    p = random(1000, 65535);
    g = random(1000, 65535);
}

void CryptoDH::randomParams128(){
    randomSeed(micros());
    for(int i=0; i<8; i++){
        p128[i] = random(1000, 65535);
        g128[i] = random(1000, 65535);
    }
}

unsigned int CryptoDH::randomSecureKey(){
    randomSeed(micros());
    a = random(10000, 65535);
    return a;
}

unsigned int& CryptoDH::randomSecureKey128(){
    randomSeed(micros());
    for(int i=0; i<8; i++){
        a128[i] = random(10000, 65535);
    }
    return *a128;
}

unsigned long CryptoDH::getPublicKey(){
    unsigned long A = modexp(g, a, p);
    return A;
}
void CryptoDH::getPublicKey128(unsigned long* A128){
    unsigned long _A128[8];
    for(int i=0; i<8; i++){
        _A128[i] = modexp(g128[i], a128[i], p128[i]);
        if(i%2 == 1){
            *A128++ = _A128[i]<<16 | _A128[i-1];
        }
    }
}

void CryptoDH::setRemotePublicKey(unsigned long _B){
    B = _B;
}
void CryptoDH::setRemotePublicKey128(unsigned long* _B128){
    memcpy(B128, _B128, 16);
}

unsigned long CryptoDH::getRemotePublicKey(){
    return B;
}

unsigned long CryptoDH::getPrivateKey(){
    K = modexp(B, a, p);
    return K;
}
unsigned long CryptoDH::getPrivateKey128(unsigned long* _K128){
    memcpy(_K128, K128, 16);
}

unsigned long CryptoDH::setPrivateKey128(unsigned long* _K128){
    memcpy(K128, _K128, 16);
}

unsigned long CryptoDH::calcPrivateKey128(){
    unsigned long k128[2];
    for(int i=0; i<8; i++){
        k128[i%2] = modexp( (B128[i/2]>>(i%2==1?16:0)) & 0xFFFF, a128[i], p128[i] );
        if(i%2 == 1){
            K128[i/2] = k128[1]<<16 | k128[0];
        }
    }
}

void CryptoDH::getParams(unsigned int& _p, unsigned int& _g){
    _p = p;
    _g = g;
}
void CryptoDH::getParams128(unsigned int* _p128, unsigned int* _g128){
    memcpy(_p128, p128, 16);
    memcpy(_g128, g128, 16);
}

    

unsigned long CryptoDH::modexp(unsigned long a, unsigned long b, unsigned long n){
    unsigned long y = 1;
    while(b != 0){
	if(b&1) y = (y*a)%n;
	a = (a*a)%n;
	b = b >> 1;
    }
    return y;
}
