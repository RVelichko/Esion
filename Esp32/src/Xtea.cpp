#include "Xtea.hpp"

static const uint32_t KEY[4] = { 0xFDA5,0xD54E,0xFC00,0xB55A }; ///<  encryption key
static const uint8_t BLOCK_SIZE = 8;


void Xtea::xtea_encipher(unsigned int num_rounds, uint32_t v[2], uint32_t const key[4]) {
    unsigned int i;
    uint32_t v0 = v[0];
    uint32_t v1 = v[1];
    uint32_t sum = 0;
    uint32_t delta = 0x9E3779B9;
    for (i=0; i < num_rounds; i++) {
        v0 += (((v1 << 4) ^ (v1 >> 5)) + v1) ^ (sum + key[sum & 3]);
        sum += delta;
        v1 += (((v0 << 4) ^ (v0 >> 5)) + v0) ^ (sum + key[(sum>>11) & 3]);
    }
    v[0] = v0; 
    v[1] = v1;
}


void Xtea::xtea_decipher(unsigned int num_rounds, uint32_t v[2], uint32_t const key[4]) {
    unsigned int i;
    uint32_t v0 = v[0];
    uint32_t v1 = v[1];
    uint32_t delta = 0x9E3779B9;
    uint32_t sum = delta * num_rounds;
    for (i = 0; i < num_rounds; i++) {
        v1 -= (((v0 << 4) ^ (v0 >> 5)) + v0) ^ (sum + key[(sum >> 11) & 3]);
        sum -= delta;
        v0 -= (((v1 << 4) ^ (v1 >> 5)) + v1) ^ (sum + key[sum & 3]);
    }
    v[0] = v0; 
    v[1] = v1;
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


Xtea::Xtea() 
{}


Xtea::~Xtea() 
{}


void Xtea::StringCrypt(char *inout, int len, bool encrypt) {
    for (int i = 0; i < len / BLOCK_SIZE; i++) {
        if (encrypt) {
            xtea_encipher(32, (uint32_t*)(inout + (i * BLOCK_SIZE)), KEY);
        } else {
            xtea_decipher(32, (uint32_t*)(inout + (i * BLOCK_SIZE)), KEY);
        }
    }
    if (len % BLOCK_SIZE not_eq 0) {
        int mod = len % BLOCK_SIZE;
        int offset = (len / BLOCK_SIZE) * BLOCK_SIZE;
        char data[BLOCK_SIZE];
        memcpy(data, inout + offset, mod);
        if (encrypt) {
            xtea_encipher(32, (uint32_t*)data, KEY);
        } else {
            xtea_decipher(32, (uint32_t*)data, KEY);
        }
        memcpy(inout + offset, data, mod);
    }
}
