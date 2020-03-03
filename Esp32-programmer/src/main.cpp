#include <Arduino.h>
#include <functional>

#include "Programmer.hpp"


static const uint32_t DEFAULT_SERIAL_SPEED = 115200;


class Lights {
  int _blue_pin;
  int _red_pin;

public:
    Lights(int blue_pin, int red_pin) 
      : _blue_pin(blue_pin)
      , _red_pin(red_pin) {
        pinMode(_blue_pin, OUTPUT);
        pinMode(_red_pin, OUTPUT);
        delay(10);
    }

    ~Lights() 
    {}

    void blue(bool swch) {
        digitalWrite(_blue_pin, (swch ? HIGH : LOW));
    }

    void red(bool swch) {
        digitalWrite(_red_pin, (swch ? HIGH : LOW));
    }
};


void setup() {
    Serial.begin(DEFAULT_SERIAL_SPEED);
    delay(300);
    Serial.println("INIT Programmer.");
    Lights ls(2, 4);
    Programmer __prg;
    __prg.setFlashFunc(std::bind(&Lights::blue, &ls, std::placeholders::_1));
    __prg.setErrorFunc(std::bind(&Lights::red, &ls, std::placeholders::_1));
    Serial.println("Start load.");
    __prg.load(CODE, sizeof(CODE));
    delay(300);
    Serial.println("Loading compette.");
}


void loop() {
}