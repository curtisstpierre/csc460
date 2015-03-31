/*
 * Sonar.h
 *
 *  Created on: May 17, 2013
 *      Author: andpol
 *
 *		Timer:
 *		Sonar uses TIMER4
 *
 *      Wiring:
 *       - Sonar[RX] -> PE3 - Arduino 5
 *       - Sonar[PW] -> PL0 (ICP4) - Arduino 49
 */

#ifndef SONAR_H_
#define SONAR_H_

#include <avr/interrupt.h>
#include <avr/io.h>

void SonarInit();

// Blocking call to get the sonar distance
int SonarGetDistance();


#endif /* SONAR_H_ */
