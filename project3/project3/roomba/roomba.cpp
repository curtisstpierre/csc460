/*
 * roomba.c
 *
 *  Created on: 4-Feb-2009
 *      Author: nrqm
 */

#include <util/delay.h>
#include "../uart/uart.h"
#include "roomba.h"
#include "roomba_sci.h"

void Roomba_Init()
{
	// At 8 MHz, the AT90 generates a 57600 bps signal with a framing error rate of over 2%, which means that more than
	// 1 out of every 50 bits is wrong.  The fastest bitrate with a low error rate that the Roomba supports is
	// 38400 bps (0.2% error rate, or 1 bit out of every 500).

	// Try 57.6 kbps to start (this is the Roomba's default baud rate after the battery is installed).
	Roomba_UART_Init(UART_115200);

	// Try to start the SCI
	Roomba_Send_Byte(START);
	_delay_ms(20);

	// change the baud rate to 38400 bps.  Have to wait for 100 ms after changing the baud rate.
	Roomba_Send_Byte(BAUD);
	Roomba_Send_Byte(ROOMBA_19200BPS);
	_delay_ms(100);		// this delay will not work on old versions of WinAVR (new versions will see reduced but
						// still acceptable resolution; see _delay_ms definition)

	// change the AT90's UART clock
	Roomba_UART_Init(UART_19200);

	// start the SCI again in case the first start didn't go through.
	Roomba_Send_Byte(START);
	_delay_ms(20);

	// finally put the Roomba into safe mode.
	Roomba_Send_Byte(SAFE);
	_delay_ms(20);
}

void Roomba_Finish() {
	Roomba_Send_Byte(STOP);
}

void Roomba_UpdateSensorPacket(ROOMBA_SENSOR_GROUP group, roomba_sensor_data_t* sensor_packet)
{
	Roomba_Send_Byte(SENSORS);
	_delay_ms(20);
	Roomba_Send_Byte(group);
	switch(group)
	{
	case EXTERNAL:
		// environment sensors
		while (uart_bytes_received() != 10);
		sensor_packet->bumps_wheeldrops = uart_get_byte(0);
		sensor_packet->wall = uart_get_byte(1);
		sensor_packet->cliff_left = uart_get_byte(2);
		sensor_packet->cliff_front_left = uart_get_byte(3);
		sensor_packet->cliff_front_right = uart_get_byte(4);
		sensor_packet->cliff_right = uart_get_byte(5);
		sensor_packet->virtual_wall = uart_get_byte(6);
		sensor_packet->motor_overcurrents = uart_get_byte(7);
		sensor_packet->dirt_left = uart_get_byte(8);
		sensor_packet->dirt_right = uart_get_byte(9);
		break;
	case CHASSIS:
		// chassis sensors	
		while (uart_bytes_received() != 6);
		sensor_packet->remote_opcode = uart_get_byte(0);
		sensor_packet->buttons = uart_get_byte(1);
		sensor_packet->distance.bytes.high_byte = uart_get_byte(2);
		sensor_packet->distance.bytes.low_byte = uart_get_byte(3);
		sensor_packet->angle.bytes.high_byte = uart_get_byte(4);
		sensor_packet->angle.bytes.low_byte = uart_get_byte(5);
		break;
	case INTERNAL:
		// internal sensors
		while (uart_bytes_received() != 10);
		sensor_packet->charging_state = uart_get_byte(0);
		sensor_packet->voltage.bytes.high_byte = uart_get_byte(1);
		sensor_packet->voltage.bytes.low_byte = uart_get_byte(2);
		sensor_packet->current.bytes.high_byte = uart_get_byte(3);
		sensor_packet->current.bytes.low_byte = uart_get_byte(4);
		sensor_packet->temperature = uart_get_byte(5);
		sensor_packet->charge.bytes.high_byte = uart_get_byte(6);
		sensor_packet->charge.bytes.low_byte = uart_get_byte(7);
		sensor_packet->capacity.bytes.high_byte = uart_get_byte(8);
		sensor_packet->capacity.bytes.low_byte = uart_get_byte(9);
		break;
	case LIGHT_SENSOR:
		// light sensors sensors
		while (uart_bytes_received() != 28);
		sensor_packet->left_encoder_counts.bytes.high_byte = uart_get_byte(0);
		sensor_packet->left_encoder_counts.bytes.low_byte = uart_get_byte(1);
		sensor_packet->right_encoder_counts.bytes.high_byte = uart_get_byte(2);
		sensor_packet->right_encoder_counts.bytes.low_byte = uart_get_byte(3);
		sensor_packet->light_bumber = uart_get_byte(4);
		sensor_packet->left_light_bumber_signal.bytes.high_byte = uart_get_byte(5);
		sensor_packet->left_light_bumber_signal.bytes.low_byte = uart_get_byte(6);
		sensor_packet->left_front_light_bumber_signal.bytes.high_byte = uart_get_byte(7);
		sensor_packet->left_front_light_bumber_signal.bytes.low_byte = uart_get_byte(8);
		sensor_packet->left_center_light_bumber_signal.bytes.high_byte = uart_get_byte(9);
		sensor_packet->left_center_light_bumber_signal.bytes.low_byte = uart_get_byte(10);
		sensor_packet->right_center_light_bumber_signal.bytes.high_byte = uart_get_byte(11);
		sensor_packet->right_center_light_bumber_signal.bytes.low_byte = uart_get_byte(12);
		sensor_packet->right_front_light_bumber_signal.bytes.high_byte = uart_get_byte(13);
		sensor_packet->right_front_light_bumber_signal.bytes.low_byte = uart_get_byte(14);
		sensor_packet->right_light_bumber_signal.bytes.high_byte = uart_get_byte(15);
		sensor_packet->right_light_bumber_signal.bytes.low_byte = uart_get_byte(16);
		sensor_packet->left_motor_current.bytes.high_byte = uart_get_byte(17);
		sensor_packet->left_motor_current.bytes.low_byte = uart_get_byte(18);
		sensor_packet->right_motor_current.bytes.high_byte = uart_get_byte(19);
		sensor_packet->right_motor_current.bytes.low_byte = uart_get_byte(20);
		sensor_packet->main_brush_motor_current.bytes.high_byte = uart_get_byte(21);
		sensor_packet->main_brush_motor_current.bytes.low_byte = uart_get_byte(22);
		sensor_packet->side_brush_motor_current.bytes.high_byte = uart_get_byte(23);
		sensor_packet->side_brush_motor_current.bytes.low_byte = uart_get_byte(24);
		break;
	}
	uart_reset_receive();
}

void Roomba_Drive( int16_t velocity, int16_t radius )
{
	Roomba_Send_Byte(DRIVE);
	Roomba_Send_Byte(HIGH_BYTE(velocity));
	Roomba_Send_Byte(LOW_BYTE(velocity));
	Roomba_Send_Byte(HIGH_BYTE(radius));
	Roomba_Send_Byte(LOW_BYTE(radius));
}

void Roomba_Play()
{
	int16_t song = 1;
	Roomba_Send_Byte(PLAY);
	Roomba_Send_Byte(HIGH_BYTE(song));
	Roomba_Send_Byte(LOW_BYTE(song));
}
