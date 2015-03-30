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
#include <transmitter/packet.h>
#include <transmitter/radio.h>
#include <transmitter/stdio.h>

SERVICE* service1;
SERVICE* service2;

int16_t system_value;
int16_t rr_value;
int16_t periodic_value;



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

void setup(){
	wirelessSetup();
	timer_three_setup();
	pwm_timer_setup();
}

void wirelessSetup(){
	volatile uint8_t rxflag = 0;

	uint8_t station_addr[5] = { 0xAA, 0xAA, 0xAA, 0xAA, 0xAA }; // Receiver address
	uint8_t my_addr[5] = { 0x66, 0x66, 0x66, 0x66, 0x66 }; // Transmitter address

	radiopacket_t packet;

	uint8_t IRMessage = 'A';

	// Joystick Variables
	uint8_t driveCommand[4];

	Radio_Init();

	// configure the receive settings for radio pipe 0
	Radio_Configure_Rx(RADIO_PIPE_0, my_addr, ENABLE);
	// configure radio transceiver settings.
	Radio_Configure(RADIO_2MBPS, RADIO_HIGHEST_POWER);
	Radio_Set_Tx_Addr(station_addr);
}

void timer_three_setup(){
    //Clear config
    TCCR3A = 0;
    TCCR3B = 0;

    //Set CTC 4
    TCCR3B |= (1 << WGM32); 

    //Set prescaler to 256
    TCCR3B |= (1<<CS32); 
    
    //Set value (0.25 seconds)
    OCR3A = 0x22FF; 

    //Enable interrupt
    TIMSK3 |= (1<<OCIE3A); 

    //Set timer to 0
    TCNT3=0;
}

void pwm_timer_setup(){
	// Clear timer config
	TCCR1A = 0;
	TCCR1B = 0;
	TIMSK1 &= ~(1<<OCIE1C);

	// Set to Fast PWM (mode 15)
	TCCR1A |= (1<<WGM10) | (1<<WGM11);
	TCCR1B |= (1<<WGM12) | (1<<WGM13);

	// Enable output C.
	TCCR1A |= (1<<COM1C1);

	// No prescaler
	TCCR1B |= (1<<CS10);

	OCR1A = 421;  // 38khz frequency
	OCR1C = 0;    // Target
}

/***************************
 * * * * * * * * * * * * * *
 *       Interrupts        *
 * * * * * * * * * * * * * *
 ***************************/
// Use this to check flags and publish services
ISR(TIMER3_COMPA_vect){
	// Send a HIGH value for inrared
	OCR1C = 140;
	// Send a LOW value for infrared
	OCR1C = 0;
}
/***************************
 * * * * * * * * * * * * * *
 *      Main Function      *
 * * * * * * * * * * * * * *
 ***************************/

int r_main(){
	// setup();

	// Add RTOS function here

	return 0;
}

// Lets us know a message has come in
void radio_rxhandler(uint8_t pipe_number){
  rxflag = 1; // Need to do a interupt publish here that calls the receive function
}

