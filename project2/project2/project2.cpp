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

/*toggle PIN 53*/
void system() {
	PORTB |= 1 << PB0;
	_delay_ms(5);
	PORTB ^= 1 << PB0;
	Task_Next();
}

/*toggle PIN 13*/
void periodic1(){
	for(;;) {
		PORTB |= 1 << PB7;
		_delay_ms(5);
		Task_Create_System(system, 0);
		_delay_ms(5);
		PORTB ^= 1 << PB7;
		Task_Next();
	}
}

int16_t system_value;
int16_t rr_value;

/*toggle PIN 50 */
void service_publisher(){
	for(;;) {
		PORTB |= 1 << PB3;
		Service_Publish(service, 255);
		_delay_ms(5);
		PORTB ^= 1 << PB3;
		Task_Next();
	}
}

/*toggle PIN 51*/
void system_service_subscriber(){
	for(;;) {
		PORTB |= 1 << PB2;
		Service_Subscribe(service, &system_value);
		_delay_ms(5);
		PORTB ^= 1 << PB2;
	}
}

/*toggle PIN 12*/
void rr_service_subscriber(){
	for(;;) {
		PORTB |= 1 << PB6;
		Service_Subscribe(service, &rr_value);
		_delay_ms(1);
		PORTB ^= 1 << PB6;
	}
}

/*toggle PIN 12*/
void rr1(){
	for(;;) {
		PORTB |= 1 << PB1;
		_delay_ms(1);
		PORTB ^= 1 << PB1;
	}
}

/*toggle PIN 11*/
void rr2(){
	for(;;) {
		PORTB |= 1 << PB0;
		_delay_ms(1);
		PORTB ^= 1 << PB0;
	}
}

/*toggle PIN 10*/
void rr3(){
	for(;;) {
		PORTB |= 1 << PB4;
		_delay_ms(1);
		PORTB ^= 1 << PB4;
	}
}

/*toggle PIN 51*/
void periodic2(){
	for(;;) {
		PORTB |= 1 << PB2;
		_delay_ms(5);
		PORTB ^= 1 << PB2;
		Task_Next();
	}
}

/*toggle PIN 52*/
void periodic3(){
	for(;;) {
		PORTB |= 1 << PB3;
		_delay_ms(5);
		PORTB ^= 1 << PB3;
		Task_Next();
	}
}

/*toggle PIN 52*/
void periodic3_no_task_next(){
	for(;;) {
		PORTB |= 1 << PB3;
		_delay_ms(5);
		PORTB ^= 1 << PB3;
	}
}

int r_main(){
	DDRB |= 1 << PB7; //initialize PIN 13
	DDRB |= 1 << PB6; //initialize PIN 12
	DDRB |= 1 << PB5; //initialize PIN 11
	DDRB |= 1 << PB4; //initialize PIN 10
	DDRB |= 1 << PB3; //initialize PIN 50
	DDRB |= 1 << PB2; //initialize PIN 51
	DDRB |= 1 << PB1; //initialize PIN 52
	DDRB |= 1 << PB0; //initialize PIN 53
	
	PORTB &= 0 << PB6;
	PORTB &= 0 << PB5;
	PORTB &= 0 << PB4;
	PORTB &= 0 << PB3;
	PORTB &= 0 << PB2;
	PORTB &= 0 << PB1;
	PORTB &= 0 << PB0;
	return 0;
}

int test_subscribe(){
	service = Service_Init();
	Task_Create_System(system_service_subscriber, 0);
	Task_Create_RR(rr_service_subscriber, 0);
	Task_Create_RR(rr2, 0);
	Task_Create_RR(rr3, 0);
	Task_Create_Periodic(service_publisher, 0, 25, 1, 5);
	return 0;
}

int test_period(){
	Task_Create_Periodic(periodic2, 0, 5, 2, 0);
	Task_Create_RR(rr1, 0);
	Task_Create_RR(rr2, 0);
	Task_Create_RR(rr3, 0);
	return 0;
}

int test_multiple_period(){
	Task_Create_Periodic(periodic2, 0, 5, 2, 0);
	Task_Create_Periodic(periodic3, 0, 5, 2, 3);
	Task_Create_RR(rr1, 0);
	Task_Create_RR(rr2, 0);
	Task_Create_RR(rr3, 0);
	return 0;
}

/* should fail PB6 (pin 12) error 6 pulses: task collision */
int test_multiple_period_fail(){
	Task_Create_Periodic(periodic2, 0, 5, 2, 0);
	Task_Create_Periodic(periodic3, 0, 5, 2, 10);
	Task_Create_RR(rr1, 0);
	Task_Create_RR(rr2, 0);
	Task_Create_RR(rr3, 0);
	return 0;
}

/* should fail PB5 (pin 11) error 3 pulses: periodic took too long */
int test_period_miss_fail(){
	Task_Create_Periodic(periodic3_no_task_next, 0, 5, 2, 10);
	Task_Create_RR(rr1, 0);
	Task_Create_RR(rr2, 0);
	Task_Create_RR(rr3, 0);
	return 0;
}

/* should fail PB5 (pin 11) error 2 pulses: too many tasks*/
int test_too_many_tasks_fail(){
	Task_Create_Periodic(periodic3_no_task_next, 0, 5, 2, 10);
	Task_Create_RR(rr1, 0);
	Task_Create_RR(rr2, 0);
	Task_Create_RR(rr3, 0);
	Task_Create_Periodic(periodic3_no_task_next, 0, 5, 2, 7);
	Task_Create_RR(rr1, 0);
	Task_Create_RR(rr2, 0);
	Task_Create_RR(rr3, 0);
	return 0;
}