/*
 * test.c
 *
 *  Created on: 4-Feb-2009
 *      Author: nrqm
 */

#include "roomba.h"
#include "roomba_sci.h"
#include "../uart/uart.h"
#include <util/delay.h>
//#include "awesomedelay.h"
#include "avr/interrupt.h"
#include "../transmitter/radio.h"
#include "../transmitter/cops_and_robbers.h"
#include "../transmitter/ir.h"

//#define     clock8MHz()    CLKPR = _BV(CLKPCE); CLKPR = 0x00;
#define		TEST_UNIT 10
#define		ROOMBA_NUM COP1

volatile uint8_t rxflag = 0;

radiopacket_t packet;

void roomba_Drive( int16_t velocity, int16_t radius )
{
	Roomba_Send_Byte(DRIVE);
	Roomba_Send_Byte(HIGH_BYTE(velocity));
	Roomba_Send_Byte(LOW_BYTE(velocity));
	Roomba_Send_Byte(HIGH_BYTE(radius));
	Roomba_Send_Byte(LOW_BYTE(radius));
}

void roomba_Drive_Encode( int16_t velocity, int16_t radius, radiopacket_t *tx_packet )
{
	tx_packet->type = COMMAND;
	tx_packet->payload.command.command = DRIVE;
	tx_packet->payload.command.arguments[0] = HIGH_BYTE(velocity);
	tx_packet->payload.command.arguments[1] = LOW_BYTE(velocity);
	tx_packet->payload.command.arguments[2] = HIGH_BYTE(radius);
	tx_packet->payload.command.arguments[3] = LOW_BYTE(radius);
	tx_packet->payload.command.num_arg_bytes = 4;
}

void testCommands() {
	int16_t speed = 0;
	while(speed < 100) {
		speed += 1;
		roomba_Drive(speed, 0x8000);
		_delay_ms(100);
	}
	_delay_ms(1000);
	while(speed > -100) {
		speed -= 1;
		roomba_Drive(speed, 0x8000);
		_delay_ms(100);
		PORTB ^= (1<<PB7);
	}
	_delay_ms(1000);
}

