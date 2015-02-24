/*
 * c_buffer.h
 *
 *  Created on: May 28, 2013
 *      Author: andpol
 */

#ifndef C_BUFFER_H_
#define C_BUFFER_H_

#include <stdlib.h>

typedef struct {
	uint8_t *data;
	uint8_t start;
	uint8_t len;
} cirBuffer_t;

void init_cirBuffer(cirBuffer_t *buffer, size_t size){
	buffer->len = 0;
	buffer->start = 0;
	buffer->data = calloc(size, 1);
}

#endif /* C_BUFFER_H_ */
