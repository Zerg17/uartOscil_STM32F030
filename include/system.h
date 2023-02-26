#pragma once

#include "stm32f0xx.h"

#define UART1_BAUD 500000

extern uint16_t adcR[256];

void sysInit();
void uart1Write(uint8_t d);