#include <Arduino.h>

#include <functional>


static const uint32_t ATTINY84_ID = 0x1e930c;
static const uint8_t CODE[] = {
    0x2c,0xc0,0x46,0xc0,0x45,0xc0,0x44,0xc0,0x8f,0xc1,0x42,0xc0,0x41,0xc0,0x40,0xc0,0x3f,0xc0,0x3e,0xc0,0x3d,0xc0,0x3e,0xc1,0x3b,0xc0,0x3a,0xc0,0x39,0xc0,0x38,0xc0,0x37,0xc0,0x00,0x00,0x39,0x00,0x36,0x00,0x00,0x00,0x3a,0x00,0x37,0x00,0x00,0x00,0x3b,0x00,0x38,0x00,0x02,0x02,0x02,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x02,
    0x01,0x02,0x04,0x80,0x40,0x20,0x10,0x08,0x04,0x02,0x01,0x08,0x00,0x00,0x01,0x02,0x03,0x04,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x11,0x24,0x1f,0xbe,0xcf,0xe5,0xd2,0xe0,0xde,0xbf,0xcd,0xbf,0x10,0xe0,0xa0,0xe6,0xb0,0xe0,0xee,0xe6,0xf4,0xe0,0x02,0xc0,0x05,0x90,0x0d,0x92,0xa2,0x36,0xb1,0x07,0xd9,0xf7,0x20,0xe0,0xa2,0xe6,
    0xb0,0xe0,0x01,0xc0,0x1d,0x92,0xac,0x36,0xb2,0x07,0xe1,0xf7,0x5c,0xd1,0xed,0xc1,0xb7,0xcf,0x81,0x30,0x31,0xf4,0x80,0xb7,0x8f,0x77,0x80,0xbf,0x80,0xb7,0x8f,0x7b,0x07,0xc0,0x82,0x30,0x39,0xf4,0x80,0xb7,0x8f,0x7d,0x80,0xbf,0x80,0xb7,0x8f,0x7e,0x80,0xbf,0x08,0x95,0x83,0x30,0x31,0xf4,0x8f,0xb5,0x8f,0x77,0x8f,0xbd,0x8f,0xb5,
    0x8f,0x7b,0x07,0xc0,0x84,0x30,0x31,0xf4,0x8f,0xb5,0x8f,0x7d,0x8f,0xbd,0x8f,0xb5,0x8f,0x7e,0x8f,0xbd,0x08,0x95,0x1f,0x93,0xcf,0x93,0xdf,0x93,0x16,0x2f,0x87,0xff,0x08,0xc0,0x28,0x2f,0x2f,0x77,0x28,0x30,0x18,0xf4,0x8a,0xe0,0x82,0x1b,0x01,0xc0,0x8f,0xef,0x28,0x2f,0x30,0xe0,0xf9,0x01,0xe4,0x5b,0xff,0x4f,0x84,0x91,0xf9,0x01,
    0xe0,0x5c,0xff,0x4f,0xd4,0x91,0xf9,0x01,0xec,0x5c,0xff,0x4f,0xc4,0x91,0xcc,0x23,0xd1,0xf0,0x81,0x11,0xbe,0xdf,0xec,0x2f,0xf0,0xe0,0xee,0x0f,0xff,0x1f,0xe2,0x5d,0xff,0x4f,0xa5,0x91,0xb4,0x91,0x11,0x11,0x08,0xc0,0x9f,0xb7,0xf8,0x94,0x8c,0x91,0xd0,0x95,0xd8,0x23,0xdc,0x93,0x9f,0xbf,0x06,0xc0,0x8f,0xb7,0xf8,0x94,0xec,0x91,
    0xde,0x2b,0xdc,0x93,0x8f,0xbf,0xdf,0x91,0xcf,0x91,0x1f,0x91,0x08,0x95,0xcf,0x93,0xdf,0x93,0x87,0xff,0x08,0xc0,0xe8,0x2f,0xef,0x77,0xe8,0x30,0x18,0xf4,0x8a,0xe0,0x8e,0x1b,0x01,0xc0,0x8f,0xef,0x90,0xe0,0xfc,0x01,0xe0,0x5c,0xff,0x4f,0x24,0x91,0xfc,0x01,0xec,0x5c,0xff,0x4f,0x84,0x91,0x88,0x23,0x61,0xf1,0x90,0xe0,0x88,0x0f,
    0x99,0x1f,0xfc,0x01,0xe8,0x5d,0xff,0x4f,0xc5,0x91,0xd4,0x91,0xfc,0x01,0xe2,0x5d,0xff,0x4f,0xa5,0x91,0xb4,0x91,0x61,0x11,0x09,0xc0,0x9f,0xb7,0xf8,0x94,0x88,0x81,0x20,0x95,0x82,0x23,0x88,0x83,0xec,0x91,0x2e,0x23,0x0b,0xc0,0x62,0x30,0x61,0xf4,0x9f,0xb7,0xf8,0x94,0x88,0x81,0x32,0x2f,0x30,0x95,0x83,0x23,0x88,0x83,0xec,0x91,
    0x2e,0x2b,0x2c,0x93,0x9f,0xbf,0x06,0xc0,0x8f,0xb7,0xf8,0x94,0xe8,0x81,0x2e,0x2b,0x28,0x83,0x8f,0xbf,0xdf,0x91,0xcf,0x91,0x08,0x95,0xcf,0x93,0xc8,0x2f,0x61,0xe0,0xb6,0xdf,0x60,0xe0,0x8c,0x2f,0xcf,0x91,0x76,0xcf,0x0f,0x93,0x1f,0x93,0xcf,0x93,0xdf,0x93,0x16,0x2f,0xd4,0x2f,0x87,0xff,0x08,0xc0,0x28,0x2f,0x2f,0x77,0x28,0x30,
    0x18,0xf4,0x8a,0xe0,0x82,0x1b,0x01,0xc0,0x8f,0xef,0x28,0x2f,0x30,0xe0,0xf9,0x01,0xe4,0x5b,0xff,0x4f,0x84,0x91,0xf9,0x01,0xe0,0x5c,0xff,0x4f,0x04,0x91,0xf9,0x01,0xec,0x5c,0xff,0x4f,0xc4,0x91,0xcc,0x23,0x99,0xf0,0x81,0x11,0x32,0xdf,0xec,0x2f,0xf0,0xe0,0xee,0x0f,0xff,0x1f,0xee,0x5d,0xff,0x4f,0xa5,0x91,0xb4,0x91,0xec,0x91,
    0xe0,0x23,0x91,0xe0,0x80,0xe0,0x09,0xf4,0x90,0xe0,0x29,0x2f,0x38,0x2f,0x02,0xc0,0x20,0xe0,0x30,0xe0,0x80,0x91,0x62,0x00,0x8d,0x23,0x21,0x30,0x31,0x05,0x71,0xf4,0x81,0x11,0x13,0xc0,0x80,0x91,0x62,0x00,0xd8,0x2b,0xd0,0x93,0x62,0x00,0x61,0xe0,0x81,0x2f,0xdf,0x91,0xcf,0x91,0x1f,0x91,0x0f,0x91,0x2d,0xcf,0x88,0x23,0x29,0xf0,
    0x80,0x91,0x62,0x00,0xd8,0x27,0xd0,0x93,0x62,0x00,0xdf,0x91,0xcf,0x91,0x1f,0x91,0x0f,0x91,0x08,0x95,0x1f,0x92,0x0f,0x92,0x0f,0xb6,0x0f,0x92,0x11,0x24,0x2f,0x93,0x3f,0x93,0x8f,0x93,0x9f,0x93,0xaf,0x93,0xbf,0x93,0x80,0x91,0x68,0x00,0x90,0x91,0x69,0x00,0xa0,0x91,0x6a,0x00,0xb0,0x91,0x6b,0x00,0x30,0x91,0x67,0x00,0x26,0xe0,
    0x23,0x0f,0x2d,0x37,0x30,0xf0,0x29,0xe8,0x23,0x0f,0x03,0x96,0xa1,0x1d,0xb1,0x1d,0x03,0xc0,0x02,0x96,0xa1,0x1d,0xb1,0x1d,0x20,0x93,0x67,0x00,0x80,0x93,0x68,0x00,0x90,0x93,0x69,0x00,0xa0,0x93,0x6a,0x00,0xb0,0x93,0x6b,0x00,0x80,0x91,0x63,0x00,0x90,0x91,0x64,0x00,0xa0,0x91,0x65,0x00,0xb0,0x91,0x66,0x00,0x01,0x96,0xa1,0x1d,
    0xb1,0x1d,0x80,0x93,0x63,0x00,0x90,0x93,0x64,0x00,0xa0,0x93,0x65,0x00,0xb0,0x93,0x66,0x00,0xbf,0x91,0xaf,0x91,0x9f,0x91,0x8f,0x91,0x3f,0x91,0x2f,0x91,0x0f,0x90,0x0f,0xbe,0x0f,0x90,0x1f,0x90,0x18,0x95,0x1f,0x92,0x0f,0x92,0x0f,0xb6,0x0f,0x92,0x11,0x24,0x8f,0x93,0x81,0xe0,0x80,0x93,0x60,0x00,0x8f,0x91,0x0f,0x90,0x0f,0xbe,
    0x0f,0x90,0x1f,0x90,0x18,0x95,0x81,0xb7,0x81,0xbf,0x80,0xb7,0x82,0x60,0x80,0xbf,0x80,0xb7,0x81,0x60,0x80,0xbf,0x83,0xb7,0x88,0x7f,0x83,0x60,0x83,0xbf,0x78,0x94,0x89,0xb7,0x81,0x60,0x89,0xbf,0x8e,0xb5,0x88,0x7f,0x8e,0xbd,0x8f,0xb5,0x81,0x60,0x8f,0xbd,0x8f,0xb5,0x8d,0x7f,0x8f,0xbd,0x8e,0xb5,0x87,0x7f,0x8e,0xbd,0x8e,0xb5,
    0x8f,0x7e,0x8e,0xbd,0x8e,0xb5,0x83,0x60,0x8e,0xbd,0x86,0xb1,0x88,0x77,0x86,0x68,0x86,0xb9,0x37,0x9a,0x83,0xea,0x80,0xbf,0x81,0xe0,0x83,0xbf,0x88,0xe0,0x90,0xe0,0x1c,0xdf,0x87,0xe0,0x90,0xe0,0x19,0xdf,0x86,0xe0,0x90,0xe0,0x16,0xdf,0x85,0xe0,0x90,0xe0,0x13,0xdf,0x60,0xe0,0x83,0xe0,0xca,0xde,0x60,0xe0,0x82,0xe0,0xc7,0xde,
    0x60,0xe0,0x81,0xe0,0xc4,0xde,0x60,0xe0,0x80,0xe0,0xc1,0xde,0x84,0xb7,0x87,0x7f,0x84,0xbf,0x81,0xb5,0x88,0x61,0x81,0xbd,0x85,0xe1,0x81,0xbd,0x81,0xb5,0x80,0x64,0x81,0xbd,0x80,0x91,0x60,0x00,0x81,0x30,0xe1,0xf7,0x10,0x92,0x60,0x00,0x41,0xe0,0x68,0xe0,0x70,0xe0,0x83,0xe0,0x90,0xe0,0xf8,0xde,0x42,0xe0,0x67,0xe0,0x70,0xe0,
    0x82,0xe0,0x90,0xe0,0xf2,0xde,0x44,0xe0,0x66,0xe0,0x70,0xe0,0x81,0xe0,0x90,0xe0,0xec,0xde,0x48,0xe0,0x65,0xe0,0x70,0xe0,0x80,0xe0,0x90,0xe0,0xe6,0xde,0x8f,0xe1,0x9e,0xe4,0x01,0x97,0xf1,0xf7,0x00,0xc0,0x00,0x00,0x60,0xe0,0x88,0xe0,0x53,0xde,0x60,0xe0,0x87,0xe0,0x50,0xde,0x60,0xe0,0x86,0xe0,0x4d,0xde,0x60,0xe0,0x85,0xe0,
    0x4a,0xde,0x37,0x98,0x85,0xb7,0x87,0x7e,0x80,0x61,0x85,0xbf,0x85,0xb7,0x80,0x62,0x85,0xbf,0x85,0xb7,0x80,0x62,0x85,0xbf,0x88,0x95,0x85,0xb7,0x8f,0x7d,0x85,0xbf,0x85,0xb7,0x8f,0x7d,0x85,0xbf,0x37,0x9a,0xbc,0xcf,0xf8,0x94,0xff,0xcf,0x01,0x00
    };


