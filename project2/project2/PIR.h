/*
 * PIR.h
 *
 *  Created on: May 17, 2013
 *      Author: andpol
 */

#ifndef PIR_H_
#define PIR_H_

#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdbool.h>

/*
 * PIR Sensor Wiring:
 * 	- PIR[out] -> PD2
 */

void init_PIR() {
	// Set PB2 to input
	DDRD &= ~_BV(PD2);
}

inline bool get_PIR_state() {
	return PIND & _BV(PD5);
}

#endif /* PIR_H_ */
