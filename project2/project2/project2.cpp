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

SERVICE* service;

void system() {
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

int16_t system_value;
int16_t rr_value;

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

void rr2(){
	for(;;) {
		PORTB |= 1 << PB5;
		_delay_ms(1);
		PORTB ^= 1 << PB5;
	}
}

void rr3(){
	for(;;) {
		PORTB |= 1 << PB4;
		_delay_ms(1);
		PORTB ^= 1 << PB4;
	}
}

int r_main(){
	DDRB |= 1 << PB5;
	DDRB |= 1 << PB6;
	DDRB |= 1 << PB4;
	DDRB |= 1 << PB3;
	DDRB |= 1 << PB2;
	
	PORTB &= 0 << PB5;
	PORTB &= 0 << PB6;
	PORTB &= 0 << PB4;
	PORTB &= 0 << PB3;
	PORTB &= 0 << PB2;
	service = Service_Init();
	Task_Create_System(system_service_subscriber, 0);
	Task_Create_RR(rr_service_subscriber, 0);
	Task_Create_RR(rr2, 0);
	Task_Create_RR(rr3, 0);
	Task_Create_Periodic(service_publisher, 0, 25, 1, 5);
	return 0;
}
