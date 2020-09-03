

//LIGHT INTERRUPTS

#include "stm8l15x.h"
#include "stm8l15x_it.h"
#include "stm8l15x_spi.h"
#include "..\globals.h"
#include "light_mappings.h"
#include "spi_595.h"

INTERRUPT_HANDLER(NonHandledInterrupt,0)
{
  while (1);
}
INTERRUPT_HANDLER_TRAP(TRAP_IRQHandler)
{
  while (1);
}
INTERRUPT_HANDLER(FLASH_IRQHandler,1)
{
  while (1);
}
INTERRUPT_HANDLER(DMA1_CHANNEL0_1_IRQHandler,2)
{
  while (1);
}
INTERRUPT_HANDLER(DMA1_CHANNEL2_3_IRQHandler,3)
{
/*	extern volatile uint8_t read_buf1;
	extern volatile uint8_t write_buf1;
	extern volatile uint8_t updating;
	extern volatile uint8_t step;

	//GPIOD->ODR|=(uint8_t)(GPIO_Pin_7);//debug

	while(SPI_595->SR&SPI_FLAG_BSY);
	latch_595_ring();
	SPI_Cmd(SPI_595,DISABLE);

	step++;
	if (step>7) {
		step=0;
		if (!updating) read_buf1=write_buf1;
	}

	//disable to be able to reapply settings at tim2 int
	DMA1->GCSR &= (uint8_t)(~DMA_GCSR_GE);
	DMA1_Channel2->CCR &= (uint8_t)(~DMA_CCR_CE);
	
	//GPIOD->ODR&=(uint8_t)(~GPIO_Pin_7);//debug
	*/
	DMA1_Channel2->CSPR&=(uint8_t)(~(DMA_CSPR_TCIF|DMA_CSPR_HTIF));
}

INTERRUPT_HANDLER(RTC_IRQHandler,4)
{
  while (1);
}
INTERRUPT_HANDLER(EXTIE_F_PVD_IRQHandler,5)
{
  while (1);
}
INTERRUPT_HANDLER(EXTIB_IRQHandler,6)
{
  while (1);
}
INTERRUPT_HANDLER(EXTID_IRQHandler,7)
{
  while(1);
}

//ENA3
INTERRUPT_HANDLER(EXTI0_IRQHandler,8)
{
  //if (CS_port->IDR & CS_pin) USART1->CR2|=USART_CR2_REN;//enable UART's RX
  //else USART1->CR2&=(uint8_t)(USART_CR2_REN);//disable
  //Clear interrupt flag
  EXTI->SR1 = (uint8_t)0x01;//bit0
}

INTERRUPT_HANDLER(EXTI1_IRQHandler,9)
{
  while(1);
}
INTERRUPT_HANDLER(EXTI2_IRQHandler,10)
{
  while (1);
}
INTERRUPT_HANDLER(EXTI3_IRQHandler,11)
{
  while (1);
}
INTERRUPT_HANDLER(EXTI4_IRQHandler,12)
{
	/*if (CS_port->IDR & CS_pin) USART1->CR2|=(uint8_t)(USART_CR2_REN);
	else USART1->CR2&=(uint8_t)(~USART_CR2_REN);*/
	//clear interrupt
	EXTI->SR1 = (uint8_t)0x10;//bit4
}
INTERRUPT_HANDLER(EXTI5_IRQHandler,13)
{
  while (1);
}
INTERRUPT_HANDLER(EXTI6_IRQHandler,14)
{
  while (1);
}

