#include "Programmer.hpp"


#define PIN_MOSI 12
#define PIN_MISO 13
#define PIN_SCK	 14

#define RESET     15

#define LED_HB    2
#define LED_ERR   4
#define LED_PMODE 2

#define SPI_CLOCK (1000000/6)
#define PTIME 50
#define PAGE_LEN 32


#ifdef DEBUG
String GetHexStr(uint8_t byte) {
    return (byte < 0x10 ? "0x0" : "0x") + String(byte, HEX);
}
#endif


class MySPISettings {
    friend class BitBangedSPI;

    uint32_t clock;

public:
    MySPISettings(uint32_t clock, uint8_t bitOrder, uint8_t dataMode) : clock(clock) {
        (void)bitOrder;
        (void)dataMode;
    };
};


class BitBangedSPI {
    unsigned long pulseWidth; // in microseconds

public:
    void begin() {
        digitalWrite(PIN_SCK, LOW);
        digitalWrite(PIN_MOSI, LOW);
        pinMode(PIN_SCK, OUTPUT);
        pinMode(PIN_MOSI, OUTPUT);
        pinMode(PIN_MISO, INPUT);
    }

    void beginTransaction(MySPISettings settings) {
        pulseWidth = (500000 + settings.clock - 1) / settings.clock;
        if (pulseWidth == 0) {
            pulseWidth = 1;
        }
    }

    void end() 
    {}

    uint8_t transfer(uint8_t b) {
        for (unsigned int i = 0; i < 8; ++i) {
            digitalWrite(PIN_MOSI, (b & 0x80) ? HIGH : LOW);
            digitalWrite(PIN_SCK, HIGH);
            delayMicroseconds(pulseWidth);
            b = (b << 1) | digitalRead(PIN_MISO);
            digitalWrite(PIN_SCK, LOW); // slow pulse
            delayMicroseconds(pulseWidth);
        }
        return b;
    }
};


static BitBangedSPI BbSpi;
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


TransResult Programmer::transaction(uint8_t a, uint8_t b, uint8_t c, uint8_t d) {
    TransResult tr;
    #ifdef DEBUG
    Serial.println("> " + String(a, HEX) + " " + String(b, HEX) + " " + String(c, HEX) + " " + String(d, HEX));
    #endif
    tr.a = BbSpi.transfer(a);
    tr.b = BbSpi.transfer(b);
    tr.c = BbSpi.transfer(c);
    tr.d = BbSpi.transfer(d);
    return tr;
}


void Programmer::sendReset(bool reset) {
    digitalWrite(RESET, ((reset and _rst_active_high) || (not reset and not _rst_active_high)) ? HIGH : LOW);
}


bool Programmer::setPmode() {
    #ifdef DEBUG
    Serial.println("Chip RESET.");
    #endif
    pinMode(RESET, OUTPUT);
    sendReset(true);
    BbSpi.begin();
    BbSpi.beginTransaction(MySPISettings(SPI_CLOCK, MSBFIRST, SPI_MODE0));
    digitalWrite(PIN_SCK, LOW);
    delay(20); 
    sendReset(false);
    delayMicroseconds(100);
    sendReset(true);
    delay(PTIME); ///< Должно быть > 20 msec
    uint8_t res = transaction(0xAC, 0x53, 0x00, 0x00).c;
    #ifdef DEBUG
    Serial.println("Chip to PMODE. Res is " + String(res, HEX));
    #endif
    return res == 0x53; ///< Должно вернуть 2й байт команды, при успешном выполнении.
}


void Programmer::endPmode() {
    BbSpi.end();
    pinMode(PIN_MOSI, INPUT);
    pinMode(PIN_SCK, INPUT);
    sendReset(false);
    pinMode(RESET, INPUT);
}


uint32_t Programmer::get3Bytes(uint8_t a, uint8_t b, uint8_t d0, uint8_t d1, uint8_t d2) {
    uint32_t bytes = 0;
    uint8_t* pb = reinterpret_cast<uint8_t*>(&bytes);
    pb[2] = transaction(a, b, d0, 0x00).d;
    pb[1] = transaction(a, b, d1, 0x00).d;
    pb[0] = transaction(a, b, d2, 0x00).d;
    return bytes;
}


uint32_t Programmer::getChipId() {
    return get3Bytes(0x30, 0x00, 0x00, 0x01, 0x02);
}

uint32_t Programmer::getLowFuses() {
    return get3Bytes(0x50, 0x00, 0x00, 0x00, 0x00);
}


uint32_t Programmer::getHighFuses() {
    return get3Bytes(0x58, 0x08, 0x00, 0x00, 0x00);
}


uint32_t Programmer::getExtFuses() {
    return get3Bytes(0x50, 0x08, 0x00, 0x00, 0x00);
}


