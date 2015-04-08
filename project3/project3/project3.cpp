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
SERVICE* ir_interrupt_service;

#define LEFT_BUMPER_HIT 0x2
#define RIGHT_BUMPER_HIT 0x1
#define LEFT_LIGHT_BUMPER_HIT 0x7
#define RIGHT_LIGHT_BUMPER_HIT 0x38

// Packet for radio receive
radiopacket_t in_packet;
radiopacket_t out_packet;

uint8_t station_addr[5] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF }; // Receiver address
uint8_t my_addr[5] = {0x4A,0x4A,0x4A,0x4A,0x4A}; // Transmitter address

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

int cont_delay = 0; //used to keep a command going without sensor input
int16_u distance; // keep track of accumulated distance
int angle;
int direction = 0;
int lockon = 0;

/***************************
 * * * * * * * * * * * * * *
 *      RTOS Functions     *
 * * * * * * * * * * * * * *
 ***************************/

void IR_Transmit_Periodic(){
	for(;;) {
		if(program_state.state & DEAD || startGame == 0)
		{
			// Add something funny here
		} else{
			IR_transmit(TEAM_CODE);
		}
		Task_Next();
	}
}
/************************************************************************/
/* This function collects all sensor data from the roomba               */
/************************************************************************/
void Collect_Logic_Periodic(){
	
		for(;;) {
			if (startGame == 1){
				// Add collection of all sensors and set appropriate information
				Roomba_UpdateSensorPacket(CHASSIS, &roomba_sensor_packet); // updates the sensors in the roombas chassis
				Roomba_UpdateSensorPacket(EXTERNAL, &roomba_sensor_packet); // updates the external sensors of the bot
				Roomba_UpdateSensorPacket(LIGHT_SENSOR, &roomba_sensor_packet); // updates the light sensors of the bot
			}
			Task_Next();
		}
}
/************************************************************************/
/* Handles turning based on time distance and previous directions       */
/************************************************************************/
void Patrol(){
	// counterclockwise positive // clockwise negative
	if (direction == 1){
		program_state.v_drive = 200; // setting speed of roomba
		program_state.v_turn = 1; // setting radius of roomba turn clockwise
	}
	else if (direction == 0){
		program_state.v_drive = 200; // setting speed of roomba
		program_state.v_turn = -1; // setting radius of roomba turn counterclockwise
	}
	if(angle > 4){
		if (direction == 1){
			direction = 0;
		}
		else{
			direction = 1;
		}
		distance.value = 0;
		angle = 0;
	}
	angle++;
}

/************************************************************************/
/* Driving logic tells the rooms which commands to enact				*/
/* In it's current iteration the system will "Track" an object picked	*/
/* up by it's light sensors and attempt to aim at it, if it hits		*/
/* an object in front of it, it attempts to turn away                   */
/************************************************************************/
void Driving_Logic(){
	if(program_state.state & DEAD)
	{
		program_state.v_drive = 0; // setting speed of roomba
		program_state.v_turn = 0; // setting radius of roomba turn
	} else {
		//if the right bumper is hit turn left
		if(roomba_sensor_packet.bumps_wheeldrops & RIGHT_BUMPER_HIT)
		{
			program_state.v_drive = 100; // setting speed of roomba
			program_state.v_turn = 1; // setting radius of roomba turn
			cont_delay = 5;
			distance.value = 0;
		}
		//if the left bumper is hit turn right
		else if (roomba_sensor_packet.bumps_wheeldrops & LEFT_BUMPER_HIT){
			program_state.v_drive = 100; // setting speed of roomba
			program_state.v_turn = -1; // setting radius of roomba turn
			cont_delay = 5;
			distance.value = 0;
		}
		else if (lockon > 10){
			program_state.v_drive = 200; // setting speed of roomba
			program_state.v_turn = -1; // setting radius of roomba turn
			if (lockon == 11){
				lockon = 17;
			}
			if (lockon == 12){
				lockon=1;
			}
			lockon--;			
		}
		//if the left light bumper(s) sense an object turn left
		else if(roomba_sensor_packet.light_bumber & LEFT_LIGHT_BUMPER_HIT)
		{
			lockon += 1;
			program_state.v_drive = 100; // setting speed of roomba
			program_state.v_turn = 1; // setting radius of roomba turn
		}
		//if the right light bumper(s) sense an object turn right
		else if (roomba_sensor_packet.light_bumber & RIGHT_LIGHT_BUMPER_HIT){
			lockon += 1;
			program_state.v_drive = 100; // setting speed of roomba
			program_state.v_turn = -1; // setting radius of roomba turn
		}
		//straight ahead till dawn
		else{
			if (cont_delay == 0){
				if (distance.value > 1500){
					Patrol();
				}
				else{
					program_state.v_drive = 300; // setting speed of roomba
					program_state.v_turn = 0; // setting radius of roomba turn
				}
			}
			else{
				cont_delay--;
			}
			distance.value += program_state.v_drive / 4;
		}
	}
}

