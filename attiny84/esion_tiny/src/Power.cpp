
#include "Power.h"
#include <Arduino.h>
#include <avr/sleep.h>
#include <avr/power.h>    
#include <avr/wdt.h>
#include "Setup.h"


ESPPowerButton::ESPPowerButton(const uint8_t p)
	: power_pin(p)
	, power_on(false)
{
	pinMode(power_pin, INPUT);
}

void ESPPowerButton::power(const bool on)
{
	power_on = on;
	
	if (on)
	{
		pinMode(power_pin, OUTPUT);
		digitalWrite(power_pin, HIGH);
		wake_up_timestamp = millis();
	}
	else
	{
		delayMicroseconds(50000);   // чтобы заснул
		digitalWrite(power_pin, LOW);
		pinMode(power_pin, INPUT);  // снижаем потребление
		wake_up_timestamp = 0;
	}
}

bool ESPPowerButton::elapsed(const unsigned long msec)
{
	return millis() - wake_up_timestamp > msec;
}


// Меряем напряжение питания Attiny85. 
// Есть погрешность, поэтому надо калбировать. Для каждой attiny будет своя константа 1126400L.
uint16_t readVcc() 
{
	// Read 1.1V reference against AVcc
	// set the reference to Vcc and the measurement to the internal 1.1V reference

	//Включаем ADC 
	adc_enable();

	#if defined(__AVR_ATmega32U4__) || defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__)
		ADMUX = _BV(REFS0) | _BV(MUX4) | _BV(MUX3) | _BV(MUX2) | _BV(MUX1);
	#elif defined (__AVR_ATtiny25__) || defined(__AVR_ATtiny45__) || defined(__AVR_ATtiny85__)
		ADMUX = _BV(MUX3) | _BV(MUX2);
	#endif 

	delay(2); // Wait for Vref to settle
	ADCSRA |= _BV(ADSC); // Start conversion
	while (bit_is_set(ADCSRA,ADSC)); // measuring

	uint8_t low  = ADCL; // must read ADCL first - it then locks ADCH 
	uint8_t high = ADCH; // unlocks both

	uint16_t result = (high<<8) | low;

	//result = 1126400L / result; // calculate Vcc (in mV); 1126400 = 1.1*1024*1000 // generally
	result = 1126400L / result; // calculate Vcc (in mV); 1168538 = 1.14115*1024*1000 // for this ATtiny

	//Выключаем ADC
	adc_disable();

	return result; //милиВольт
}