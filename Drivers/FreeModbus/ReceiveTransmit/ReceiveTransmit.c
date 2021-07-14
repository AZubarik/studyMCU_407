#include "ReceiveTransmit.h"

extern uint16_t usSRegInBuf[];
extern uint16_t usSRegHoldBuf[];

uint16_t out;
uint16_t* tmpe = 0;

uint16_t RegisterValueOne,
         RegisterValueTwo = 0;
uint32_t result = 0;
float value;

float dataReceive (int registr) {
  RegisterValueOne = usSRegHoldBuf[registr];
  RegisterValueTwo = usSRegHoldBuf[registr + 1];

  result = (RegisterValueOne ) | (RegisterValueTwo << 16);

  memcpy(&value, &result, sizeof(value));

  return value;
}

void dataTransmit (int registr, float out) {
	  tmpe = (uint16_t*) &out;
    usSRegInBuf[registr] = *tmpe;
    usSRegInBuf[registr + 1] = *(tmpe + 1);
}