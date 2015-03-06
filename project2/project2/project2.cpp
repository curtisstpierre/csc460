/*
 *  project2.cpp
 *
 *  Created: March, 2015
 *  Author: Mark Roller and Curtis St. Pierre
 *
 */

#define F_CPU 16000000UL
#include "os.h"
#include "BlockingUART.h"
#include <util/delay.h>
#include <avr/io.h>
#include <stdbool.h>

SERVICE* service;

int16_t system_value;
int16_t rr_value;


/***************************
 * * * * * * * * * * * * * *
 *     Task Functions      *
 * * * * * * * * * * * * * *
 ***************************/

void system1() {
	PORTB |= 1 << PB2;
	_delay_ms(5);
	PORTB ^= 1 << PB2;
	Task_Next();
}

void periodic1(){
	for(;;) {
		PORTB |= 1 << PB3;
		_delay_ms(5);
		Task_Create_System(system, 0);
		_delay_ms(5);
		PORTB ^= 1 << PB3;
		Task_Next();
	}
}

void periodic2(){
	for(;;) {
		PORTB |= 1 << PB2;
		_delay_ms(5);
		PORTB ^= 1 << PB2;
		Task_Next();
	}
}

void rr1(){
	for(;;) {
		PORTB |= 1 << PB5;
		_delay_ms(1);
		PORTB ^= 1 << PB5;
	}
}

void rr2(){
	for(;;) {
		PORTB |= 1 << PB4;
		_delay_ms(1);
		PORTB ^= 1 << PB4;
	}
}

void service_publisher(){
	for(;;) {
		PORTB |= 1 << PB3;
		Service_Publish(service, 255);
		_delay_ms(5);
		PORTB ^= 1 << PB3;
		Task_Next();
	}
}

void system_service_subscriber(){
	for(;;) {
		PORTB |= 1 << PB2;
		Service_Subscribe(service, &system_value);
		_delay_ms(5);
		PORTB ^= 1 << PB2;
	}
}

void rr_service_subscriber(){
	for(;;) {
		PORTB |= 1 << PB6;
		Service_Subscribe(service, &rr_value);
		_delay_ms(1);
		PORTB ^= 1 << PB6;
	}
}


/***************************
 * * * * * * * * * * * * * *
 *     Test Functions      *
 * * * * * * * * * * * * * *
 ***************************/

void setup(){
	DDRB |= 1 << PB5;
	DDRB |= 1 << PB6;
	DDRB |= 1 << PB4;
	DDRB |= 1 << PB3;
	DDRB |= 1 << PB2;
}

void test001_Now(){

}

int r_main(){
	setup();
	test001_Now();
	
	return 0;
}
