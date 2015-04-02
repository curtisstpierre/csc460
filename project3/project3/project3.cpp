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
#include "sonar/Sonar.h"
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
SERVICE* ir_service;

// Codes of two teams in game
#define ENEMY_CODE (uint8_t)'B'
#define TEAM_CODE (uint8_t)'A'

// Flag for radio receive
int16_t rxflag;

typedef struct {
	// true value is alive
	uint8_t state;
	uint8_t sonar_value; // Add sonar value to this when its changed
	int16_t v_drive; // Forwards/backwards speed roomba
	int16_t v_turn; // Turning speed roomba
} roomba_state;

roomba_state program_state;
roomba_sensor_data_t roomba_sensor_packet;
ROOMBA_SENSOR_GROUP test = CHASSIS;
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

void Collect_Logic_RR(){
	for(;;) {
		// Add collection of all sensors and set appropriate information

		//Roomba_UpdateSensorPacket(test, &roomba_sensor_packet); // updates the sensors in the roombas chassis
		//Roomba_UpdateSensorPacket(EXTERNAL, &roomba_sensor_packet); // updates the external sensors of the bot
		//roomba_sensor_packet->distance - sensor in chasis and gives distance to an object
		//roomba_sensor_packet->wall - sensor on the external of the roomba and says if you hit a wall (there are more for angles on this)
		program_state.v_drive = 250; // setting speed of roomba
		program_state.v_turn = 0; // setting radius of roomba turn
	}
}

void Collect_Logic_Periodic(){
	// Add collection of all sensors and set appropriate information
	//Roomba_UpdateSensorPacket(CHASSIS, &roomba_sensor_packet); // updates the sensors in the roombas chassis
	//Roomba_UpdateSensorPacket(EXTERNAL, &roomba_sensor_packet); // updates the external sensors of the bot
	//roomba_sensor_packet->distance - sensor in chasis and gives distance to an object
	//roomba_sensor_packet->wall - sensor on the external of the roomba and says if you hit a wall (there are more for angles on this)
	program_state.v_drive = 250; // setting speed of roomba
	program_state.v_turn = 0; // setting radius of roomba turn
	Task_Next();
}

void Collect_Sonar_System(){
	// Add collection of all sensors and set appropriate information
	Roomba_UpdateSensorPacket(CHASSIS, &roomba_sensor_packet); // updates the sensors in the roombas chassis
	//Roomba_UpdateSensorPacket(EXTERNAL, &roomba_sensor_packet); // updates the external sensors of the bot
	//roomba_sensor_packet->distance - sensor in chasis and gives distance to an object
	//roomba_sensor_packet->wall - sensor on the external of the roomba and says if you hit a wall (there are more for angles on this)
	//SonarGetDistance();
	Task_Next();
}

