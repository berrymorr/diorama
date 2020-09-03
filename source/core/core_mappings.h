#ifndef __MAPPINGS_H
#define __MAPPINGS_H

#include "stm8l15x.h"

//#inserted
#define SD_Sb_port			GPIOD
#define SD_Sb_pin				GPIO_Pin_0
//clock
#define SD_CLK_port			GPIOB
#define SD_CLK_pin			GPIO_Pin_5
//mosi
#define SD_DI_port			GPIOB
#define SD_DI_pin				GPIO_Pin_6
//miso
#define SD_DO_port			GPIOB
#define SD_DO_pin				GPIO_Pin_7
//#cs
#define SD_CS_port			GPIOD
#define SD_CS_pin				GPIO_Pin_4


//DAC clk
#define DAC_clk_port		GPIOD
#define DAC_clk_pin			GPIO_Pin_2


//PWOK
#define PWOK_port				GPIOB
#define PWOK_pin				GPIO_Pin_2
#define power_is_ok()		(PWOK_port->IDR & PWOK_pin)


//#PWON
#define PWON_port				GPIOB
#define PWON_pin				GPIO_Pin_3
#define power_on()			PWON_port->ODR|=(uint8_t)(PWON_pin)
#define power_off()			PWON_port->ODR&=(uint8_t)(~PWON_pin)


//SC16IS741 bridge
//#RESET
#define SC16_RESET_port	GPIOD
#define SC16_RESET_pin	GPIO_Pin_6
//#IRQ
#define SC16_IRQ_port		GPIOD
#define SC16_IRQ_pin		GPIO_Pin_7
//SDA
#define SC16_SDA_port		GPIOC
#define SC16_SDA_pin		GPIO_Pin_0
//SCL
#define SC16_SCL_port		GPIOC
#define SC16_SCL_pin		GPIO_Pin_1


//ENA breakouts @ 74126
//light
#define ENA2_port				GPIOC
#define ENA2_pin				GPIO_Pin_2
#define light_head			2
#define ena2_on();			{ENA2_port->ODR|=(uint8_t)(ENA2_pin);while(!(ENA2_port->IDR&ENA2_pin));}
#define ena2_off();			{ENA2_port->ODR&=(uint8_t)(~ENA2_pin);while(ENA2_port->IDR & ENA2_pin);}
//sound
#define ENA3_port				GPIOC
#define ENA3_pin				GPIO_Pin_3
#define sound_head			3
#define ena3_on();			{ENA3_port->ODR|=(uint8_t)(ENA3_pin);while(!(ENA3_port->IDR&ENA3_pin));}
#define ena3_off();			{ENA3_port->ODR&=(uint8_t)(~ENA3_pin);while(ENA3_port->IDR & ENA3_pin);}
//smoke
#define ENA1_port				GPIOC
#define ENA1_pin				GPIO_Pin_4
#define smoke_head			1
#define ena1_on();			{ENA1_port->ODR|=(uint8_t)(ENA1_pin);while(!(ENA1_port->IDR&ENA1_pin));}
#define ena1_off();			{ENA1_port->ODR&=(uint8_t)(~ENA1_pin);while(ENA1_port->IDR & ENA1_pin);}


//UART
//rx
#define RX_port					GPIOC
#define RX_pin					GPIO_Pin_6
#define wait_rx()				while(!(USART1->SR & USART_SR_RXNE))
#define uart_rx(a);			{wait_rx(); a=USART1->DR;}
#define uart_rxtmt(a,b)	{while((--b)&&(!(USART1->SR & USART_SR_RXNE)));a=USART1->DR;}
//tx
#define TX_port					GPIOC
#define TX_pin					GPIO_Pin_5
#define wait_tx()				while(!(USART1->SR & USART_SR_TXE))
#define uart_tx(a);			{wait_tx(); USART1->DR=a;}


//othrz
#define PD3_on()				GPIOD->ODR|=(uint8_t)(GPIO_Pin_3)
#define PD3_off()				GPIOD->ODR&=(uint8_t)(~GPIO_Pin_3)
#define PB0_on()				GPIOB->ODR|=(uint8_t)(GPIO_Pin_0)
#define PB0_off()				GPIOB->ODR&=(uint8_t)(~GPIO_Pin_0)
#define PB1_on()				GPIOB->ODR|=(uint8_t)(GPIO_Pin_1)
#define PB1_off()				GPIOB->ODR&=(uint8_t)(~GPIO_Pin_1)

#endif //__MAPPINGS_H