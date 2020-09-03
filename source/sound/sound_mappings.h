#ifndef __MAPPINGS_H
#define __MAPPINGS_H

#include "stm8l15x.h"

//UART, RX - PC6, TX - PC5
#define RX_port				GPIOC
#define RX_pin				GPIO_Pin_6
#define TX_port				GPIOC
#define TX_pin				GPIO_Pin_5

//chipselect for uart transmissions, PC0
#define CS_port				GPIOC
#define CS_pin				GPIO_Pin_0
#define CS_exti				EXTI_Pin_0

//DAC, PD7
#define DAC_clk_port	GPIOD
#define DAC_clk_pin		GPIO_Pin_7
#define DAC_clk_exti	EXTI_Pin_7

#endif //__MAPPINGS_H