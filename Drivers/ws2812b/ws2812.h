#ifndef WS2812_H_
#define WS2812_H_
//--------------------------------------------------
#include "stm32f4xx_hal.h"
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
//--------------------------------------------------
#define DELAY_LEN 48
#define LED_COUNT 144
#define ARRAY_LEN DELAY_LEN + LED_COUNT*24

#define HIGH 65
#define LOW 26
//--------------------------------------------------

#endif /* WS2812_H_ */