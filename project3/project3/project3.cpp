/*
 *  project3.cpp
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

// Transmitter
#include "transmitter/packet.h"
#include "transmitter/radio.h"
#include "transmitter/cops_and_robbers.h"

// IR lib
#include "ir/ir.h"

// roomba communication lib
#include "roomba/roomba.h"
#include "roomba/roomba_sci.h"
#include "roomba/sensor_struct.h"
#include "uart/uart.h"

SERVICE* radio_service;
SERVICE* radio_service_response;

// Packet for radio receive
radiopacket_t in_packet;
radiopacket_t out_packet;

uint8_t station_addr[5] = { 0xA7, 0xA7, 0xA7, 0xA7, 0xA7 }; // Receiver address
uint8_t my_addr[5] = { 0x6b, 0x6b, 0x6b, 0x6b, 0x6b }; // Transmitter address

typedef struct {
	COPS_AND_ROBBERS id = COP1;
	uint8_t state; // true value is alive
	uint8_t sonar_value; // Add sonar value to this when its changed
	int16_t v_drive; // Forwards/backwards speed roomba
	int16_t v_turn; // Turning speed roomba
} roomba_state;

roomba_state program_state;

roomba_sensor_data_t roomba_sensor_packet;
uint8_t startGame;

IR_TEAM_CODE ENEMY_CODE = ROBBER_CODE;
IR_TEAM_CODE TEAM_CODE = COP_CODE;

/***************************
 * * * * * * * * * * * * * *
 *      RTOS Functions     *
 * * * * * * * * * * * * * *
 ***************************/

void IR_Transmit_Periodic(){
	for(;;) {
		IR_transmit(TEAM_CODE);
		Task_Next();
	}
}

void Collect_Logic_Periodic(){
	for(;;) {
		// Add collection of all sensors and set appropriate information
		//Roomba_UpdateSensorPacket(LIGHT_SENSOR, &roomba_sensor_packet);
		Roomba_UpdateSensorPacket(CHASSIS, &roomba_sensor_packet); // updates the sensors in the roombas chassis
		Roomba_UpdateSensorPacket(EXTERNAL, &roomba_sensor_packet); // updates the external sensors of the bot
		Roomba_UpdateSensorPacket(LIGHT_SENSOR, &roomba_sensor_packet); // updates the light sensors of the bot
		
		/*Roomba_Send_Byte(142);
		Roomba_Send_Byte(9);
		_delay_ms(20);
		roomba_sensor_packet->cliff_left = uart_get_byte(0);

		Roomba_Send_Byte(142);
		Roomba_Send_Byte(10);
		_delay_ms(20);
		roomba_sensor_packet->cliff_front_left = uart_get_byte(0);

		Roomba_Send_Byte(142);
		Roomba_Send_Byte(11);
		_delay_ms(20);
		roomba_sensor_packet->cliff_front_right = uart_get_byte(0);

		Roomba_Send_Byte(142);
		Roomba_Send_Byte(12);
		_delay_ms(20);
		roomba_sensor_packet->cliff_right = uart_get_byte(0);*/		
		Task_Next();
	}
}

// Telling the roomba to specifically drive
void Send_Drive_Command(){
	for(;;) {
		if((program_state.state & DEAD) > 0)
		{
			program_state.v_drive = 0; // setting speed of roomba
			program_state.v_turn = 0; // setting radius of roomba turn
		}
		else if(roomba_sensor_packet.bumps_wheeldrops & 0x1)
		{
			program_state.v_drive = 100; // setting speed of roomba
			program_state.v_turn = 1; // setting radius of roomba turn
		}
		else if (roomba_sensor_packet.bumps_wheeldrops & 0x2)
		{
			program_state.v_drive = 100; // setting speed of roomba
			program_state.v_turn = -1; // setting radius of roomba turn
		}
		else if(roomba_sensor_packet.light_bumber & 0x7)
		{
			program_state.v_drive = 100; // setting speed of roomba
			program_state.v_turn = 1; // setting radius of roomba turn
		}
		else if (roomba_sensor_packet.light_bumber & 0x38){
			program_state.v_drive = 100; // setting speed of roomba
			program_state.v_turn = -1; // setting radius of roomba turn
		}
		else{
			program_state.v_drive = 100; // setting speed of roomba
			program_state.v_turn = 0; // setting radius of roomba turn
		}
		Roomba_Drive(program_state.v_drive, program_state.v_turn);
		Task_Next();
	}
}