// Telling the roomba to specifically drive
void Send_Drive_Command(){
		for(;;) {
			if(startGame == 1){
				Driving_Logic();
				Roomba_Drive(program_state.v_drive, program_state.v_turn);
			}
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
    				if((in_packet.payload.gamestate.roomba_states[program_state.id] & FORCED) == 0) {
    					if((program_state.state & FORCED) != 0) {
    						program_state.state = in_packet.payload.gamestate.roomba_states[program_state.id];
    					}
	    				if(in_packet.payload.gamestate.game_state == GAME_RUNNING){
	    					startGame = 1;
	    				}
	    				if(in_packet.payload.gamestate.game_state == GAME_OVER && (program_state.state & DEAD) == 0){
							program_state.v_drive = 100;
							program_state.v_turn = -1;
							Roomba_Drive(program_state.v_drive, program_state.v_turn);
	    				}
	    				if(in_packet.payload.gamestate.roomba_states[program_state.id] != program_state.state){
	    					Service_Publish(radio_service_response, 1);
	    				}
	    			}else {
                        program_state.state = in_packet.payload.gamestate.roomba_states[program_state.id];
						if(in_packet.payload.gamestate.game_state == GAME_RUNNING){
							startGame = 1;
						}
						if(in_packet.payload.gamestate.game_state == GAME_OVER && (program_state.state & DEAD) == 0){
							startGame = 0;
							program_state.v_drive = 100;
							program_state.v_turn = -1;
							Roomba_Drive(program_state.v_drive, program_state.v_turn);
						}
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
	// RADIO INITIALIZATION
	DDRL |= (1 << PL2);
	PORTL &= ~(1 << PL2);
	_delay_ms(500);  /* max is 262.14 ms / F_CPU in MHz */
	PORTL |= 1 << PL2;
	_delay_ms(500);
	
	Radio_Init();

	// configure the receive settings for radio pipe 0
	Radio_Configure_Rx(RADIO_PIPE_0, my_addr, ENABLE);
	// configure radio transceiver settings.
	Radio_Configure(RADIO_1MBPS, RADIO_HIGHEST_POWER);
	Radio_Set_Tx_Addr(station_addr);
}

/************************************************************************/
/* If shot received from enemy die (or stay dead)                       */
/* If shot received by ally then revive (or stay alive)					*/
/************************************************************************/
void ir_shots_recieved(){
	int16_t radio_response_service_value;
	for(;;){
		Service_Subscribe(ir_interrupt_service, &radio_response_service_value);
		uint8_t ir_value = IR_getLast();
		if (ir_value == TEAM_CODE){
			program_state.state &= ~DEAD;
		} else if (ir_value == ENEMY_CODE){
			program_state.state |= DEAD;
		}
	}
	
}

/***************************
 * * * * * * * * * * * * * *
 *       Interrupts        *
 * * * * * * * * * * * * * *
 ***************************/

/************************************************************************/
/* Handler for recieved IR shots                                     */
/************************************************************************/
void ir_rxhandler() {
	Service_Publish(ir_interrupt_service, 0);
}

/************************************************************************/
/* Handler for wireless information being received                      */
/************************************************************************/
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

	IR_init();
	Roomba_Init();

	program_state.state &= ~DEAD; // Set bot to alive
	program_state.v_drive = 0; // Set bot to stand still
	program_state.v_turn = 0; // Set bot to stand still

	distance.value = 0;
	angle= 0;
	startGame = 0; // Game hasn't started yet
}

int r_main(){
	setup();
	radio_service = Service_Init();
	radio_service_response = Service_Init();
	ir_interrupt_service = Service_Init();
	Task_Create_System(ir_shots_recieved, 0);
	Task_Create_System(Wireless_Receiving, 0);
	Task_Create_System(Wireless_Sending, 0);
	Task_Create_Periodic(IR_Transmit_Periodic, 0, 50, 5, 41);
	//Task_Create_RR(IR_Transmit_Periodic, 0);
	Task_Create_Periodic(Collect_Logic_Periodic, 0, 50, 40, 0);
	Task_Create_Periodic(Send_Drive_Command, 0, 50, 5, 46);
	//Task_Create_RR(Send_Drive_Command, 0);

	Wireless_Init();
	return 0;
}

