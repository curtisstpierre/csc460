/*
 *  project2.cpp
 *
 *  Created: March, 2015
 *  Author: Mark Roller and Curtis St. Pierre
 *
 */

#include "os.h"
#include "BlockingUART.h"
#include <util/delay.h>
#include <avr/io.h>
#include <stdbool.h>

#define F_CPU 16000000UL

SERVICE* service;

int16_t system_value;
int16_t rr_value;
int16_t periodic_value;


/***************************
 * * * * * * * * * * * * * *
 *     Basic Functions     *
 * * * * * * * * * * * * * *
 ***************************/

/* Uses: Test02, Test03, Test04, Test05, Test07, Test09 */
void rr1(){
	for(;;) {
		PORTB |= 1 << PB0;
		_delay_ms(1);
		PORTB ^= 1 << PB0;
	}
}

/* Uses: Test03, Test04, Test05, Test07 */
void rr2(){
	for(;;) {
		PORTB |= 1 << PB1;
		_delay_ms(1);
		PORTB ^= 1 << PB1;
	}
}

/* Uses: Test03, Test04, Test05, Test07 */
void rr3(){
	for(;;) {
		PORTB |= 1 << PB2;
		_delay_ms(1);
		PORTB ^= 1 << PB2;
	}
}

/* Uses: Test03, Test04, Test05, Test07 */
void periodic1(){
	for(;;) {
		PORTB |= 1 << PB3;
		_delay_ms(5);
		PORTB ^= 1 << PB3;
		Task_Next();
	}
}

/* Uses: Test04, Test05, Test07 */
void periodic2(){
	for(;;) {
		PORTB |= 1 << PB4;
		_delay_ms(5);
		PORTB ^= 1 << PB4;
		Task_Next();
	}
}

/* Uses: Test01 */
void system1(){
	PORTB |= 1 << PB0;
	_delay_ms(5);
	PORTB ^= 1 << PB0;
	Task_Next();
}

/***************************
 * * * * * * * * * * * * * *
 *    Special Functions    *
 * * * * * * * * * * * * * *
 ***************************/

/* Uses: Test01 */
void periodic_with_system(){
	for(;;) {
		PORTB |= 1 << PB1;
		_delay_ms(5);
		Task_Create_System(system1, 0);
		_delay_ms(5);
		PORTB ^= 1 << PB1;
		Task_Next();
	}
}

/* Uses: Test06 */
void periodic_no_task_next(){
	for(;;) {
		PORTB |= 1 << PB0;
		_delay_ms(5);
		PORTB ^= 1 << PB0;
	}
}

/* Uses: Test08 */
void rr_now(){
	for(;;) {
		if(Now()%5 == 0) {
            PORTB |= 1 << PB0;
        } else {
            PORTB ^= 1 << PB0;
        }
    }
 }
        
/***************************
 * * * * * * * * * * * * * *
 *    Service Functions    *
 * * * * * * * * * * * * * *
 ***************************/

/* Uses: Test09 */
void service_publisher(){
	for(;;) {
		PORTB |= 1 << PB3;
		Service_Publish(service, 255);
		_delay_ms(5);
		PORTB ^= 1 << PB3;
		Task_Next();
	}
}

/* Uses: Test09 */
void system_subscriber(){
	for(;;) {
		Service_Subscribe(service, &system_value);
		PORTB |= 1 << PB2;
		_delay_ms(5);
		PORTB ^= 1 << PB2;
	}
}

/* Uses: Test09 */
void rr_subscriber(){
	for(;;) {
		Service_Subscribe(service, &rr_value);
		PORTB |= 1 << PB1;
		_delay_ms(1);
		PORTB ^= 1 << PB1;
	}
}

/* Uses: Test11 */
void periodic_subscriber(){
	for(;;) {
		Service_Subscribe(service, &periodic_value);
		PORTB |= 1 << PB0;
		_delay_ms(1);
		PORTB ^= 1 << PB0;
	}
}


/***************************
 * * * * * * * * * * * * * *
 *     Test Functions      *
 * * * * * * * * * * * * * *
 ***************************/

void setup(){
	DDRB |= 1 << PB4; //initialize PIN 10
	DDRB |= 1 << PB3; //initialize PIN 50
	DDRB |= 1 << PB2; //initialize PIN 51
	DDRB |= 1 << PB1; //initialize PIN 52
	DDRB |= 1 << PB0; //initialize PIN 53
	
	//PORTB &= 0 << PB4;
	//PORTB &= 0 << PB3;
	//PORTB &= 0 << PB2;
	//PORTB &= 0 << PB1;
	//PORTB &= 0 << PB0;
}

