

////SOUND HEAD

#include "stm8l15x.h"
#include "sound_mappings.h"
#include "..\globals.h"
#include "..\delay.h"


//global vars
volatile uint8_t dac_buf[dac_bufsize];
volatile uint16_t bi_in=0,bi_out=0;
volatile uint32_t buf_underrun=0;
volatile uint8_t state=S_PWRON;






int main()
{
  volatile uint16_t i;
	uint8_t data;
	uint16_t r_bufsize=0;

	//CLK init
	CLK_SYSCLKSourceSwitchCmd(ENABLE);
  CLK_SYSCLKSourceConfig(CLK_SYSCLKSource_HSE);
	CLK_HSEConfig(CLK_HSE_ON);
	while (CLK_GetSYSCLKSource() != CLK_SYSCLKSource_HSE);
  CLK_SYSCLKDivConfig(CLK_SYSCLKDiv_1);


	delay_ms(100);//wait 4 core init

	//DAC clk from CORE(11), TIM1CH1(PD2)
	EXTI_DeInit();
	EXTI_SetPinSensitivity((uint8_t)(DAC_clk_exti),EXTI_Trigger_Rising);
	//GPIO_Init(DAC_clk_port,DAC_clk_pin,GPIO_Mode_In_PU_No_IT);
	GPIO_Init(DAC_clk_port,DAC_clk_pin,GPIO_Mode_In_PU_IT);


	//CS from CORE(28), ENA3(PC3)
	//EXTI_SetPinSensitivity((uint8_t)(CS_exti),EXTI_Trigger_Rising_Falling);
	//GPIO_Init(CS_port,CS_pin,GPIO_Mode_In_PU_IT);
	

	//remap UART to PC5-TX, PC6-RX
	SYSCFG_REMAPPinConfig(REMAP_Pin_USART1TxRxPortC,ENABLE);
	//UART init, PC5-TX to RX3, PC6-RX to TX
	CLK_PeripheralClockConfig(CLK_Peripheral_USART1,ENABLE);
	USART_DeInit(USART1);
  GPIO_Init(RX_port,RX_pin,GPIO_Mode_In_PU_No_IT);
	GPIO_Init(TX_port,TX_pin,GPIO_Mode_Out_PP_High_Fast);
	USART_Init(USART1,uart_baud,USART_WordLength_8b,USART_StopBits_1,
							USART_Parity_No,USART_Mode_Tx|USART_Mode_Rx);
	//USART_ITConfig(USART1,USART_IT_RXNE,ENABLE);
	

	//DAC
  CLK_PeripheralClockConfig(CLK_Peripheral_DAC,ENABLE);
	CLK_PeripheralClockConfig(CLK_Peripheral_COMP,ENABLE);//need 4 RI functional
	RI->IOSR3|=0x10;//connect DAC_OUT to PB4
  DAC->CH1CR1=DAC_CR1_TEN//|DAC_CR1_BOFF
	|DAC_Trigger_Software;
  DAC->CH1CR1|=DAC_CR1_EN;
	

	_asm("rim");
	
	while(1){
		while(!(USART1->SR & USART_SR_RXNE));//wait for rx
		data=USART1->DR;
		if (CS_port->IDR & CS_pin) {//chip selected, private data
			switch(data) {

				case C_SELF_TEST:
					//selftesting, check here for DAC clks and maybe use ADC to check 5532
					USART1->DR=A_OK;
					break;
				
				case C_RESET:
					//GPIO_Init(DAC_clk_port,DAC_clk_pin,GPIO_Mode_In_PU_No_IT);
					/*buf_underrun=0;
					state=S_RESET;	
					bi_in=0; bi_out=0;
					DAC->CH1DHR8=0;//set zero value
					DAC->SWTRIGR|=DAC_SWTRIGR_SWTRIG1;*/
					IWDG->KR=0xCC;//enable watchdog 4 software reset
					while(1);
					break;
				
				case C_SEND_BUF:
				//getting bufsize
				while(!(USART1->SR & USART_SR_RXNE));//wait for rx
				r_bufsize=USART1->DR;//MSB
				r_bufsize*=0x100;
				while(!(USART1->SR & USART_SR_RXNE));//wait for rx
				r_bufsize|=USART1->DR;//LSB
				//here check if we can get all this r_bufsize bytes into our ring buffer
				if ((dac_bufsize-dac_bufremain)>r_bufsize) {//get buffer free size
					USART1->DR=A_OK;
					for (i=0; i<r_bufsize; i++) {
						while(!(USART1->SR & USART_SR_RXNE));//wait for rx
						dac_buf[bi_in]=USART1->DR;
						bi_in++;
						bi_in&=dac_bufmask;
					}
					USART1->DR=A_OK;//return OK if we get all data ok
				}
				else {
					USART1->DR=A_ERROR;//we don't like this data size
				}
					break;
				
				case C_SET_PARAM:
					while(!(USART1->SR & USART_SR_RXNE));//wait for rx
					data=USART1->DR;
					if (data==P_DAC){//set DAC value here
						USART1->DR=A_OK;
						while(!(USART1->SR & USART_SR_RXNE));//wait for rx
						data=USART1->DR;
						DAC->CH1DHR8=data;//set value
						DAC->SWTRIGR|=DAC_SWTRIGR_SWTRIG1;
					}
					else USART1->DR=A_ERROR;//dont' know anymore parameters
					break;

				case C_GET_PARAM:
					while(!(USART1->SR & USART_SR_RXNE));//wait for rx
					data=USART1->DR;

					switch (data) {
						case P_DAC: //get DAC value
							USART1->DR=A_OK;//send ack
							while(!(USART1->SR & USART_SR_TXE));//wait for tx
							USART1->DR=DAC->CH1DHR8;//send current value
							break;
							
						case P_STATE:
							USART1->DR=A_OK;//send ack
							while(!(USART1->SR & USART_SR_TXE));//wait for tx
							USART1->DR=state;//send current value
							break;
							
						case P_BUF_REMAIN:
							USART1->DR=A_OK;//send ack
							while(!(USART1->SR & USART_SR_TXE));//wait for tx
							USART1->DR=hi(dac_bufremain);//send current value
							while(!(USART1->SR & USART_SR_TXE));//wait for tx
							USART1->DR=lo(dac_bufremain);//send current value
							break;
							
						case P_BUF_UNDERRUN:
							USART1->DR=A_OK;//send ack
							while(!(USART1->SR & USART_SR_TXE));//wait for tx
							if (buf_underrun>0xFF) USART1->DR=0xFF;
							else USART1->DR=(uint8_t)(buf_underrun);//just LSB
							break;
						
						default:
							USART1->DR=A_ERROR;//dont' know anymore parameters
							break;
					}
					
					break;
				
				default://unrecognized command
					USART1->DR=A_ERROR;
					break;
			}
		}
		else {//chip not selected, maybe it's broadcast?
		/*	if (!(data & 0x80)) {//MSBit=0, it's broadcast
				switch(data) {
					case BC_START:
						if (dac_bufremain>0) GPIO_Init(DAC_clk_port,DAC_clk_pin,GPIO_Mode_In_PU_IT);
						state=S_PLAY;
						break;
					
					case BC_STOP:
						GPIO_Init(DAC_clk_port,DAC_clk_pin,GPIO_Mode_In_PU_No_IT);
						bi_in=0; bi_out=0;
						DAC->CH1DHR8=0;//set zero value
						DAC->SWTRIGR|=DAC_SWTRIGR_SWTRIG1;				
						state=S_STOP;
						break;
						
					case BC_PAUSE:
						GPIO_Init(DAC_clk_port,DAC_clk_pin,GPIO_Mode_In_PU_No_IT);
						state=S_STOP;
						break;
					
					default:
						//do nothing - unrecognized broadcast
						break;
					}
				
			}
			else*/ data=0;//this message is for another head
		}

	}

	return 0;
}