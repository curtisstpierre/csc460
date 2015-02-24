/*
 * BlockingUART.h
 *
 *  Created on: Jun 23, 2013
 *      Author: andpol
 */

#ifndef BLOCKINGUART_H_
#define BLOCKINGUART_H_

#include <avr/io.h>
#include <stdio.h>

#define TX_BUFFER_SIZE 20

typedef enum {
	UART_CHANNEL_0, UART_CHANNEL_1,
} UART_CHANNEL;

#define MYBRR(baud_rate) (F_CPU / 16 / (baud_rate) - 1)
void UART_Init0(uint32_t baud_rate) {
	// Set baud rate
	UBRR0 = MYBRR(baud_rate);
	// Enable receiver and transmitter
	UCSR0B = _BV(TXEN0) | _BV(RXEN0);
	// Default frame format: 8 data, 1 stop bit , no parity
}

void UART_Transmit0(unsigned char data) {
	// Busy wait for empty transmit buffer
	while (!(UCSR0A & _BV(UDRE0)))
		;
	// Put data into buffer, sends the data
	UDR0 = data;
}

void UART_Init1(uint32_t baud_rate) {
	// Set baud rate
	UBRR1 = MYBRR(baud_rate);
	// Enable receiver and transmitter
	UCSR1B = _BV(TXEN1) | _BV(RXEN1);
	// Default frame format: 8 data, 1 stop bit , no parity
}

void UART_Transmit1(unsigned char data) {
	// Busy wait for empty transmit buffer
	while (!(UCSR1A & _BV(UDRE1)))
		;
	// Put data into buffer, sends the data
	UDR1 = data;
}

unsigned char UART_Receive0() {
	// Busy wait for data to be received
	while (!(UCSR0A & _BV(RXC0)))
		;
	// Get and return received data from buffer
	return UDR0 ;
}

unsigned char UART_Receive1() {
	// Busy wait for data to be received
	while (!(UCSR1A & _BV(RXC1)))
		;
	// Get and return received data from buffer
	return UDR1 ;
}

void (*get_send_funct(UART_CHANNEL ch))(unsigned char) {
	switch (ch) {
		case UART_CHANNEL_0:
			return &UART_Transmit0;
		case UART_CHANNEL_1:
			return &UART_Transmit1;
		default:
			return NULL ;
	}
}

void UART_print(const UART_CHANNEL ch, const char* fmt, ...) {
	char buffer[TX_BUFFER_SIZE];
	va_list args;
	size_t size;

	va_start(args, fmt);
	size = vsnprintf(buffer, sizeof(buffer) - 1, fmt, args);
	va_end(args);

	// Error case: do not output to UART
	if (size < 0) {
		return;
	}
	void (*send_funct)(unsigned char) = get_send_funct(ch);

	uint8_t i = 0;
	while (buffer[i] != '\0') {
		(*send_funct)(buffer[i++]);
	}
}

void UART_send_raw_bytes(const UART_CHANNEL ch, const uint8_t num_bytes, const uint8_t* data) {
	void (*send_funct)(unsigned char) = get_send_funct(ch);
	uint8_t i;
	for (i = 0; i < num_bytes; i++) {
		(*send_funct)(data[i]);
	}
}

#endif /* BLOCKINGUART_H_ */