/***************************
 * * * * * * * * * * * * * *
 *    Wireless Functions   *
 * * * * * * * * * * * * * *
 ***************************/

// This is called when we receive a notification that a message has been received
void Wireless_Receiving(){
    RADIO_RX_STATUS radio_status;
    int16_t radio_service_value;

    for(;;){
        Service_Subscribe(radio_service, &radio_service_value);

        do {
            radio_status = Radio_Receive(&in_packet);

            if(radio_status == RADIO_RX_MORE_PACKETS || radio_status == RADIO_RX_SUCCESS) {
    			if(in_packet.type == GAMESTATE_PACKET){
    				if((in_packet.payload.gamestate.roomba_states[roomba_state.id] & FORCED) == 0) {
    					if(program_state.state & FORCED != 0) {
    						program_state.state = in_packet.payload.gamestate.roomba_states[roomba_state.id];
    					}
	    				if(in_packet.payload.gamestate.game_state == GAME_RUNNING){
	    					startGame = 1;
	    				}
	    				if(in_packet.payload.gamestate.game_state == GAME_OVER && ){
	    					Roomba_Drive(100, -1);
	    				}
	    				if(in_packet.payload.gamestate.roomba_states[roomba_state.id] != program_state.state){
	    					Service_Publish(radio_service_response, 1);
	    				}
	    			}else {
                        program_state.state = in_packet.payload.gamestate.roomba_states[roomba_state.id];
                    }
    			}
    			break;
            }
        } while(radio_status == RADIO_RX_MORE_PACKETS);
    }
}

// This function applies logic to build the parameters later used in the sendPacket function
void Wireless_Sending(){
	int16_t radio_response_service_value;
	pf_roombastate_t roomba_state_command;

	for(;;){
	    Service_Subscribe(radio_service_response, &radio_response_service_value);

        roomba_state_command.roomba_state = program_state.state;
        roomba_state_command.roomba_id = program_state.id;

        out_packet.type = ROOMBASTATE_PACKET;
        memcpy(&out_packet.payload.roombastate, &roomba_state_command, sizeof(pf_roombastate_t));

        uint8_t status = Radio_Transmit(&out_packet, RADIO_RETURN_ON_TX);
	}
}

/***************************
 * * * * * * * * * * * * * *
 *     Setup Functions     *
 * * * * * * * * * * * * * *
 ***************************/

void Wireless_Init(){
	Radio_Init();

	// configure the receive settings for radio pipe 0
	Radio_Configure_Rx(RADIO_PIPE_0, my_addr, ENABLE);
	// configure radio transceiver settings.
	Radio_Configure(RADIO_2MBPS, RADIO_HIGHEST_POWER);
	Radio_Set_Tx_Addr(station_addr);
}

/***************************
 * * * * * * * * * * * * * *
 *       Interrupts        *
 * * * * * * * * * * * * * *
 ***************************/
void ir_rxhandler() {
	uint8_t ir_value = IR_getLast();
	if (ir_value == TEAM_CODE){	
		program_state.state = 1;
		PORTB |= 1 << PB4;
		_delay_ms(500);
		PORTB ^= 1 << PB4;
	} else if (ir_value == ENEMY_CODE){
		program_state.state = 0;
	}
}

void radio_rxhandler(uint8_t pipe_number){
    Service_Publish(radio_service, pipe_number);
}

/***************************
 * * * * * * * * * * * * * *
 *      Main Function      *
 * * * * * * * * * * * * * *
 ***************************/
void setup(){
	DDRB |= 1 << PB4; // Testing IR alive

	Wireless_Init();
	IR_init();
	Roomba_Init();

	program_state.state = ALIVE; // Set bot to alive
	program_state.v_drive = 0; // Set bot to stand still
	program_state.v_turn = 0; // Set bot to stand still

	startGame = 0; // Game hasnt started yet
}

int r_main(){
	setup();

	while(!startGame){}; // Wait until game starts from interupt (implement better)

	radio_service = Service_Init();
    radio_service_response = Service_Init();

	// Add RTOS functions here
	Task_Create_Periodic(IR_Transmit_Periodic, 0, 50, 5, 46);
	Task_Create_Periodic(Collect_Logic_Periodic, 0, 50, 40, 0);
	Task_Create_Periodic(Send_Drive_Command, 0, 50, 5, 41);
	Task_Create_System(Wireless_Receiving, 0);
	Task_Create_System(Wireless_Sending, 0);

	return 0;
}

