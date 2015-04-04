/*
 * cops_and_robbers.h
 *
 * Created: 2015-01-26 17:09:37
 *  Author: Daniel
 */


#ifndef COPS_AND_ROBBERS_H_
#define COPS_AND_ROBBERS_H_

#include "avr/io.h"

#define DEAD 1
#define FORCED 2

typedef enum _game_state {
  GAME_STARTING,
  GAME_RUNNING,
  GAME_OVER
} GAME_STATE;

typedef enum _cops_and_robbers {
  COP1 = 0,
  COP2,
  ROBBER1,
  ROBBER2
} COPS_AND_ROBBERS;

extern uint8_t BASE_ADDRESS[5];
extern uint8_t ROOMBA_ADDRESSES[][5];

extern uint8_t BASE_FREQUENCY;
extern uint8_t ROOMBA_FREQUENCIES[];

#endif /* COPS_AND_ROBBERS_H_ */