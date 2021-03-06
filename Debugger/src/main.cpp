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


#define PO_1 10
#define PO_2 9
#define PO_3 8
#define PO_4 7

// Routines to set and claer bits (used in the sleep code)
#ifndef cbi
#define cbi(sfr, bit) (_SFR_BYTE(sfr) &= ~_BV(bit))
#endif
#ifndef sbi
#define sbi(sfr, bit) (_SFR_BYTE(sfr) |= _BV(bit))
#endif


int __pins[] = { PO_1, PO_2, PO_3, PO_4 };


#define INTERNAL2V56NC (6)  // We use the internal voltage reference
#define CORRECTING_DELTA 3


// Variables for the Sleep/power down modes:
volatile boolean f_wdt = 1;
volatile byte f_flags = 0x00;
volatile int f_relax_count = -1;

volatile uint32_t f_ctrl_counts = 0;

union Counters {
    uint32_t i;
    uint8_t arr[4];
};


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


void CheckPin(int pin, uint8_t mask) {
    if (f_flags & mask) {
        f_flags ^= mask;
        digitalWrite(pin, LOW);
    } else {
        f_flags |= mask;
        digitalWrite(pin, HIGH);
    }
}


int GetCorreectedPinId(int pin_id) {
    Counters c;
    c.i = f_ctrl_counts;
    auto reset_fn = [&](uint8_t val) {
        for (uint8_t i = 0; i < sizeof(c.arr); ++i) {
            if (c.arr[i] < val) {
                c.arr[i] = val - c.arr[i];
            } else {
                c.arr[i] = c.arr[i] - val;
            }
        }
    };
    int cnt = c.arr[pin_id] + 1;
    for (uint8_t i = 0; i < sizeof(c.arr); ++i) {
        if (c.arr[i] < cnt and CORRECTING_DELTA <= (cnt - c.arr[i])) {
            reset_fn(cnt);
            pin_id = i;
            break;
        } else if (c.arr[i] == 255) {
            reset_fn(c.arr[i]);
            break;
        }
    }
    ++c.arr[pin_id];
    f_ctrl_counts = c.i;
    return pin_id;
}


void WorkInputs() {
    randomSeed(analogRead(0));
    if (random(100) < 72) {
        randomSeed(analogRead(1));
        //int rnd_pin = random(4);
        int rnd_pin = 0;
        int p = random(100);
        if (p < 25) {
            rnd_pin = 3;
        } else if (p < 50) {
            rnd_pin = 2;
        } else if (p < 75) {
            rnd_pin = 1;
        }
        int npin = GetCorreectedPinId(rnd_pin);
        CheckPin(__pins[npin], 1 << rnd_pin);
    } else  if (f_flags) {
        f_flags = 0x00;
        digitalWrite(PO_1, LOW);
        digitalWrite(PO_2, LOW);
        digitalWrite(PO_3, LOW);
        digitalWrite(PO_4, LOW);
    }
}


void InitOutPin(int pin) {
    pinMode(pin, OUTPUT);
    digitalWrite(pin, LOW);
    //digitalWrite(pin, HIGH);
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

    //SetupWatchdog(5); // approximately 0.5 seconds sleep
    SetupWatchdog(6); // approximately 1 seconds sleep
}


// the loop routine runs over and over again forever:
void loop()  {
    if (f_wdt == 1) {  // wait for timed out watchdog / flag is set when a watchdog timeout occurs
        f_wdt = 0;       // reset flag
        WorkInputs();
        SystemSleep();  // Send the unit to sleep
    }
}


// Watchdog Interrupt Service / is executed when watchdog timed out
ISR(WDT_vect) {
    f_wdt = 1;  // set global flag
}
