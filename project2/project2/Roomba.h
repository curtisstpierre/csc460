/*
 * Roomba.h
 *
 *  Created on: May 21, 2013
 *      Author: andpol
 *
 *
 *      Wiring:
 *      roomba[DD] -> PD4
 *      roomba[TX] -> RX1
 *      roomba[RX] -> TX1
 */

#ifndef ROOMBA_H_
#define ROOMBA_H_

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <stdarg.h> // For varargs
#include "BlockingUART.h"
#include <string.h>

#define RADIUS_STRAIGHT 0x8000

typedef enum {
	OC_START = 128,
	OC_BAUD,
	OC_CONTROL,
	OC_SAFE,
	OC_FULL,
	OC_POWER,
	OC_SPOT,
	OC_CLEAN,
	OC_MAX,
	OC_DRIVE,
	OC_MOTORS,
	OC_LEDS,
	OC_SONG,
	OC_PLAY,
	OC_SENSORS,
	OC_FORCE_SEEKING_DOCK,
} _ROOMBA_OPCODE;

typedef enum {
	ROOMBA_SENSOR_PACKET_ALL = 0,
	ROOMBA_SENSOR_PACKET_1,
	ROOMBA_SENSOR_PACKET_2,
	ROOMBA_SENSOR_PACKET_3,
} ROOMBA_SENSOR_PACKET;

typedef struct {
	uint8_t bumps_wheeldrops;
	uint8_t wall;
	uint8_t cliff_left;
	uint8_t cliff_front_left;
	uint8_t cliff_front_right;
	uint8_t cliff_right;
	uint8_t virtual_wall;
	uint8_t motor_overcurrents;
	uint8_t dirt_detector_left;
	uint8_t dirt_detector_right;
} ROOMBA_PACKET_1;

typedef struct {
	uint8_t remote_opcode;
	uint8_t buttons;
	uint16_t distance;
	uint16_t angle;
} ROOMBA_PACKET_2;

typedef struct {
	uint8_t charging_state;
	uint16_t voltage;
	uint16_t current;
	uint8_t temperature;
	uint16_t charge;
	uint16_t capacity;
} ROOMBA_PACKET_3;

/**
 *  Send a command to the Roomba
 * @param opcode
 * 			the opcode of the command
 * @param num_data_bytes
 * 			the number of vararg data bytes to follow
 * @param ...
 * 			the variable argument data bytes to send
 */
void _roomba_send_command(_ROOMBA_OPCODE opcode, uint8_t num_data_bytes, const uint8_t * data) {
	uint8_t packet[num_data_bytes + 1];
	packet[0] = (uint8_t) opcode;
	memcpy(&packet[1], data, num_data_bytes);

	UART_send_raw_bytes(UART_CHANNEL_1, num_data_bytes + 1, packet);
}

void init_roomba() {
	UART_Init1(57600);

	// Wake up the Roomba by sending a low signal on the roomba[DD] pin for 500ms
	DDRD |= _BV(PD4);
	PORTD &= ~_BV(PD4);
	_delay_ms(500);

	// Put Roomba into PASSIVE MODE
	_roomba_send_command(OC_START, 0, NULL );
	_delay_ms(20);
	// Put Roomba into SAFE mode
	_roomba_send_command(OC_CONTROL, 0, NULL );
	_delay_ms(20);
}

void roomba_drive(int16_t velocity, int16_t radius) {
	uint8_t data[4];
	data[0] = (uint8_t) (velocity >> 8);
	data[1] = (uint8_t) (velocity & 0xFF);
	data[2] = (uint8_t) (radius >> 8);
	data[3] = (uint8_t) (radius & 0xFF);

	_roomba_send_command(OC_DRIVE, 4, data);
}

// Blocking call
ROOMBA_PACKET_1 roomba_sense_1() {

	// Send request
	const uint8_t data[] = { (uint8_t) ROOMBA_SENSOR_PACKET_1 };
	_roomba_send_command(OC_SENSORS, 1, data);

	// Receive into a buffer
	int packet_size = sizeof(ROOMBA_PACKET_1);
	uint8_t buffer[packet_size];

	// Busy wait until entire packet is read
	int i = 0;
	while (i < packet_size) {
		buffer[i++] = UART_Receive1();
	}

	ROOMBA_PACKET_1 packet;
	memcpy(&packet, buffer, packet_size);

	return packet;
}

#endif /* ROOMBA_H_ */