/* 
 * This test shows that a single system task can be run 
 */
void test01_system(){
	Task_Create_Periodic(periodic_with_system, 0, 5, 1, 5);
}

/* 
 * This test shows that a single rr task can be run 
 */
void test02_rr(){
	Task_Create_RR(rr1, 0);
}

/* 
 * This test shows that a single periodic task can be run 
 * amidst other tasks
 */
void test03_periodic(){
	Task_Create_Periodic(periodic1, 0, 5, 2, 0);
	Task_Create_RR(rr1, 0);
	Task_Create_RR(rr2, 0);
	Task_Create_RR(rr3, 0);
}

/* 
 * This test shows that two periodic tasks can be run 
 * amidst other tasks.
 */
void test04_multiple_periodic(){
	Task_Create_Periodic(periodic1, 0, 5, 2, 0);
	Task_Create_Periodic(periodic2, 0, 5, 2, 3);
	Task_Create_RR(rr1, 0);
	Task_Create_RR(rr2, 0);
	Task_Create_RR(rr3, 0);
}

/* 
 * This test should fail due to task collision
 * ERR_6_PERIODIC_TASK_COLLISION: (pin 12) shows 6 pulses
 */
void test05_multiple_periodic_collision_error(){
	Task_Create_Periodic(periodic1, 0, 5, 2, 0);
	Task_Create_Periodic(periodic2, 0, 5, 2, 10);
	Task_Create_RR(rr1, 0);
	Task_Create_RR(rr2, 0);
	Task_Create_RR(rr3, 0);
}

/* 
 * This test should fail due to the periodic task taking to long
 * ERR_RUN_3_PERIODIC_TOOK_TOO_LONG: (pin 11) shows 3 pulses.
 */
void test06_periodic_too_long_error(){
	Task_Create_Periodic(periodic_no_task_next, 0, 5, 2, 10);
}

/* 
 * This test should fail due to the too many tasks error.
 * ERR_RUN_2_TOO_MANY_TASKS: (pin 11) shows 2 pulses.
 */
void test07_too_many_tasks_error(){
	Task_Create_Periodic(periodic1, 0, 5, 2, 10);
	Task_Create_RR(rr1, 0);
	Task_Create_RR(rr2, 0);
	Task_Create_RR(rr3, 0);
	Task_Create_Periodic(periodic2, 0, 5, 2, 7);
	Task_Create_RR(rr1, 0);
	Task_Create_RR(rr2, 0);
	Task_Create_RR(rr3, 0);
}

/*
 * This test uses the Now() function to get the run time of the 
 * rtos. It uses the time to take action every 5 ms.
 */
void test08_now(){
	rr_now();
}

/*
 * This test shows that one can sucessfully subscribe/publish 
 * from a service. Changing the period on the publisher
 * task shows that the subscribers are triggered after it is.
 */
void test09_service(){
	service = Service_Init();

	Task_Create_System(system_subscriber, 0);
    Task_Create_RR(rr_subscriber, 0);
    Task_Create_RR(rr1, 0);
    Task_Create_Periodic(service_publisher, 0, 5, 1, 5);
}

/*
 * This test should throw an error regarding too many services
 * being created (we have a max of 4 - but it can be changed).
 * ERR_RUN_6_INIT_SERVICE_MAX_ERROR
 */
void test10_too_many_services_error(){
	service = Service_Init();
	service = Service_Init();
	service = Service_Init();
	service = Service_Init();
	service = Service_Init();
}

/*
 * This test should throw an error regarding attempting to add
 * a periodic function to a service.
 * ERR_RUN_7_SUBSCRIBE_PERIODIC
 */
void test11_subscribe_periodic_error(){
	service = Service_Init();

    Task_Create_Periodic(periodic_subscriber, 0, 5, 1, 5);
}

/***************************
 * * * * * * * * * * * * * *
 *      Main Function      *
 * * * * * * * * * * * * * *
 ***************************/

int r_main(){
	setup();
	//test01_system();
	//test02_rr();
	//test03_periodic();
	//test04_multiple_periodic();
	//test05_multiple_periodic_collision_error();
	//test06_periodic_too_long_error();
	//test07_too_many_tasks_error();
	//test08_now();
	//test09_service();
	//test10_too_many_services_error();
	//test11_subscribe_periodic_error();

	return 0;
}
