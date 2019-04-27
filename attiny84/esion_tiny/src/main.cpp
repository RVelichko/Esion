#include <stdlib.h>

#include <avr/io.h>        // Adds useful constants
#include <util/delay.h>    // Adds delay_ms and delay_us functions

#include <avr/sleep.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>
#include <avr/wdt.h>
#include <avr/power.h>

#include <Arduino.h>

//              Attiny84
//                 _
//          Vcc --| |-- Gnd
//  0        10 --| |-- 0         10
//  1         9 --| |-- 1         9
// 11       Rst --| |-- 2         8
//  2         8 --| |-- 3         7
//  3         7 --| |-- 4  (SCK)  6
//  4 (MOSI)  6 --|_|-- 5  (MISO) 5


#define PI_1 3 // 7
#define PI_2 2 // 8
#define PI_3 1 // 9
#define PI_4 0 // 10

#define PO_1 8 // 2
#define PO_2 7 // 3
#define PO_3 6 // 4
#define PO_4 5

static const uint8_t IMPULS_DEPTH = 10;

// Routines to set and claer bits (used in the sleep code)
#ifndef cbi
#define cbi(sfr, bit) (_SFR_BYTE(sfr) &= ~_BV(bit))
#endif
#ifndef sbi
#define sbi(sfr, bit) (_SFR_BYTE(sfr) |= _BV(bit))
#endif


#define INTERNAL2V56NC (6)  // We use the internal voltage reference


// Variables for the Sleep/power down modes:
volatile boolean f_wdt = 1;
volatile byte f_flags = 0x00;


// 0 = 16ms, 1 = 32ms, 2 = 64ms, 3 = 128ms, 4 = 250ms, 5 = 500ms
// 6 = 1sec, 7 = 2sec, 8 = 4sec, 9 = 8sec
void SetupWatchdog(int ii) {
    if (ii > 9 ) {
        ii = 9;
    }
    byte bb = ii & 7;
    if (ii > 7) {
        bb |= (1 << 5);
    }
    bb |= (1 << WDCE);

    MCUSR &= ~(1 << WDRF);
    // start timed sequence
    WDTCSR |= (1 << WDCE) | (1 << WDE);
    // set new watchdog timeout value
    WDTCSR = bb;
    WDTCSR |= _BV(WDIE);
}


// set system into the sleep state
// system wakes up when wtchdog is timed out
void SystemSleep() {
    cbi(ADCSRA, ADEN);                   // switch Analog to Digitalconverter OFF
    set_sleep_mode(SLEEP_MODE_PWR_DOWN); // sleep mode is set here
    sleep_enable();
    sleep_mode();                        // System actually sleeps here
    sleep_disable();                     // System continues execution here when watchdog timed out
    sbi(ADCSRA, ADEN);                   // switch Analog to Digitalconverter ON
}


//void test(int pin) {
//    digitalWrite(pin, HIGH);
//    _delay_ms(100);
//    digitalWrite(pin, LOW);
//    //_delay_ms(100);
//}


void CheckPin(int in_pin, int out_pin, uint8_t mask) {
    bool is_pin = (digitalRead(in_pin) == 1);
    bool is_flag = f_flags & mask;
    if (is_pin and not is_flag) {
        f_flags |= mask;
        digitalWrite(out_pin, HIGH);
    }
    if (not is_pin and is_flag) {
        f_flags ^= mask;
    }
}


void WorkInputs() {
    CheckPin(PI_1, PO_1, 0b00000001);
    CheckPin(PI_2, PO_2, 0b00000010);
    CheckPin(PI_3, PO_3, 0b00000100);
    CheckPin(PI_4, PO_4, 0b00001000);
    _delay_ms(IMPULS_DEPTH);
    digitalWrite(PO_1, LOW);
    digitalWrite(PO_2, LOW);
    digitalWrite(PO_3, LOW);
    digitalWrite(PO_4, LOW);
}


void InitOutPin(int pin) {
    pinMode(pin, OUTPUT);
    digitalWrite(pin, LOW);
}


// the setup routine runs once when you press reset:
void setup()  {
    // Set up FAST PWM
    TCCR0A = 2 << COM0A0 | 2 << COM0B0 | 3 << WGM00; // Set control register A for Timer 0
    TCCR0B = 0 << WGM02 | 1 << CS00;                 // Set control register B for Timer 0

    InitOutPin(PO_1);
    InitOutPin(PO_2);
    InitOutPin(PO_3);
    InitOutPin(PO_4);

    pinMode(PI_1, INPUT);
    pinMode(PI_2, INPUT);
    pinMode(PI_3, INPUT);
    pinMode(PI_4, INPUT);

    SetupWatchdog(5); // approximately 0.5 seconds sleep
    //SetupWatchdog(6); // approximately 0.5 seconds sleep
}


// the loop routine runs over and over again forever:
void loop()  {
    if (f_wdt == 1) {  // wait for timed out watchdog / flag is set when a watchdog timeout occurs
        f_wdt = 0;       // reset flag
        //test(PO_4);
        WorkInputs();
        SystemSleep();  // Send the unit to sleep
    }
}


// Watchdog Interrupt Service / is executed when watchdog timed out
ISR(WDT_vect) {
    f_wdt = 1;  // set global flag
}
