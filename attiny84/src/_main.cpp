#include <avr/pgmspace.h>
#include <avr/sleep.h>
#include <avr/wdt.h>
#include <avr/power.h>

#include <Arduino.h>
#include <Wire.h>

// sleep bit patterns:
#define WDT_TIMEOUT 0b000101 ///< 0.5 секунды.
//#define WDT_TIMEOUT 0b000110 ///<   1 second:
//#define WDT_TIMEOUT 0b000111 ///<   2 seconds:
//#define WDT_TIMEOUT 0b100000 ///<   4 seconds:
//#define WDT_TIMEOUT 0b100001 ///<   8 seconds:

//#define SEND_TIMEOUT 60 * 60 * 24 ///< Таймаут из ~0.5 секундных интервалов.
#define SEND_TIMEOUT 1 ///< Таймаут из ~0.5 секундных интервалов.
#define WIRE_ADDRESS 8

const uint8_t ESP_READY = 100;
const uint8_t ESP_OK = 101;
const uint8_t ESP_ERR = 102;

#define A0_PIN 10


volatile uint16_t __wdt = 0;
////////////////////////////////////////////////////////////////////////////////


void SendCounters() {
    noInterrupts();
    wdt_disable();
    power_all_enable();

    static bool _is_enable = true;
    if (_is_enable) {
      _is_enable = false;
      digitalWrite(A0_PIN, HIGH);
    } else {
      _is_enable = true;
      digitalWrite(A0_PIN, LOW);
    }

    /// Разбудить ESP.
    /// Встать в режим ожидания отправки данных. (I2C)
    /// Отправка. (I2C)
    /// Ожидание завершения работы ESP. (I2C)

    interrupts();
}


void EnableSleep() {

}


bool AttemptByError() {
    static uint8_t attempt = 0;
    if (attempt < 10) {
        ++attempt;
        return true;
    } else {
        return false;
    }
}

void ReceiveEvent(int how_many) {
    while (1 < Wire.available()) { // loop through all but the last
        uint8_t b = Wire.read(); // receive byte as a character
        switch (b) {
            case ESP_READY:
                SendCounters();
                break;
            case ESP_OK:
                EnableSleep();
                return;
            case ESP_ERR:
            default:
                AttemptByError();
                break;
        }
    }
}

void SetupSlaveWire() {
    Wire.begin(WIRE_ADDRESS);                // join i2c bus with address #8
    Wire.onReceive(ReceiveEvent); // register event
    Serial.begin(9600);           // start serial for output
}
////////////////////////////////////////////////////////////////////////////////


ISR(WDT_vect) {
    ++__wdt;
}


void WatchdogEnable(const uint8_t interval) {
    noInterrupts();
    __wdt = 0;

    wdt_reset();

    MCUSR = 0;                          // reset various flags
    WDTCSR |= 0b00011000;               // see docs, set WDCE, WDE
    WDTCSR =  0b01000110 | interval;    // set WDIE, and appropriate delay

    ADCSRA &= ~_BV(ADEN);

    power_all_disable();  // power off ADC, Timer 0 and 1, serial interface
    set_sleep_mode(SLEEP_MODE_PWR_DOWN);
    sleep_enable();
    interrupts();
    sleep_mode();
}

struct Controller {
  uint16_t _bt1_count;
  uint16_t _bt2_count;
  uint16_t _bt3_count;
  uint16_t _bt4_count;
  uint16_t _bt5_count;
  uint16_t _bt6_count;

  Controller()
    : _bt1_count(0)
    , _bt2_count(0)
    , _bt3_count(0)
    , _bt4_count(0)
    , _bt5_count(0)
    , _bt6_count(0)
  {}

  void espUp() {

  }

  void waitEspOk() {

  }

  void sendCounters() {

  }
};
///////////////////////////////////////////////////////////////////////////////////////////////////

void setup() {
  pinMode(PIN_A0, OUTPUT);
  digitalWrite(PIN_A0, LOW);
  WatchdogEnable(WDT_TIMEOUT);
}


void loop() {
  if (__wdt < SEND_TIMEOUT) {
      sleep_enable();
      sleep_mode();
  } else {
      /// Выполнить отправку.
      SendCounters();
      /// WDT interrupt is disabled (from WDT ISR)
      WatchdogEnable(WDT_TIMEOUT);
  }
}
