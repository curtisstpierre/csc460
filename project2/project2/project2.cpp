/*
 * main.c
 *
 *  Created on: Jun 23, 2013
 *      Author: andpol
 *
 */

#include "os.h"
#include "PIR.h"
#include "Sonar.h"
#include "Roomba.h"
#include "BlockingUART.h"

#include <avr/io.h>
#include <stdbool.h>

#define NUM_TICKS_INTRUDER_TIMEOUT 1000
#define TURN_VELOCITY 100
#define DRIVE_VELOCITY 200
#define SONAR_EPSILON 5
#define FOLLOW_DISTANCE 100

enum PERIODIC_TASK {
	PIR_CHECK = 1, SONAR_PING, ROOMBA_SENSOR_QUERY, ROOMBA_DRIVE,
};

typedef enum {
	PATROLLING, INTRUDER_DETECTED,
} FSM;

typedef enum {
	DRIVE_FORWARDS = 1, DRIVE_REVERSE, DRIVE_LEFT, DRIVE_RIGHT, DRIVE_STOP,
} DRIVE_OPTION;

FSM fsm = PATROLLING;
DRIVE_OPTION drive_mode = DRIVE_STOP;

// Latest sensor readings
bool are_you_there = false;
int distance = 100;
ROOMBA_PACKET_1 roomba_sensor_data;

extern const unsigned int PT = 4;
extern const unsigned char PPP[] = { PIR_CHECK, 1, SONAR_PING, 10, ROOMBA_SENSOR_QUERY, 4, ROOMBA_DRIVE, 1 };

void P_TASK_pir_check() {
	for (;;) {
		are_you_there = get_PIR_state();
		Task_Next();
	}
}

void P_TASK_sonar_ping() {
	for (;;) {
		distance = (distance + get_sonar_distance()) / 2;
		Task_Next();
	}
}

void P_TASK_roomba_sensor_query() {
	for (;;) {
		roomba_sensor_data = roomba_sense_1();
		Task_Next();
	}
}

void P_TASK_roomba_drive() {
	for (;;) {
		switch (drive_mode) {
			case DRIVE_FORWARDS:
				roomba_drive(DRIVE_VELOCITY, RADIUS_STRAIGHT);
				break;
			case DRIVE_REVERSE:
				roomba_drive(-DRIVE_VELOCITY, RADIUS_STRAIGHT);
				break;
			case DRIVE_LEFT:
				roomba_drive(TURN_VELOCITY, 1);
				break;
			case DRIVE_RIGHT:
				roomba_drive(TURN_VELOCITY, -1);
				break;
			case DRIVE_STOP:
				roomba_drive(0, RADIUS_STRAIGHT);
				break;
			default:
				// NOP
				break;
		}
		Task_Next();
	}
}

void RR_TASK_fsm_transition() {
	static unsigned int intruder_detect_timestamp;
	for (;;) {
		if (are_you_there) {
			// If the PIR is triggered, go into the INTRUDER_DETECTED state
			fsm = INTRUDER_DETECTED;
			intruder_detect_timestamp = Now();
		} else if (fsm == INTRUDER_DETECTED) {
			unsigned int delta = Now() - intruder_detect_timestamp;
			if (delta >= NUM_TICKS_INTRUDER_TIMEOUT || roomba_sensor_data.virtual_wall) {
				fsm = PATROLLING;
			}
		}
		Task_Next();
	}
}

void RR_TASK_adjust_movement() {
	bool in_virtual_wall = false;
	for (;;) {
		if (fsm == PATROLLING) {
			if (roomba_sensor_data.bumps_wheeldrops & 2) {
				// Turn Right
				drive_mode = DRIVE_RIGHT;
			} else if (roomba_sensor_data.bumps_wheeldrops & 1) {
				// Turn Left
				drive_mode = DRIVE_LEFT;
			} else if (roomba_sensor_data.virtual_wall) {
				if (!in_virtual_wall) {
					// Turn Left
					drive_mode = DRIVE_LEFT;
					in_virtual_wall = true;
				}
			} else {
				// No relevant sensors triggered, drive straight
				drive_mode = DRIVE_FORWARDS;
				in_virtual_wall = false;
			}
		} else if (fsm == INTRUDER_DETECTED) {
			// Too close
			if (distance < FOLLOW_DISTANCE - SONAR_EPSILON) {
				drive_mode = DRIVE_REVERSE;
			}
			// Too far
			else if (distance > FOLLOW_DISTANCE + SONAR_EPSILON) {
				drive_mode = DRIVE_FORWARDS;
			}
			// Stay still
			else {
				drive_mode = DRIVE_STOP;
			}
		}
		Task_Next();
	}
}

extern int r_main() {

	init_PIR();
	init_sonar();
	init_roomba();

	_delay_ms(200);

	// Periodic tasks
	Task_Create(P_TASK_pir_check, 0, PERIODIC, PIR_CHECK);
	Task_Create(P_TASK_sonar_ping, 0, PERIODIC, SONAR_PING);
	Task_Create(P_TASK_roomba_sensor_query, 0, PERIODIC, ROOMBA_SENSOR_QUERY);
	Task_Create(P_TASK_roomba_drive, 0, PERIODIC, ROOMBA_DRIVE);

	// Round Robin Tasks
	Task_Create(RR_TASK_adjust_movement, 0, RR, 0);
	Task_Create(RR_TASK_fsm_transition, 0, RR, 0);

	return 0;
}
