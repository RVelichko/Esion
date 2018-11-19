#include "Setup.h"

#include <avr/pgmspace.h>
#include <USIWire.h>

#include "Power.h"
#include "SlaveI2C.h"
#include "Storage.h"
#include "counter.h"
#include <avr/wdt.h>
#include <avr/sleep.h>
#include <avr/power.h>  


#define BUTTON_PIN1 0   
#define BUTTON_PIN2 1   
#define BUTTON_PIN3 11  
#define BUTTON_PIN4 2   
#define BUTTON_PIN5 3   
#define BUTTON_PIN6 4   

#define DEVICE_ID 5   	   // Модель устройства

#define ESP_POWER_PIN 10     // пин включения ESP8266. 


// Счетчики импульсов
static Counter counter_1(BUTTON_PIN1);
static Counter counter_2(BUTTON_PIN2);
static Counter counter_3(BUTTON_PIN3);
static Counter counter_4(BUTTON_PIN4);
static Counter counter_5(BUTTON_PIN5);
static Counter counter_6(BUTTON_PIN6);

// Класс для подачи питания на ESP и нажатия кнопки
static ESPPowerButton esp(ESP_POWER_PIN);

// Данные
struct Header info = {DEVICE_ID, 0, 0, 0, 0, {0, 0, 0, 0, 0, 0} };

//Кольцевой буфер для хранения показаний на случай замены питания или перезагрузки
//100к * 20 = 2 млн * 10 л / 2 счетчика = 10 000 000 л или 10 000 м3
static EEPROMStorage<Data> storage(20); // 8 byte * 20 + crc * 20

SlaveI2C slaveI2C;

volatile int wdt_count; // таймер может быть < 0 ?

/* Вектор прерываний сторожевого таймера watchdog */
ISR( WDT_vect ) { 
	wdt_count--;
}  

/* Подготовка сторожевого таймера watchdog */
void resetWatchdog() {
	MCUSR = 0; // очищаем все флаги прерываний
	WDTCSR = bit( WDCE ) | bit( WDE ); // allow changes, disable reset, clear existing interrupt
	WDTCSR = bit( WDIE ) | bit( WDP0 ) | bit( WDP1 );     // 128 ms
	#define ONE_MINUTE 480
	wdt_reset(); // pat the dog
} 

// Проверяем входы на замыкание. 
// Замыкание засчитывается только при повторной проверке.
inline void counting() {
	if (counter_1.check_close(info.data.value1)) {
		storage.add(info.data);
	}
	if (counter_2.check_close(info.data.value2)) {
		storage.add(info.data);
	}
	if (counter_3.check_close(info.data.value3)) {
		storage.add(info.data);
	}
	if (counter_4.check_close(info.data.value4)) {
		storage.add(info.data);
	}
	if (counter_5.check_close(info.data.value5)) {
		storage.add(info.data);
	}
	if (counter_6.check_close(info.data.value6)) {
		storage.add(info.data);
	}
}

// Настройка. Вызывается однократно при запуске.
void setup() {
	info.service = MCUSR; //причина перезагрузки

	noInterrupts();
	ACSR |= bit( ACD ); //выключаем компаратор
	interrupts();
	resetWatchdog(); 
	adc_disable(); //выключаем ADC

	//pinMode(SETUP_BUTTON_PIN, INPUT); //кнопка на корпусе

	if (storage.get(info.data)) { //не первая загрузка
		info.resets = EEPROM.read(storage.size());
		info.resets++;
		EEPROM.write(storage.size(), info.resets);
	} else {
		EEPROM.write(storage.size(), 0);
	}
	//`DEBUG_CONNECT(9600); 
	//`LOG_DEBUG(F("==== START ===="));
	//`LOG_DEBUG(F("MCUSR"));
	//`LOG_DEBUG(info.service);
	//`LOG_DEBUG(F("RESET"));
	//`LOG_DEBUG(info.resets);
	//`LOG_INFO(F("Data:"));
	//`LOG_INFO(info.data.value0);
	//LOG_INFO(info.data.value1);
}

//// Проверка нажатия кнопки SETUP
//bool button_pressed() {
//
//	if (digitalRead(SETUP_BUTTON_PIN) == LOW)
//	{	//защита от дребезга
//		delayMicroseconds(20000);  //нельзя delay, т.к. power_off
//		return digitalRead(SETUP_BUTTON_PIN) == LOW;
//	}
//	return false;
//}

//// Замеряем сколько времени нажата кнопка в мс
//unsigned long wait_button_release() {
//
//	unsigned long press_time = millis();
//	while(button_pressed())
//		;  
//	return millis() - press_time;
//}


// Главный цикл, повторящийся раз в сутки или при настройке вотериуса
void loop() {
	// Отключаем все лишнее: ADC, Timer 0 and 1, serial interface
	power_all_disable();  

	// Режим сна
	set_sleep_mode( SLEEP_MODE_PWR_DOWN );

	// Настраиваем служебный таймер (watchdog)
	resetWatchdog(); 

	// Цикл опроса входов
	// Выход по прошествию WAKE_EVERY_MIN минут или по нажатию кнопки
	for (unsigned int i = 0; i < ONE_MINUTE; ++i)  {
		wdt_count = WAKE_EVERY_MIN; 
		while ( wdt_count > 0 ) {
			noInterrupts();
			counting(); //Опрос входов
            interrupts();
            sleep_mode();  // Спим (WDTCR)
		}
	}
		
	wdt_disable();        // disable watchdog
	power_all_enable();   // power everything back on

	//info.voltage = readVcc();   // Текущее напряжение
	storage.get(info.data);     // Берем из хранилища текущие значения импульсов
	
	//DEBUG_CONNECT(9600);
	//LOG_INFO(F("Data:"));
	//LOG_INFO(info.data.value0);
	//LOG_INFO(info.data.value1);

	// Если пользователь нажал кнопку SETUP, ждем когда отпустит 
	// иначе ESP запустится в режиме программирования (да-да кнопка на i2c и 2 пине ESP)
	// Если кнопка не нажата или нажата коротко - передаем показания 
	unsigned long wake_up_limit;
    //LOG_DEBUG(F("wake up for transmitting"));
    slaveI2C.begin(TRANSMIT_MODE);
    wake_up_limit = WAIT_ESP_MSEC; //15 секунд при передаче данных

	esp.power(true);
	//LOG_DEBUG(F("ESP turn on"));
	
	while (!slaveI2C.masterGoingToSleep() && !esp.elapsed(wake_up_limit)) {
		counting();
		delayMicroseconds(65000);
		//if (wait_button_release() > LONG_PRESS_MSEC) {
		//	break; // принудительно выключаем
		//}
	}

	esp.power(false);
	slaveI2C.end();			// выключаем i2c slave.

    slaveI2C.masterGoingToSleep();
	//if (!slaveI2C.masterGoingToSleep()) {
	//	LOG_ERROR(F("ESP wake up fail"));
	//}
}
