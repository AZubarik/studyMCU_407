#ifndef __ReceiveTransmit_H
#define __ReceiveTransmit_H
#ifdef __cplusplus
extern "C" {
#endif

// #include "main.h"
#include <string.h>

#include "mb.h"
#include "mbport.h"
#include "user_mb_app.h"

float dataReceive (int registr);
void dataTransmit (int registr, float out);

#ifdef __cplusplus
}
#endif
#endif /*__ ReceiveTransmit_H */