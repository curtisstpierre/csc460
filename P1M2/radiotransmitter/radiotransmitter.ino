/*
 * http://www.irobot.com/images/consumer/hacker/Roomba_SCI_Spec_Manual.pdf
 * PDF of messages to send to the Roomba to make it move (page 4)
 *
 * Based on code provided by Neil MacMillan in an Arduino plataform.
 * Written and Modified by Curtis St. Pierre and Mark Roller for csc 460
 *
 * IMPORTANT:
 *   Make sure you have the correct CE_PIN and CSN_PIN definitions in radio.cpp and
 *   also MISO, MOSI, SCK and SS pins in spi.cpp
 *
 *   Copy nRF24L01.h, packet.h, radio.cpp, radio.h, spi.cpp and spi.h in the same
 *   folder of your sketch.
 */

#include "packet.h"
#include "radio.h"
#include "stdio.h"

volatile uint8_t rxflag = 0;

//uint8_t station_addr[5] = { 0xAB, 0xAB, 0xAB, 0xAB, 0xAB }; // Receiver address
uint8_t station_addr[5] = { 0xAA, 0xAA, 0xAA, 0xAA, 0xAA };
uint8_t my_addr[5] = { 0x66, 0x66, 0x66, 0x66, 0x66 }; // Transmitter address

radiopacket_t packet;
char output[128];

// Pin assignments
const int buttonPin = 6;
const int potpinx = 0;
const int potpiny = 1;

// Button trigger variables
int buttonState = 0; 
int messageSend = 0;
int buttonReset = 0;
uint8_t IRMessage = 'A';

// Joystick Variables
uint8_t driveCommand[4];
int potXVal = -1;
int potYVal = -1;
int16_t YMap;
int16_t XMap;


void setup()
{
  Serial.begin(9600);
  // Set button Pin
  pinMode(buttonPin, INPUT);
  pinMode(47, OUTPUT);
  digitalWrite(47, LOW);
  delay(100);
  digitalWrite(47, HIGH);
  Radio_Init();

  // configure the receive settings for radio pipe 0
  Radio_Configure_Rx(RADIO_PIPE_0, my_addr, ENABLE);
  // configure radio transceiver settings.
  Radio_Configure(RADIO_2MBPS, RADIO_HIGHEST_POWER);
  Radio_Set_Tx_Addr(station_addr);
  
  delay(100);
}

void loop()
{
  if (rxflag)
  {
    if (Radio_Receive(&packet) != RADIO_RX_MORE_PACKETS) // Receive packet.
    {
      rxflag = 0;
    }
    if (packet.type == MESSAGE)
    {
      snprintf(output, 128, "from 0x%.2X%.2X%.2X%.2X%.2X: '%s'\n\r",
                packet.payload.message.address[0], packet.payload.message.address[1],
	              packet.payload.message.address[2],
		            packet.payload.message.address[3],
		            packet.payload.message.address[4],
		            packet.payload.message.messagecontent);
      Serial.print(output);
    }
  }
  
  buttonState = digitalRead(buttonPin);
    
  // Logic for handling button press and release
  if(buttonState == LOW && buttonReset == 0){
    buttonReset = 1;
    messageSend = 1;
  } else if(buttonState == HIGH && buttonReset == 1){
    buttonReset = 0;
  }
    
  if (potXVal != analogRead(potpinx) || potYVal != analogRead(potpiny) || (potXVal == -1 && potYVal == -1)){
    potXVal = analogRead(potpinx);
    potYVal = analogRead(potpiny);
      
    if (potXVal < 550 && potXVal > 480 && potYVal < 550 && potYVal > 480){
      // do nothing
      driveCommand[0] = 0;
      driveCommand[1] = 0;
      driveCommand[2] = 0;
      driveCommand[3] = 0;
        
    } else if (potYVal < 550 && potYVal > 480){
      // spin in place
      YMap = map(potXVal, 0, 1023, -350, 350);
      
      if (potXVal < 512){
        YMap = YMap*(-1);
        driveCommand[0] = (YMap >> 8);
        driveCommand[1] = YMap & 0xff;
        driveCommand[2] = 255;
        driveCommand[3] = 255;
      } else{
        driveCommand[0] = (YMap >> 8);
        driveCommand[1] = YMap & 0xff;
        driveCommand[2] = 0;
        driveCommand[3] = 1;
      } 
    } else if (potXVal < 550 && potXVal > 480){   
      // straight forward and back
      XMap = 32768;
      driveCommand[2] = (XMap >> 8);
      driveCommand[3] = XMap & 0xff;
      YMap = map(potYVal, 0, 1023, -350, 350);
      driveCommand[0] = (YMap >> 8);
      driveCommand[1] = YMap & 0xff;
      
    } else {
      // mix it up a little
      XMap = map(potXVal, 0, 1023, -1800, 1800);
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
    }
  }

  sendPacket();
  messageSend = 0;
  delay(100);
}

void sendPacket(){
  if (messageSend == 1){    
    // load up the packet contents
    packet.type = IR_COMMAND;
    memcpy(packet.payload.ir_command.sender_address, my_addr, RADIO_ADDRESS_LENGTH);
    pf_ir_command_t * ir_cmd =  &(packet.payload.ir_command);
    ir_cmd->ir_command = SEND_BYTE;
    ir_cmd->servo_angle = 0;
    ir_cmd->ir_data = IRMessage;
    
    if (Radio_Transmit(&packet, RADIO_WAIT_FOR_TX) == RADIO_TX_MAX_RT){
      Serial.println("Data not trasmitted. Max retry.");
    }else{
      Serial.println("Data transmitted.");
    }

  } else { 
    // load up the packet contents
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
      Serial.println("Data not trasmitted. Max retry.");
    }else{
      Serial.println("Data transmitted.");
    }
  }
}

// The radio_rxhandler is called by the radio IRQ pin interrupt routine when RX_DR is read in STATUS register.
void radio_rxhandler(uint8_t pipe_number)
{
  rxflag = 1;
}
