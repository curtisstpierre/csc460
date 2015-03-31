/*
 * Sonar.c
 *
 *  Created on: Aug 17, 2013
 *      Author: andpol
 */
#define F_CPU 16000000UL
#include "Sonar.h"
#include <util/delay.h>

// DELTA_MULTIPLIER =  64 (prescaler) * 34029 (cm/s) / 16'000'000 (CPU freq) / 2 (there and back again time)
#define DELTA_MULTIPLIER 0.068058
#define SONAR_RX_DDR DDRE
#define SONAR_RX_PORT PORTE
#define SONAR_RX_PIN PE3  // pin 5

#define SONAR_PW_DDR DDRL
#define SONAR_PW_PIN PL1  // pin 48

uint16_t rising_timestamp;

void SonarInit() {
	sei();

	TCCR4A = 0;
	TCCR4B = 0;
	TCCR4C = 0;

	// Set input capture for rising edge
	TCCR4B |= _BV(ICES5);

	// Sonar RX pin configured for output (controlling the sonar)
	SONAR_RX_DDR |= _BV(SONAR_RX_PIN);
	// Sonar PW pin configured for input (capturing the PW signal from the sonar)
	SONAR_PW_DDR &= ~_BV(SONAR_PW_PIN);

	// Power-up delay as per specifications
	_delay_ms(250);

	SonarGetDistance();
}

#define START_TIMER \
	TCNT4 = 0;\
	TCCR4B |= _BV(ICES5);\
	TCCR4B |= (3 << CS50);\

#define STOP_TIMER \
	TCCR4B &= ~(7 << CS50);

int SonarGetDistance() {
	// Raise the request for the sonar
	SONAR_RX_PORT |= _BV(SONAR_RX_PIN);

	START_TIMER

	// Busy wait until rising edge
	while (!(TIFR5 & _BV(ICF5)))
		;
	// Clear the flag
	TIFR5 |= _BV(ICF5);

	rising_timestamp = ICR5;

	// Set input capture for falling edge
	TCCR5B &= ~_BV(ICES5);

	// Busy wait until falling edge
	while (!(TIFR5 & _BV(ICF5)))
		;
	// Clear the flag
	TIFR5 |= _BV(ICF5);

	// Lower the request for the sonar
	SONAR_RX_PORT &= ~_BV(SONAR_RX_PIN);

	STOP_TIMER

	// The falling edge triggered the ISR
	uint32_t delta = (uint32_t) ICR5 - (uint32_t) rising_timestamp;

	return delta * DELTA_MULTIPLIER;
}
