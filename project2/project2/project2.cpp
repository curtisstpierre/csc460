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
#include <avr/interrupt.h>

SERVICE* service1;
SERVICE* service2;

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
            PORTB &= 0 << PB0;
        }
    }
 }

 /* Uses: Test13 */
 void rr_arg(){
 	int16_t wait = Task_GetArg();
    int16_t count;
	for(;;) {
		PORTB |= 1 << PB0;
		for(count=0; count<wait; count++) {
			_delay_ms(1);
		}
		PORTB ^= 1 << PB0;
	}
 }
        
/***************************
 * * * * * * * * * * * * * *
 *    Service Functions    *
 * * * * * * * * * * * * * *
 ***************************/

/* Uses: Test09, Test14 */
void service_publisher(){
	for(;;) {
		PORTB |= 1 << PB3;
		Service_Publish(service1, 255);
		_delay_ms(5);
		PORTB ^= 1 << PB3;
		Task_Next();
	}
}

/* Uses: Test14 */
void service_publisher_rr(){
	for(;;) {
		PORTB |= 1 << PB4;
		Service_Publish(service2, 255);
		_delay_ms(5);
		PORTB ^= 1 << PB4;
	}
}

/* Uses: Test15 */
void service_publisher_values(){
	int16_t count = 0;

	for(;;) {
		PORTB |= 1 << PB3;
		Service_Publish(service1, count);
		_delay_ms(5);
		PORTB ^= 1 << PB3;

		count++;
		if(count > 5){
			count = 0;
		}
		Task_Next();
	}
}

/* Uses: Test09, Test14 */
void system_subscriber1(){
	for(;;) {
		Service_Subscribe(service1, &system_value);
		PORTB |= 1 << PB2;
		_delay_ms(5);
		PORTB ^= 1 << PB2;
	}
}

/* Uses: Test14 */
void system_subscriber2(){
	for(;;) {
		Service_Subscribe(service2, &system_value);
		PORTB |= 1 << PB1;
		_delay_ms(5);
		PORTB ^= 1 << PB1;
	}
}

/* Uses: Test15 */
void system_subscriber_values(){
	int16_t count;

	for(;;) {
		Service_Subscribe(service1, &system_value);
		PORTB |= 1 << PB2;
		for(count=0;count<system_value;count++){
			_delay_ms(1);
		}
		PORTB ^= 1 << PB2;
	}
}

/* Uses: Test09 */
void rr_subscriber(){
	for(;;) {
		Service_Subscribe(service1, &rr_value);
		PORTB |= 1 << PB1;
		_delay_ms(1);
		PORTB ^= 1 << PB1;
	}
}

/* Uses: Test11 */
void periodic_subscriber(){
	for(;;) {
		Service_Subscribe(service1, &periodic_value);
		PORTB |= 1 << PB0;
		_delay_ms(1);
		PORTB ^= 1 << PB0;
	}
}

ISR(TIMER3_COMPA_vect){
    PORTB |= 1 << PB3;
	Service_Publish(service1, 0);
	_delay_ms(5);
	PORTB ^= 1 << PB3;

	Task_Next();
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
 * ERR_5_PERIODIC_TASK_COLLISION: (pin 12) shows 5 pulses
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
	service1 = Service_Init();

	Task_Create_System(system_subscriber1, 0);
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
	service1 = Service_Init();
	service1 = Service_Init();
	service1 = Service_Init();
	service1 = Service_Init();
	service1 = Service_Init();
}

/*
 * This test should throw an error regarding attempting to add
 * a periodic function to a service.
 * ERR_RUN_7_SUBSCRIBE_PERIODIC
 */
void test11_subscribe_periodic_error(){
	service1 = Service_Init();

    Task_Create_Periodic(periodic_subscriber, 0, 5, 1, 5);
}

/*
 * This test should throw an error regarding calling OS_Abort().
 * ERR_RUN_1_USER_CALLED_OS_ABORT
 */
void test12_user_called_abort_error(){
	OS_Abort();
}

/*
 * This test demonstrates that a argument can be used to delay a round 
 * robin by the number of milliseconds specified.
 */
void test13_arguments(){
	Task_Create_RR(rr_arg, 4);
}

/*
 * This tests that multiple services can run at the same time.
 */
void test14_multiple_services(){
	service1 = Service_Init();
	service2 = Service_Init();

	Task_Create_System(system_subscriber1, 0);
	Task_Create_System(system_subscriber2, 0);
    Task_Create_Periodic(service_publisher, 0, 5, 1, 5);
    Task_Create_RR(service_publisher_rr, 0);
}

/*
 * This tests that the services value passing works.
 */
void test15_services_values(){
	service1 = Service_Init();

	Task_Create_System(system_subscriber_values, 0);
    Task_Create_Periodic(service_publisher_values, 0, 5, 1, 5);
}

/* 
 * test  ERR_1_PERIOD_LT_WCET: (pin 12) shows 1 pulses
 */
void test16_period_lt_wcet(){
	Task_Create_Periodic(periodic1, 0, 5, 7, 10);
}

/*
 * test  ERR_4_PERIODIC_PERIOD_LT_ONE: (pin 12) shows 4 pulses
 */
void test17_period_lt_one(){
	Task_Create_Periodic(periodic1, 0, 0, 0, 10);
}

/* 
 * test  ERR_3_PERIODIC_START_BEFORE_ZERO: (pin 12) shows 3 pulses
 */
void test18_periodic_start_before_zero(){
	Task_Create_Periodic(periodic1, 0, 5, 2, -1);
}

/* 
 * This test makes sure you can succesfully publish a service from an interrupt
 * The service should be published every 2 seconds
 */
void test19_ISR_publish(){
    service1 = Service_Init(); 

    //Clear config
    TCCR3A = 0;
    TCCR3B = 0;

    //Set CTC 4
    TCCR3B |= (1 << WGM32); 

    //Set prescaler to 1
    TCCR3B |= (1<<CS30); 
    
    //Set value (0.05 seconds)
    OCR3A = 8000; 

    //Enable interrupt
    TIMSK3 |= (1<<OCIE3A); 

    //Set timer to 0
    TCNT3=0;               


    Task_Create_System(system_subscriber1, 0);
    Task_Create_RR(rr_subscriber, 0);
    Task_Create_RR(rr1, 0);
}

/***************************
 * * * * * * * * * * * * * *
 *      Main Function      *
 * * * * * * * * * * * * * *
 ***************************/

int r_main(){
	setup();
	//test01_system(); //creates a system task in a periodic task
	//test02_rr(); //creates a round robin task
	//test03_periodic(); //creates a periodic with round robins
	//test04_multiple_periodic(); // creates multiple periodic tasks and round robins
	//test05_multiple_periodic_collision_error(); //test periodic collision
	//test06_periodic_too_long_error(); //test periodic without TaskNext() fails
	//test07_too_many_tasks_error();
	//test08_now();
	//test09_service();
	//test10_too_many_services_error();
	//test11_subscribe_periodic_error();
	//test12_user_called_abort_error();
	//test13_arguments();
	//test14_multiple_services();
	//test15_services_values();
	//test16_period_lt_wcet();
	//test17_period_lt_one();
	//test18_periodic_start_before_zero();
	//test19_ISR_publish();

	return 0;
}