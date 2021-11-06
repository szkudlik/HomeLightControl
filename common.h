#ifndef COMMON_H
#define COMMON_H

#include <Arduino.h>

#ifdef __AVR_ATmega2560__
#define CONTROLLER
#elif __AVR_ATmega328P__
#else
#error unknown board
#endif




#ifdef CONTROLLER

//#define DEBUG_1
//#define DEBUG_2
#define DEBUG_3


#define DEBUG_SERIAL Serial
#define DEBUG_SERIAL_EVENT serialEvent
#define COMM_SERIAL Serial1
#define COMM_SERIAL_EVENT serialEvent1


#include "ResponseHandler.h"

#else //ifdef CONTROLLER


#define COMM_SERIAL Serial
#define COMM_SERIAL_EVENT serialEvent

#endif //ifdef CONTROLLER




#ifdef DEBUG_1
#define DEBUG_PRINTLN_1(x) DEBUG_SERIAL.println(F(x));
#else
#define DEBUG_PRINTLN_1(x)
#endif

#ifdef DEBUG_2
#define DEBUG_PRINTLN_2(x) DEBUG_SERIAL.println(F(x));
#else
#define DEBUG_PRINTLN_2(x)
#endif

#ifdef DEBUG_3
#define DEBUG_PRINTLN_3(x) RespHandler.println(F(x));
#else
#define DEBUG_PRINTLN_3(x)
#endif









#endif