//DAC clk
INTERRUPT_HANDLER(EXTI7_IRQHandler,15)
{
  //Clear interrupt flag
  EXTI->SR1 = (uint8_t)0x80;
}
INTERRUPT_HANDLER(LCD_IRQHandler,16)
{
  while (1);
}
INTERRUPT_HANDLER(SWITCH_CSS_BREAK_DAC_IRQHandler,17)
{
  while (1);
}
INTERRUPT_HANDLER(ADC1_COMP_IRQHandler,18)
{
  while (1);
}
//BAM update
INTERRUPT_HANDLER(TIM2_UPD_OVF_TRG_BRK_IRQHandler,19)
{
	extern uint16_t l_table[10];
	extern volatile uint8_t out_buf[10][light_bufsize/8];
	extern volatile uint8_t out_buf1[10][light_bufsize/8];
	
	extern volatile uint8_t read_buf1;
	extern volatile uint8_t step;

	//4 software transmit
	extern volatile uint8_t write_buf1;
	extern volatile uint8_t updating;
/*
	GPIOD->ODR|=(uint8_t)(GPIO_Pin_6);//debug

	TIM2->PSCR=step;
	TIM2->EGR|=TIM_EGR_UG;//generate update to refresh prescaler
*/
	//SOFTWARE TRANSMIT
	/*if (read_buf1) send_595_ring(out_buf1[step]);
	else send_595_ring(out_buf[step]);
	step++;
	if (step>7) {
		step=0;
		if (!updating) read_buf1=write_buf1;
	}*/

	//=====HARDCORE SOFTWARE TRANSMIT========
	//cyclez stealz cpu cyclez
	if (read_buf1) {
		//send_595_ring(out_buf1[step]);
		while(!(SPI_595->SR&SPI_FLAG_TXE));
		SPI_595->DR=out_buf1[step][0];
		while(!(SPI_595->SR&SPI_FLAG_TXE));
		SPI_595->DR=out_buf1[step][1];
		while(!(SPI_595->SR&SPI_FLAG_TXE));
		SPI_595->DR=out_buf1[step][2];
		while(!(SPI_595->SR&SPI_FLAG_TXE));
		SPI_595->DR=out_buf1[step][3];
		while(!(SPI_595->SR&SPI_FLAG_TXE));
		SPI_595->DR=out_buf1[step][4];
		while(!(SPI_595->SR&SPI_FLAG_TXE));
		SPI_595->DR=out_buf1[step][5];
		while(!(SPI_595->SR&SPI_FLAG_TXE));
		SPI_595->DR=out_buf1[step][6];
		while(!(SPI_595->SR&SPI_FLAG_TXE));
		SPI_595->DR=out_buf1[step][7];
	}
	else {
		//send_595_ring(out_buf[step]);
		while(!(SPI_595->SR&SPI_FLAG_TXE));
		SPI_595->DR=out_buf[step][0];
		while(!(SPI_595->SR&SPI_FLAG_TXE));
		SPI_595->DR=out_buf[step][1];
		while(!(SPI_595->SR&SPI_FLAG_TXE));
		SPI_595->DR=out_buf[step][2];
		while(!(SPI_595->SR&SPI_FLAG_TXE));
		SPI_595->DR=out_buf[step][3];
		while(!(SPI_595->SR&SPI_FLAG_TXE));
		SPI_595->DR=out_buf[step][4];
		while(!(SPI_595->SR&SPI_FLAG_TXE));
		SPI_595->DR=out_buf[step][5];
		while(!(SPI_595->SR&SPI_FLAG_TXE));
		SPI_595->DR=out_buf[step][6];
		while(!(SPI_595->SR&SPI_FLAG_TXE));
		SPI_595->DR=out_buf[step][7];
	}
	
	TIM2->ARRH=(uint8_t)(l_table[step]>>8) ;
	TIM2->ARRL=(uint8_t)(l_table[step]);
	//TIM2->PSCR=step;
	TIM2->EGR|=TIM_EGR_UG;//generate update to refresh prescaler
	step++;
	if (step>9) {
		step=0;
		if (!updating) read_buf1=write_buf1;
	}
	while(SPI_595->SR&SPI_FLAG_BSY);
	TIM2->SR1&=(uint8_t)(~TIM2_FLAG_Update);
	latch_595_ring();
	//=====END OF HARDCORE SOFTWARE TRANSMIT========
	
	/*//DMA TRANSFER
	if (read_buf1) {
		DMA1_Channel2->CM0ARH = (uint8_t)((uint16_t)(out_buf1[step]) >> (uint8_t)8);
		DMA1_Channel2->CM0ARL = (uint8_t)((uint16_t)(out_buf1[step]));
	}
	else {
		DMA1_Channel2->CM0ARH = (uint8_t)((uint16_t)(out_buf[step]) >> (uint8_t)8);
		DMA1_Channel2->CM0ARL = (uint8_t)((uint16_t)(out_buf[step]));
	}

	DMA1_Channel2->CNBTR=8;
	DMA1_Channel2->CCR |= (uint8_t)(DMA_CCR_CE);//dma enable
	DMA1->GCSR |= (uint8_t)DMA_GCSR_GE;

	SPI_Cmd(SPI_595,ENABLE);*/
	
	/*GPIOD->ODR&=(uint8_t)(~GPIO_Pin_6);//debug
	
	TIM2->SR1&=(uint8_t)(~TIM2_FLAG_Update);*/
}

INTERRUPT_HANDLER(TIM2_CAP_IRQHandler,20)
{
  while (1);
}
INTERRUPT_HANDLER(TIM3_UPD_OVF_TRG_BRK_IRQHandler,21)
{
	TIM3->SR1&=(uint8_t)(~TIM3_FLAG_Update);
}

INTERRUPT_HANDLER(TIM3_CAP_IRQHandler,22)
{
  while (1);
}
INTERRUPT_HANDLER(TIM1_UPD_OVF_TRG_COM_IRQHandler,23)
{
  while (1);
}
INTERRUPT_HANDLER(TIM1_CAP_IRQHandler,24)
{
  while (1);
}
INTERRUPT_HANDLER(TIM4_UPD_OVF_TRG_IRQHandler,25)
{
  while (1);
}
INTERRUPT_HANDLER(SPI1_IRQHandler,26)
{
  while (1);
}
INTERRUPT_HANDLER(USART1_TX_IRQHandler,27)
{
  while (1);
}


INTERRUPT_HANDLER(USART1_RX_IRQHandler,28)
{
  extern volatile uint8_t rxbuf[256];
  extern volatile uint8_t rx_in;
	/*volatile uint8_t tmp;
	tmp=USART1->DR;
	if (CS_port->IDR & CS_pin) {
		rxbuf[rx_in]=tmp;
		rx_in++;
	}*/
	rxbuf[rx_in]=USART1->DR;;
	rx_in++;
}

INTERRUPT_HANDLER(I2C1_IRQHandler,29)
{
  while (1);
}
