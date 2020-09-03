#ifndef __MAPPINGS_H
#define __MAPPINGS_H

#include "stm8l15x.h"

#define SPI_595_CLK CLK_Peripheral_SPI1
#define SPI_595 SPI1

//UART, RX - PC6, TX - PC5
#define RX_port				GPIOC
#define RX_pin				GPIO_Pin_6
#define wait_rx()			while(!(USART1->SR & USART_SR_RXNE))
#define TX_port				GPIOC
#define TX_pin				GPIO_Pin_5
#define wait_tx()			while(!(USART1->SR & USART_SR_TXE))

//chipselect for uart transmissions, PC4
#define CS_port				GPIOC
#define CS_pin				GPIO_Pin_4
#define CS_exti				EXTI_Pin_4

//595 control
//#OE, output enable
#define OE_port	GPIOB
#define OE_pin		GPIO_Pin_1
//ST_c, storage clk
#define ST_clk_port	GPIOB
#define ST_clk_pin		GPIO_Pin_2
//SH_c, shift clk (to spi's sck)
#define SH_clk_port	GPIOB
#define SH_clk_pin		GPIO_Pin_5
//DI, 595's serial in (to spi's mosi)
#define DI_port	GPIOB
#define DI_pin		GPIO_Pin_6
//DO, 595's Q7' (to spi's miso)
#define DO_port	GPIOB
#define DO_pin		GPIO_Pin_7
//#RST, master reset
#define RST_port	GPIOD
#define RST_pin		GPIO_Pin_4




#endif //__MAPPINGS_H