struct TransResult {
    uint8_t a;
    uint8_t b;
    uint8_t c;
    uint8_t d;
};


class Programmer {
    uint32_t _need_chip_id;
    bool _rst_active_high;
    std::function<void(bool)> _prog_light_switch;
    std::function<void(bool)> _error_light_switch;

    uint32_t get3Bytes(uint8_t a, uint8_t b, uint8_t d0, uint8_t d1, uint8_t d2);
    TransResult transaction(uint8_t a, uint8_t b, uint8_t c, uint8_t d);
    void sendReset(bool reset);
    bool setPmode();
    void endPmode();
    uint32_t getChipId();
    uint32_t getLowFuses();
    uint32_t getHighFuses();
    uint32_t getExtFuses();
    void sendClear();
    void flash(uint8_t hilo, unsigned int addr, uint8_t data);
    void commit(unsigned int addr);
    uint8_t getByte(const uint8_t * buf, int i, int len);
    void sendImage(const uint8_t *buf, int len);
    uint8_t read(uint8_t hilo, unsigned int addr);
    bool werify(const uint8_t *buf, int len);

public:
    Programmer(uint32_t need_chip_id = ATTINY84_ID);
    ~Programmer();

    void setFlashFunc(const std::function<void(bool)> &prog_light_switch);
    void setErrorFunc(const std::function<void(bool)> &error_light_switch);

    void load(const uint8_t *buf, int len);
};