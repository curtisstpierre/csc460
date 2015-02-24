/*
 * Sonar.h
 *
 *  Created on: May 17, 2013
 *      Author: andpol
 *
 *		Timer:
 *		Sonar uses TIMER1
 *
 *      Wiring:
 *       - Sonar[PW] -> PB5
 *       - Sonar[RX] -> PB7
 */

#ifndef SONAR_H_
#define SONAR_H_

#include <avr/interrupt.h>
#include <avr/io.h>

// DELTA_MULTIPLIER =  64 (prescaler) * 34029 (cm/s) / 11'059'200 (CPU freq) / 2 (there and back again time)
//#define DELTA_MULTIPLIER 0.068058
#define DELTA_MULTIPLIER 0.09846

uint16_t rising_timestamp;

void init_sonar() {
	sei();
	// Setup timer 3 for Input Capture (ICP3/PB5)
	TCCR3B |= _BV(ICES3);
	// PB7 configured for output
	DDRB |= _BV(PB7);
}

inline void _start_timer() {
	TCNT3 = 0;
	// Set input capture for rising edge
	TCCR3B |= _BV(ICES3);
	// Start timer using a prescaler of 64
	TCCR3B |= (3 << CS10);
}

inline void _stop_timer() {
	// Set the timer to have "no clock source" - aka stop the timer
	TCCR3B &= ~(3 << CS10);
}

int get_sonar_distance() {
	// Raise the request for the sonar
	PORTB |= _BV(PB7);

	_start_timer();

	// Busy wait until rising edge
	while (!(TIFR3 & _BV(ICF3)))
		;
	TIFR3 |= _BV(ICF3);

	// Lower the request for the sonar
	PORTB &= ~_BV(PB7);

	rising_timestamp = ICR3;
	// Set input capture for falling edge
	TCCR3B &= ~_BV(ICES3);

	// Busy wait until falling edge
	while (!(TIFR3 & _BV(ICF3)))
		;
	TIFR3 |= _BV(ICF3);

	_stop_timer();
	// The falling edge triggered the ISR
	uint32_t delta = (uint32_t) ICR3 - (uint32_t) rising_timestamp;

	return delta * DELTA_MULTIPLIER;
}

#endif /* SONAR_H_ */
