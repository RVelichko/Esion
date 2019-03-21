#include <Arduino.h>

// (RESET)  5  |*  | VCC
//    (ON)  3  |   |  2   (SCK)
// (BT EN)  4  |   |  1   (MISO)
//         GND |   |  0   (MOSI)

static const uint32_t ENABLE_PIN = 4;
static const uint32_t ON_KEY_PIN = 3;
static const uint32_t DEFAULT_SERIAL_SPEED = 115200;


void setup() {
    pinMode(ENABLE_PIN, INPUT);
    pinMode(ON_KEY_PIN, OUTPUT);
    //digitalWrite(ON_KEY_PIN, HIGH);
    digitalWrite(ON_KEY_PIN, LOW);
}

void loop() {
    auto en = digitalRead(ENABLE_PIN);
    digitalWrite(ON_KEY_PIN, en);
    delay(100);
}