#pragma once

#include <Arduino.h>


class Xtea {
    void xtea_encipher(unsigned int num_rounds, uint32_t v[2], uint32_t const key[4]);

    void xtea_decipher(unsigned int num_rounds, uint32_t v[2], uint32_t const key[4]);
   
public:
    Xtea();
    ~Xtea();

    void StringCrypt(char *inout, int len, bool encrypt);
};
