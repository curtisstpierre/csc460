/*
 * main.c
 *
 *  Created on: March, 2014
 *      Author: Mark and Curtis
 *
 */
#define F_CPU 16000000UL
#include "os.h"
#include "BlockingUART.h"

#include <util/delay.h>
#include <avr/io.h>
#include <stdbool.h>

enum PERIODIC_TASK {
	PIR_CHECK = 1, SONAR_PING, ROOMBA_SENSOR_QUERY, ROOMBA_DRIVE,
};

extern const unsigned int PT = 4;
extern const unsigned char PPP[] = { PIR_CHECK, 1, SONAR_PING, 10, ROOMBA_SENSOR_QUERY, 4, ROOMBA_DRIVE, 1 };

void foo(){
    DDRB |= 1 << PB7;
    for(;;){
        _delay_ms(500);
        PORTB ^= 1 << PB7;
    }
}

void foo1(){
	_delay_ms(500);
	DDRB |= 1 << PB6;
	for(;;){
		_delay_ms(500);
		PORTB ^= 1 << PB6;
	}
}

void foo2(){
	_delay_ms(800);
	DDRB |= 1 << PB5;
	for(;;){
		_delay_ms(500);
		PORTB ^= 1 << PB5;
	}
}

void fooNow(){
    uint16_t val = 0;
    uint16_t i = 0; 

    DDRB = 1 << 7;          
    while(1){
        _delay_ms(500);  
        for(;;){
            val = Now()%5; 
            if(val == 3){
                PORTB ^= 1 << 7; 
                break; 
            }
        }
    }
}

extern int r_main() {
	_delay_ms(200);

	// Round Robin Tasks
	Task_Create(foo, 0, RR, 0);
	Task_Create(foo1, 0, RR, 0);
	Task_Create(foo2, 0, RR, 0);

	//Service_Init();
	return 0;
}