void Programmer::sendClear() {
    transaction(0xac, 0x80, 0x00, 0x00);
}


void Programmer::flash(uint8_t hilo, unsigned int addr, uint8_t data) {
    transaction(0x40 + 8 * hilo, addr >> 8 & 0xFF, addr & 0xFF, data);
}


void Programmer::commit(unsigned int addr) {
    #ifdef DEBUG
    Serial.println("Commit page by [" + String(addr, HEX) + "]");
    #endif
    if (_prog_light_switch) {
        _prog_light_switch(true);
    } 
    transaction(0x4C, (addr >> 8) & 0xFF, addr & 0xFF, 0);
    delay(PTIME);
    if (_prog_light_switch) {
        _prog_light_switch(false);
    } 
}


uint8_t Programmer::getByte(const uint8_t * buf, int i, int len) {
    if (i < len) {
        return buf[i];
    }
    return 0xff;
}


void Programmer::sendImage(const uint8_t * buf, int len) {
    #ifdef DEBUG
    Serial.println("Send image " + String(len, DEC));
    #endif
    int loaded_num = 0;
    unsigned int addr = 0;
    unsigned int commit_addr = 0;
    while (loaded_num < len) {
        #ifdef DEBUG
        String page_str;
        #endif
        for (int i = 0; i < PAGE_LEN; ++i, ++addr) {
            uint8_t byte = getByte(buf, loaded_num++, len);
            flash(LOW, addr, byte);
            #ifdef DEBUG
            page_str += GetHexStr(byte) + ",";
            #endif
            byte = getByte(buf, loaded_num++, len);
            flash(HIGH, addr, byte);
            #ifdef DEBUG
            page_str += GetHexStr(byte) + ",";
            #endif
        }
        commit(commit_addr);
        #ifdef DEBUG
        Serial.println("Write: " + page_str);
        #endif
        commit_addr = addr;
    }
}

uint8_t Programmer::read(uint8_t hilo, unsigned int addr) {
    return transaction(0x20 + hilo * 8, (addr >> 8) & 0xFF, addr & 0xFF, 0).d;
}


bool Programmer::werify(const uint8_t *buf, int len) {
    #ifdef DEBUG
    Serial.println("Werify image. ----------------------------------");
    #endif
    bool is_ok = true;
    int readed_num = 0;
    unsigned int addr = 0;
    while (readed_num < len and is_ok) {
        for (int i = 0; i < PAGE_LEN and is_ok; ++i, ++addr) {
            is_ok &= getByte(buf, readed_num++, len) == read(LOW, addr);
            is_ok &= getByte(buf, readed_num++, len) == read(HIGH, addr);
        }
        #ifdef DEBUG
        Serial.println("Werify page is " + String((is_ok ? "TRUE" : "FALSE")));
        #endif
    }
    return is_ok;
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


Programmer::Programmer(uint32_t need_chip_id) : _need_chip_id(need_chip_id) 
{}


Programmer::~Programmer() 
{}


void Programmer::setFlashFunc(const std::function<void(bool)> &prog_light_switch) {
    _prog_light_switch = prog_light_switch;
}


void Programmer::setErrorFunc(const std::function<void(bool)> &error_light_switch) {
    _error_light_switch = error_light_switch;
}


void Programmer::load(const uint8_t *buf, int len) {
    #ifdef DEBUG
    Serial.println("Load " + String(len, DEC) + " bytes");
    #endif
    if (setPmode()) {
        uint32_t chip_id = getChipId();
        #ifdef DEBUG
        Serial.println("Chip: " + String(chip_id, HEX));
        #endif
        if (_need_chip_id == chip_id) {
            uint32_t lf = getLowFuses();
            uint32_t hf = getHighFuses();
            uint32_t ef = getExtFuses();
            #ifdef DEBUG
            Serial.println("Fuses: " + String(lf, HEX) + ", " + String(hf, HEX) + ", " + String(ef, HEX));
            #endif
            sendClear();
            delay(PTIME);
            #ifdef DEBUG
            Serial.println("Chip is clearing.");
            #endif
            sendImage(buf, len);
            delay(PTIME);
            werify(buf, len);
            delay(PTIME);
            endPmode();
        } else {
            #ifdef DEBUG
            Serial.println("Chip is incorrect.");
            #endif
            if (_error_light_switch) {
                _error_light_switch(true);
                delay(PTIME);
                _error_light_switch(false);
            }
        }
    } else {
        #ifdef DEBUG
        Serial.println("Can`t set PMODE");
        #endif
        if (_error_light_switch) {
            _error_light_switch(true);
            delay(PTIME);
            _error_light_switch(false);
        }
    }
}
