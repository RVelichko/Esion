#include <stdlib.h>
 
#include <avr/io.h>        // Adds useful constants
#include <util/delay.h>    // Adds delay_ms and delay_us functions
  
#include <avr/sleep.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>
#include <avr/wdt.h>
#include <avr/power.h>

#include <Arduino.h>


#define PIN_IN_1 0
#define PIN_IN_2 1
#define PIN_IN_3 11
#define PIN_IN_4 2
#define PIN_IN_5 3
#define PIN_IN_6 4

#define PIN_OUT_1 10
#define PIN_OUT_2 9
#define PIN_OUT_3 8
#define PIN_OUT_4 7
#define PIN_OUT_5 6
#define PIN_OUT_6 5


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
volatile uint8_t f_flags = 0;
 

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
	cbi(ADCSRA,ADEN);                    // switch Analog to Digitalconverter OFF
	set_sleep_mode(SLEEP_MODE_PWR_DOWN); // sleep mode is set here
	sleep_enable();
	sleep_mode();                        // System actually sleeps here
	sleep_disable();                     // System continues execution here when watchdog timed out 
	sbi(ADCSRA,ADEN);                    // switch Analog to Digitalconverter ON
}


void test() {
	digitalWrite(PIN_OUT_1, HIGH);
	_delay_ms(10);
	digitalWrite(PIN_OUT_1, LOW);
}


void CheckPin(int in_pin, int out_pin, uint8_t mask) {
	bool is_pin = digitalRead(in_pin);
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
	CheckPin(PIN_IN_1, PIN_OUT_1, 0x01);
	CheckPin(PIN_IN_2, PIN_OUT_2, 0x02);
	CheckPin(PIN_IN_3, PIN_OUT_3, 0x04);
	CheckPin(PIN_IN_4, PIN_OUT_4, 0x08);
	CheckPin(PIN_IN_5, PIN_OUT_5, 0x10);
	CheckPin(PIN_IN_6, PIN_OUT_6, 0x20);
	_delay_ms(10);
	digitalWrite(PIN_OUT_1, LOW);
	digitalWrite(PIN_OUT_2, LOW);
	digitalWrite(PIN_OUT_3, LOW);
	digitalWrite(PIN_OUT_4, LOW);
	digitalWrite(PIN_OUT_5, LOW);
	digitalWrite(PIN_OUT_6, LOW);
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

	InitOutPin(PIN_OUT_1);
	InitOutPin(PIN_OUT_2);
	InitOutPin(PIN_OUT_3);
	InitOutPin(PIN_OUT_4);
	InitOutPin(PIN_OUT_5);
	InitOutPin(PIN_OUT_6);

  	pinMode(PIN_IN_1, INPUT);
  	pinMode(PIN_IN_2, INPUT);
  	pinMode(PIN_IN_3, INPUT);
  	pinMode(PIN_IN_4, INPUT);
  	pinMode(PIN_IN_5, INPUT);
  	pinMode(PIN_IN_6, INPUT);
	
	SetupWatchdog(5); // approximately 0.5 seconds sleep
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