int main()
{
	uint8_t i;

	cli();
	
	DDRD = 0xFF;
	PORTD = 0x00;
	
	
	if(ROOMBA_NUM != TEST_UNIT) {
		_delay_ms(500);
		PORTD |= 1<<PD1;
		DDRC = 0xFF;
		PORTC |= _BV(PC2) |  _BV(PC3) |_BV(PC4) |_BV(PC5) ;
	} else {
		DDRC = 0;
		PORTC |= (1<<PC1);
		//SW
		PORTC |= (1<<PC7);
	}
	
	DDRL |= (1 << PL2);
	PORTL &= ~(1<<PL2);
	_delay_ms(500);
	//PORTC ^= (1<<PC2);
	PORTL |= (1<<PL2);
	_delay_ms(500);
	//PORTC ^= (1<<PC2);
	
	Roomba_Init();
	IR_init();
	
	_delay_ms(2000);
	//PORTC = 0x00;
	Radio_Init();
	
	//IR_transmit('A');

	Radio_Configure_Rx(RADIO_PIPE_0, ROOMBA_ADDRESSES[ROOMBA_NUM], ENABLE);
	Radio_Configure(RADIO_2MBPS, RADIO_HIGHEST_POWER);

	sei();

	if(ROOMBA_NUM == TEST_UNIT) {
		DDRB |= (1<<PB7);
		uint8_t tempAddress[5] = {0xAB,0xAB,0xAB,0xAB,0xAB};
		//Radio_Set_Tx_Addr(tempAddress);
		Radio_Set_Tx_Addr(ROOMBA_ADDRESSES[COP1]);
		// UART test - drive straight forward at 100 mm/s for 1 second
		int16_t speed = 0;
		int16_t angle = 0x8000;
		for(;;) {
			radiopacket_t tx_packet1;
			tx_packet1.type = COMMAND;
			for(int i = 0; i < 5; i++) {
				tx_packet1.payload.command.sender_address[i] = ROOMBA_ADDRESSES[ROOMBA_NUM][i];
			}			
			
			tx_packet1.type = COMMAND;
			for(int i = 0; i < 5; i++) {
				tx_packet1.payload.command.sender_address[i] = ROOMBA_ADDRESSES[ROOMBA_NUM][i];
			}
			while(speed < 100) {
				speed += 1;
				roomba_Drive_Encode(speed, 0x8000, &tx_packet1);
				Radio_Transmit(&tx_packet1, RADIO_WAIT_FOR_TX);
				_delay_ms(100);			
				PORTB ^= (1<<PB7);	
			}
			_delay_ms(1000);
			while(speed > -100) {
				speed -= 1;
				roomba_Drive_Encode(speed, 0x8000, &tx_packet1);
				Radio_Transmit(&tx_packet1, RADIO_WAIT_FOR_TX);
				_delay_ms(100);
				PORTB ^= (1<<PB7);
 			}
			_delay_ms(1000);
			tx_packet1.type = IR_COMMAND;
			tx_packet1.payload.ir_command.ir_command = SEND_BYTE;
			tx_packet1.payload.ir_command.ir_data = 'A';
			Radio_Transmit(&tx_packet1, RADIO_WAIT_FOR_TX);
			_delay_ms(1000);
			Radio_Transmit(&tx_packet1, RADIO_WAIT_FOR_TX);
			_delay_ms(1000);
			Radio_Transmit(&tx_packet1, RADIO_WAIT_FOR_TX);
			_delay_ms(1000);
			Radio_Transmit(&tx_packet1, RADIO_WAIT_FOR_TX);
			_delay_ms(1000);
		}
		roomba_Drive(0, 0);

		for (;;);
	}
	
	IR_transmit('A');	
	for (;;)
	{
		//for (;;) {
		//	testCommands();
		//}
		if (rxflag)
		{
			PORTC |= _BV(PC3);
			// Copy the received packet into the radio packet structure.  If there are no more packets,
			// then clear the rxflag so that the interrupt will set it next time a packet is received.
			RADIO_RX_STATUS result = Radio_Receive(&packet);
			if(result != RADIO_RX_MORE_PACKETS) {
				rxflag = 0;
				PORTC &= ~(1<<PC3);
			}
			if (result != RADIO_RX_SUCCESS && result != RADIO_RX_MORE_PACKETS)
			{
				rxflag = 0;
				continue;
// 				for(;;) {
// 					PORTC ^= (1<<PC3);
// 					_delay_ms(500);
// 				}
// 				for(;;);
			}

			// If the packet is not a command, blink an error and don't do anything.
			if (packet.type != COMMAND)
			{
				PORTC ^= _BV(PC4);
				if(packet.type == IR_COMMAND) {
					if(packet.payload.ir_command.ir_command == SEND_BYTE) {
						PORTC ^= _BV(PC4);
						IR_transmit(packet.payload.ir_command.ir_data);
					}
					radiopacket_t tx_packet;
					tx_packet.type = IR_DATA;
					Radio_Set_Tx_Addr(packet.payload.command.sender_address);
					packet.payload.ir_data.roomba_number = ROOMBA_NUM;
					packet.payload.ir_data.ir_data = 0;
					Radio_Transmit(&tx_packet, RADIO_WAIT_FOR_TX);										
				}
				continue;
			}

			if (packet.payload.command.command == START ||
					packet.payload.command.command == BAUD ||
					packet.payload.command.command == SAFE ||
					packet.payload.command.command == FULL ||
					packet.payload.command.command == SENSORS)
			{
				// Don't pass the listed commands to the Roomba.
				continue;
			}

			// Output the command to the Roomba, followed by its arguments.
			Roomba_Send_Byte(packet.payload.command.command);
			for (i = 0; i < packet.payload.command.num_arg_bytes; i++)
			{
				Roomba_Send_Byte(packet.payload.command.arguments[i]);
			}

			// Set the radio's destination address to be the remote station's address
			Radio_Set_Tx_Addr(packet.payload.command.sender_address);

			// Update the Roomba sensors into the packet structure that will be transmitted.
			//Roomba_UpdateSensorPacket(1, &packet.payload.sensors.sensors);
			//Roomba_UpdateSensorPacket(2, &packet.payload.sensors.sensors);
			//Roomba_UpdateSensorPacket(3, &packet.payload.sensors.sensors);

			// send the sensor packet back to the remote station.
			packet.type = SENSOR_DATA;
			
			PORTC |= _BV(PC4);
			if (Radio_Transmit(&packet, RADIO_RETURN_ON_TX) == RADIO_TX_MAX_RT)
			{
				PORTC |= _BV(PC4);	// flash red if the packet was dropped
			}
			else
			{
				PORTC &= ~_BV(PC4);
				PORTC ^= _BV(PC5);	// flash green if the packet was received correctly
			}
		}
	}

	return 0;
}

void radio_rxhandler(uint8_t pipenumber)
{
	rxflag = 1;
	
	while(ROOMBA_NUM == TEST_UNIT && rxflag) {
		if (Radio_Receive(&packet) != RADIO_RX_MORE_PACKETS)
		{
			rxflag = 0;
		}
		
		if(packet.type == IR_DATA) {
			
		}
	}
}
