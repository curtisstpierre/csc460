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

// IR lib
#include "ir/ir.h"

// roomba communication lib
#include "roomba/roomba.h"
#include "roomba/roomba_sci.h"
#include "roomba/sensor_struct.h"

SERVICE* radio_service;
SERVICE* radio_service_response;

// Codes of two teams in game
#define ENEMY_CODE (uint8_t)'B'
#define TEAM_CODE (uint8_t)'A'

// Packet for radio receive
radiopacket_t packet;

uint8_t station_addr[5] = { 0xA7, 0xA7, 0xA7, 0xA7, 0xA7 }; // Receiver address
uint8_t my_addr[5] = { 0x6b, 0x6b, 0x6b, 0x6b, 0x6b }; // Transmitter address

typedef struct {
	uint8_t id;
	uint8_t state; // true value is alive
	uint8_t sonar_value; // Add sonar value to this when its changed
	int16_t v_drive; // Forwards/backwards speed roomba
	int16_t v_turn; // Turning speed roomba
} roomba_state;

roomba_state program_state;
roomba_sensor_data_t roomba_sensor_packet;
uint8_t startGame;

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

		//Roomba_UpdateSensorPacket(CHASSIS, &roomba_sensor_packet); - updates the sensors in the roombas chassis
		//Roomba_UpdateSensorPacket(EXTERNAL, &roomba_sensor_packet); - updates the external sensors of the bot
		//roomba_sensor_packet->distance - sensor in chasis and gives distance to an object
		//roomba_sensor_packet->wall - sensor on the external of the roomba and says if you hit a wall (there are more for angles on this)
		program_state.v_drive = 250; // setting speed of roomba
		program_state.v_turn = 0; // setting radius of roomba turn
		Task_Next();
	}
}

// Telling the roomba to specifically drive
void Send_Drive_Command(){
	for(;;) {
		Roomba_Drive(program_state.v_drive,-1*program_state.v_turn);
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
            radio_status = Radio_Receive(&packet);

            if(radio_status == RADIO_RX_MORE_PACKETS || radio_status == RADIO_RX_SUCCESS) {
    			if(packet.type == GAMESTATE_PACKET){
    				if(packet.payload.message.game_state == GAME_STARTNG){
    					startGame = 1;
    				}
    				if(packet.payload.message.game_state == GAME_OVER){
    					// Do Something
    				}
    				if(packet.payload.message.roomba_states[program_state.id] == 0 && program_state.state == 1){
    					Service_Publish(radio_service_response, 1);
    				}
    				if(packet.payload.message.roomba_states[program_state.id] == 1 && program_state.state == 0){
    					Service_Publish(radio_service_response, 0);
    				}
    			}
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

        packet.type = ROOMBASTATE_PACKET;
        memcpy(&packet.payload.message, &roomba_state_command, sizeof(pf_roombastate_t));

        uint8_t status = Radio_Transmit(&packet, RADIO_RETURN_ON_TX);
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
	//Wireless_Init();
	IR_init();
	Roomba_Init();

	program_state.state = 1; // Set bot to alive
	program_state.v_drive = 0; // Set bot to stand still
	program_state.v_turn = 0; // Set bot to stand still

	startGame = 1; // Game hasnt started yet
}

int r_main(){
	setup();

	while(!startGame){}; // Wait until game starts from interupt (implement better)

	//radio_service = Service_Init();
    //radio_service_response = Service_Init();

	// Add RTOS functions here
	Task_Create_Periodic(IR_Transmit_Periodic, 0, 10, 3, 3);
	Task_Create_Periodic(Collect_Logic_Periodic, 0, 10, 3, 0);
	Task_Create_Periodic(Send_Drive_Command, 0, 10, 3, 7);
	//Task_Create_RR(Wireless_Receiving, 0);
	//Task_Create_System(Wireless_Sending, 0);

	return 0;
}

