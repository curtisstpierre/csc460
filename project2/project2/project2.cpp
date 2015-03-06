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

/*
 * Uses: test02_Services
 */
void rr1(){
	for(;;) {
		PORTB |= 1 << PB5;
		_delay_ms(1);
		PORTB ^= 1 << PB5;
	}
}

/*
 * Uses: test01_Now
 */
void rrNow(){
	for(;;) {
		if(Now()%5 == 0) {
            PORTB |= 1 << PB4;
        } else {
            PORTB ^= 1 << PB4;
        }
	}
}
        
/***************************
 * * * * * * * * * * * * * *
 *    Service Functions    *
 * * * * * * * * * * * * * *
 ***************************/

/*
 * Uses: test02_Services
 */
void service_publisher(){
	for(;;) {
		PORTB |= 1 << PB3;
		Service_Publish(service, 255);
		_delay_ms(5);
		PORTB ^= 1 << PB3;
		Task_Next();
	}
}

/*
 * Uses: test02_Services
 */
void system_subscriber(){
	for(;;) {
		Service_Subscribe(service, &system_value);
		PORTB |= 1 << PB2;
		_delay_ms(5);
		PORTB ^= 1 << PB2;
	}
}

/*
 * Uses: test02_Services
 */
void rr_subscriber(){
	for(;;) {
		Service_Subscribe(service, &rr_value);
		PORTB |= 1 << PB6;
		_delay_ms(1);
		PORTB ^= 1 << PB6;
	}
}

/*
 * Uses: test04_SubscribePeriodicError
 */
void periodic_subscriber(){
	for(;;) {
		Service_Subscribe(service, &periodic_value);
		PORTB |= 1 << PB6;
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

/*
 * This test uses the Now() function to get the run time of the 
 * rtos. It uses the time to take action every 5 ms.
 */
void test01_Now(){
	rrNow();
}

/*
 * This test shows that one can sucessfully subscribe/publish 
 * from a service. Changing the period on the publisher
 * task shows that the subscribers are triggered after it is.
 */
void test02_Services(){
	service = Service_Init();

	Task_Create_System(system_subscriber, 0);
    Task_Create_RR(rr_subscriber, 0);
    Task_Create_RR(rr1, 0);
    Task_Create_Periodic(service_publisher, 0, 5, 1, 5);
}

/*
 * This test should throw an error regarding too many services
 * being created (we have a max of 4 - but it can be changed).
 */
void test03_TooManyServicesError(){
	service = Service_Init();
	service = Service_Init();
	service = Service_Init();
	service = Service_Init();
	service = Service_Init();
}

/*
 * This test should throw an error regarding attempting to add
 * a periodic function to a service.
 */
void test04_SubscribePeriodicError(){
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
	//test01_Now();
	//test02_Services();
	//test03_TooManyServicesError();
	//test04_SubscribePeriodicError();
	
	return 0;
}