// Telling the roomba to specifically drive
void Send_Drive_Command(){
	for(;;) {
		if (program_state.sonar_value > 2000)
		{
			Roomba_Drive(program_state.v_drive,-1*program_state.v_turn);
		}
		else
		{
			Roomba_Drive(100,-1);
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
void wirelessReceiving(){
	/*if (rxflag){
	    if (Radio_Receive(&packet) != RADIO_RX_MORE_PACKETS){
	      rxflag = 0;
	    }
	}*/
}

// This function applies logic to build the parameters later used in the sendPacket function
void wirelessSending(){
	/*XMap = map(potXVal, 0, 1023, -1800, 1800);
	YMap = map(potYVal, 0, 1023, -350, 350);

	if (XMap > 0){
	XMap = 1800 - XMap;
	} else {
	XMap = -1800 - XMap;
	}

	driveCommand[0] = (YMap >> 8);
	driveCommand[1] = YMap & 0xff;
	driveCommand[2] = (XMap >> 8);
	driveCommand[3] = XMap & 0xff;

	sendPacket();
	delay(100);*/
}

// Used to send the packets
void sendPacket(){
	/*packet.type = IR_COMMAND;
	memcpy(packet.payload.ir_command.sender_address, my_addr, RADIO_ADDRESS_LENGTH);
	pf_ir_command_t * ir_cmd =  &(packet.payload.ir_command);
	ir_cmd->ir_command = SEND_BYTE;
	ir_cmd->servo_angle = 0;
	ir_cmd->ir_data = IRMessage;

	if (Radio_Transmit(&packet, RADIO_WAIT_FOR_TX) == RADIO_TX_MAX_RT){
	  // Max retries reached
	}else{
	  // Submitted data succesfully
	}

	packet.type = COMMAND;
	memcpy(packet.payload.command.sender_address, my_addr, RADIO_ADDRESS_LENGTH);
	pf_command_t * cmd =  &(packet.payload.command);
	cmd->command = 137;
	cmd->num_arg_bytes = 4;
	cmd->arguments[0] = driveCommand[0];
	cmd->arguments[1] = driveCommand[1];
	cmd->arguments[2] = driveCommand[2];
	cmd->arguments[3] = driveCommand[3];

	if (Radio_Transmit(&packet, RADIO_WAIT_FOR_TX) == RADIO_TX_MAX_RT){
	  // Max retries reached
	}else{
	  // Submitted data succesfully
	} */
}

/***************************
 * * * * * * * * * * * * * *
 *     Setup Functions     *
 * * * * * * * * * * * * * *
 ***************************/

void wirelessSetup(){
	/*volatile uint8_t rxflag = 0;

	uint8_t station_addr[5] = { 0xA7, 0xA7, 0xA7, 0xA7, 0xA7 }; // Receiver address
	uint8_t my_addr[5] = { 0x6b, 0x6b, 0x6b, 0x6b, 0x6b }; // Transmitter address

	radiopacket_t packet;

	// Joystick Variables
	uint8_t driveCommand[4];

	Radio_Init();

	// configure the receive settings for radio pipe 0
	Radio_Configure_Rx(RADIO_PIPE_0, my_addr, ENABLE);
	// configure radio transceiver settings.
	Radio_Configure(RADIO_2MBPS, RADIO_HIGHEST_POWER);
	Radio_Set_Tx_Addr(station_addr);*/
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
		PORTB |= 1 << PB5;
		_delay_ms(500);
		PORTB ^= 1 << PB5;
	}
}

void radio_rxhandler(uint8_t pipe_number){
  //rxflag = 1; // Need to do a interupt publish here that calls the receive function
}
/***************************
 * * * * * * * * * * * * * *
 *      Main Function      *
 * * * * * * * * * * * * * *
 ***************************/
void setup(){
	DDRB |= 1 << PB4; // Testing IR alive
	DDRB |= 1 << PB5; // Testing IR dead

	//wirelessSetup();
	SonarInit();
	//IR_init();
	Roomba_Init();
	program_state.sonar_value = 100;
	program_state.state = 1; // Set bot to alive
	program_state.v_drive = 0; // Set bot to stand still
	program_state.v_turn = 0; // Set bot to stand still
	startGame = 1; // Game hasn't started yet
}

int r_main(){
	setup();

	while(!startGame){}; // Wait until game starts from interrupt (implement better)

	// Add RTOS functions here
	Task_Create_Periodic(IR_Transmit_Periodic, 0, 20, 1, 3);
	Task_Create_Periodic(Collect_Logic_Periodic, 0, 20, 10, 0);
	//Task_Create_RR(Collect_Logic_RR, 0);
	//Collect_Logic_Periodic();
	//Roomba_UpdateSensorPacket(CHASSIS, &roomba_sensor_packet);
	//PORTB ^= 1 << PB4;
	//_delay_ms(500);
	Task_Create_System(Collect_Sonar_System,0);
	Task_Create_Periodic(Send_Drive_Command, 0, 20, 2, 7);
	return 0;
}

