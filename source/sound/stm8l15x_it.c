

//SOUND INTERRUPTS

#include "stm8l15x.h"
#include "stm8l15x_it.h"
#include "..\globals.h"
#include "sound_mappings.h"


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
	while(1);
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
  while (1);
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
  extern uint16_t bi_out,bi_in;
  extern uint8_t dac_buf[dac_bufsize];
  extern uint8_t half_buf;
  extern uint32_t buf_underrun;
	extern uint8_t state;

	if (dac_bufremain>0) {
		//sound
		DAC->CH1DHR8=dac_buf[bi_out];
		DAC->SWTRIGR|=DAC_SWTRIGR_SWTRIG1;
		bi_out++;
		bi_out&=dac_bufmask;
	}
	else {
		//like BC_PAUSE
		buf_underrun++;
		state=S_STOP;
		//GPIO_Init(DAC_clk_port,DAC_clk_pin,GPIO_Mode_In_PU_No_IT);
	}



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
INTERRUPT_HANDLER(TIM2_UPD_OVF_TRG_BRK_IRQHandler,19)
{
	while (1);
}
INTERRUPT_HANDLER(TIM2_CAP_IRQHandler,20)
{
	while (1);
}
INTERRUPT_HANDLER(TIM3_UPD_OVF_TRG_BRK_IRQHandler,21)
{
  while (1);
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
	while(1);
}

INTERRUPT_HANDLER(I2C1_IRQHandler,29)
{
  while (1);
}
