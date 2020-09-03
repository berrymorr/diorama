

////LIGHT HEAD

#include "stm8l15x.h"
#include "stm8l15x_spi.h"
#include "light_mappings.h"
#include "..\globals.h"
#include "..\delay.h"
#include "spi_595.h"


//global vars
const uint16_t exp_table[256]={
0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,
16,17,18,19,20,21,22,23,24,24,24,24,24,24,24,24,
24,25,26,27,28,28,28,28,28,29,30,31,32,32,32,32,
32,33,34,35,36,36,36,36,36,37,38,39,40,41,42,43,
44,45,46,47,48,48,48,48,48,49,50,51,52,53,54,55,
56,57,58,59,60,61,62,63,64,65,66,67,68,70,72,74,
76,77,78,79,80,81,82,83,84,86,88,90,92,93,94,95,
96,98,100,102,104,106,108,110,112,114,116,118,120,122,124,126,
128,130,132,134,136,138,140,142,144,147,150,153,156,159,162,165,
168,170,172,174,176,180,184,188,192,195,198,201,204,207,210,213,
216,220,224,228,232,236,240,244,248,252,256,260,264,269,274,279,
284,289,294,299,304,309,314,319,324,330,336,342,348,354,360,366,
372,379,386,393,400,406,412,418,424,432,440,448,456,464,472,480,
488,496,504,512,520,529,538,547,556,566,576,586,596,606,616,626,
636,647,658,669,680,692,704,716,728,741,754,767,780,793,806,819,
832,847,862,877,892,907,922,937,952,969,986,1003,1020,1021,1022,1023};


/*
5000ìêñ, 2500ìêñ, 1250ìêñ, 625ìêñ, 312ìêñ, 156ìêñ, 78ìêñ, 39ìêñ, 20, 10
80k        40k      20k     10k      5k      2.5k   1.25k  625   313 156

presc=2, arr:
40k        20k      10k     5k      2.5k     1.25k   625   313   156  78
*/

uint16_t l_table[10]={78,156,313,625,1250,2500,5000,10000,20000,40000};

volatile uint8_t light_buf[light_bufsize]={0};
/*{255,129,127,0x11, 0x22,0x33,0x44,0x55,
0x66,0x77,0x88,3 ,2,1,3,2,
3,2,1,3 ,2,1,3,2,
3,2,1,3 ,2,1,3,2,
3,2,1,3 ,2,1,3,2,
3,2,1,3 ,2,1,3,2,
3,2,1,3 ,2,1,3,2,
3,2,1,3 ,2,1,0xAA,255};*/
/*volatile uint8_t out_buf[8][light_bufsize/8]={1};//DEBUG
volatile uint8_t out_buf1[8][light_bufsize/8]={0};*/

volatile uint8_t out_buf[10][light_bufsize/8]={1};//DEBUG
volatile uint8_t out_buf1[10][light_bufsize/8]={0};

volatile uint8_t rxbuf[256];
volatile uint8_t rx_in=0;
volatile uint8_t rx_out=0;

volatile uint8_t state=S_PWRON;

volatile uint8_t read_buf1=0;
volatile uint8_t write_buf1=0;
volatile uint8_t updating=0;

volatile uint8_t step=0;

/*//convert input buffer to BAM steps
void value2BAM(void)
{
	uint8_t i,k,mask;
	uint8_t *out;
	
	if (read_buf1) {
		out=&(out_buf[0][0]);
		read_buf1=0;
	}
	else {
		out=&(out_buf1[0][0]);
		read_buf1=1;
	}
	
	for(i=0; i<light_bufsize; i++) {
		if (i%8==0) {
			*(out+(uint16_t)(i/8))=(uint8_t)(light_buf[i] & 0x01);//out_buf[0][i/8]
			*(out+(uint16_t)(8+i/8))=(uint8_t)((light_buf[i] & 0x02)>>1);//out_buf[1][i/8]
			*(out+(uint16_t)(16+i/8))=(uint8_t)((light_buf[i] & 0x04)>>2);//out_buf[2][i/8]
			*(out+(uint16_t)(24+i/8))=(uint8_t)((light_buf[i] & 0x08)>>3);//out_buf[3][i/8]
			*(out+(uint16_t)(32+i/8))=(uint8_t)((light_buf[i] & 0x10)>>4);//out_buf[4][i/8]
			*(out+(uint16_t)(40+i/8))=(uint8_t)((light_buf[i] & 0x20)>>5);//out_buf[5][i/8]
			*(out+(uint16_t)(48+i/8))=(uint8_t)((light_buf[i] & 0x40)>>6);//out_buf[6][i/8]
			*(out+(uint16_t)(56+i/8))=(uint8_t)((light_buf[i] & 0x80)>>7);//out_buf[7][i/8]
		}
		else {
			*(out+(uint16_t)(i/8))|=(uint8_t)((light_buf[i] & 0x01)<<(i%8));//out_buf[0][i/8]
			*(out+(uint16_t)(8+i/8))|=(uint8_t)(((light_buf[i] & 0x02)>>1)<<(i%8));//out_buf[1][i/8]
			*(out+(uint16_t)(16+i/8))|=(uint8_t)(((light_buf[i] & 0x04)>>2)<<(i%8));//out_buf[2][i/8]
			*(out+(uint16_t)(24+i/8))|=(uint8_t)(((light_buf[i] & 0x08)>>3)<<(i%8));//out_buf[3][i/8]
			*(out+(uint16_t)(32+i/8))|=(uint8_t)(((light_buf[i] & 0x10)>>4)<<(i%8));//out_buf[4][i/8]
			*(out+(uint16_t)(40+i/8))|=(uint8_t)(((light_buf[i] & 0x20)>>5)<<(i%8));//out_buf[5][i/8]
			*(out+(uint16_t)(48+i/8))|=(uint8_t)(((light_buf[i] & 0x40)>>6)<<(i%8));//out_buf[6][i/8]
			*(out+(uint16_t)(56+i/8))|=(uint8_t)(((light_buf[i] & 0x80)>>7)<<(i%8));//out_buf[7][i/8]

		}
	}
}*/


//with exp_table
void value2BAM(void)
{
	uint8_t i,k,*out;
	uint16_t tval,mask;
	updating=1;
	write_buf1=(uint8_t)(read_buf1^0x01);//writebuf1=!readbuf1

	//reset values
	if (write_buf1) out=&(out_buf1[0][0]);
	else out=&(out_buf[0][0]);
	for (i=0;i<80;i++) {
		*out=0;
		out++;
	}
	
	//fill new
	for(k=0; k<10; k++) {//bitdepth
		mask=(uint16_t)(1<<k);
		for(i=0;i<light_bufsize;i++) {
			tval=exp_table[light_buf[i]];//256 table
			//tval=exp_table[light_buf[i]/4];//64 table
			if(tval&mask) {
				if (write_buf1) out_buf1[k][i/8]|=(uint8_t)(1<<((uint8_t)(i%8)));
				else out_buf[k][i/8]|=(uint8_t)(1<<((uint8_t)(i%8)));
			}
		}
	}
	updating=0;//we just updated this buffer
}

/*//backup
void value2BAM(void)
{
	uint8_t i,k,mask,*out;

	//_asm("sim");//atomic
	updating=1;
	write_buf1=(uint8_t)(read_buf1^0x01);//writebuf1=!readbuf1
	//_asm("rim");//end of atomic
	
	//reset values
	if (write_buf1) out=&(out_buf1[0][0]);
	else out=&(out_buf[0][0]);
	for (i=0;i<light_bufsize;i++) {
		*out=0;
		out++;
	}

	
	//fill new
	for(k=0; k<8; k++) {
		mask=(uint8_t)(1<<k);
		for(i=0;i<light_bufsize;i++) {
			if(light_buf[i]&mask) {
				if (write_buf1) out_buf1[k][i/8]|=(uint8_t)(1<<((uint8_t)(i%8)));
				else out_buf[k][i/8]|=(uint8_t)(1<<((uint8_t)(i%8)));
			}
		}
	}

	//_asm("sim");//atomic
	updating=0;//we just updated this buffer
	//_asm("rim");//end of atomic
}*/

void BAM_reset(void)
{
	uint8_t i,*out;

	//reset values
	if (write_buf1) out=&(out_buf1[0][0]);
	else out=&(out_buf[0][0]);
	for (i=0;i<light_bufsize;i++) {
		*out=0;
		out++;
	}
}


void hardware_init(void)
{
  //CLK init
	delay_ms(13);//wait 4 13*8=100ms (CKDIV8==1)
  CLK_SYSCLKSourceSwitchCmd(ENABLE);
  CLK_SYSCLKSourceConfig(CLK_SYSCLKSource_HSE);
  CLK_HSEConfig(CLK_HSE_ON);
  while (CLK_GetSYSCLKSource() != CLK_SYSCLKSource_HSE);
  CLK_SYSCLKDivConfig(CLK_SYSCLKDiv_1);

	//delay_ms(1000);//wait 4 Vcc (core started us)

	//CS from CORE
	/*EXTI_SetPinSensitivity((uint8_t)(CS_exti),EXTI_Trigger_Rising_Falling);
	GPIO_Init(CS_port,CS_pin,GPIO_Mode_In_PU_IT);*/
	GPIO_Init(CS_port,CS_pin,GPIO_Mode_In_PU_No_IT);

	//UART
	GPIO_Init(RX_port,RX_pin,GPIO_Mode_In_PU_No_IT);
	GPIO_Init(TX_port,TX_pin,GPIO_Mode_Out_PP_High_Fast);
  //remap UART to PC5-TX, PC6-RX
  SYSCFG_REMAPPinConfig(REMAP_Pin_USART1TxRxPortC,ENABLE);
  //UART init, PC5-TX to RX3, PC6-RX to TX
  CLK_PeripheralClockConfig(CLK_Peripheral_USART1,ENABLE);
  USART_DeInit(USART1);
  GPIO_Init(RX_port,RX_pin,GPIO_Mode_In_PU_No_IT);
  GPIO_Init(TX_port,TX_pin,GPIO_Mode_Out_PP_High_Fast);
  USART_Init(USART1,uart_baud,USART_WordLength_8b,USART_StopBits_1,
              USART_Parity_No,USART_Mode_Tx|USART_Mode_Rx);
  USART_ITConfig(USART1,USART_IT_RXNE,ENABLE);
	
	//TIM2
	TIM2_DeInit();
	CLK_PeripheralClockConfig(CLK_Peripheral_TIM2,ENABLE);
	TIM2_TimeBaseInit(TIM2_Prescaler_1,TIM2_CounterMode_Down,156);//624=39us
	TIM2_ARRPreloadConfig(ENABLE);
	TIM2_ITConfig(TIM2_IT_Update,ENABLE);
	TIM2_Cmd(ENABLE);

	//SPI
	SPI_DeInit(SPI_595);
  CLK_PeripheralClockConfig(SPI_595_CLK, ENABLE);
  GPIO_Init(SH_clk_port,SH_clk_pin,GPIO_Mode_Out_PP_Low_Fast);
  GPIO_Init(DO_port,DO_pin,GPIO_Mode_In_PU_No_IT);
  GPIO_Init(DI_port,DI_pin,GPIO_Mode_Out_PP_High_Fast);
  SPI_Init(SPI_595, SPI_FirstBit_LSB, SPI_BaudRatePrescaler_2, SPI_Mode_Master,
           SPI_CPOL_Low, SPI_CPHA_1Edge, SPI_Direction_2Lines_FullDuplex,
           SPI_NSS_Soft, 0x07);
	//SPI_DMACmd(SPI_595,SPI_DMAReq_TX,ENABLE);
  SPI_Cmd(SPI_595, ENABLE);

	/*//DMA
	DMA_GlobalDeInit();
	DMA_DeInit(DMA1_Channel2);
	CLK_PeripheralClockConfig(CLK_Peripheral_DMA1, ENABLE);
	DMA_Init(DMA1_Channel2,((uint16_t)(out_buf[0])),((uint16_t)(&SPI_595->DR)),8,
					DMA_DIR_MemoryToPeripheral,DMA_Mode_Normal,DMA_MemoryIncMode_Inc,
					DMA_Priority_Low,DMA_MemoryDataSize_Byte);//(SPI1_BASE+0x04)
	DMA_SetTimeOut(60);
	DMA_ITConfig(DMA1_Channel2, DMA_ITx_TC, ENABLE);*/
	
	//595
	GPIO_Init(ST_clk_port,ST_clk_pin,GPIO_Mode_Out_PP_Low_Fast);//storage clk
	GPIO_Init(OE_port,OE_pin,GPIO_Mode_Out_PP_High_Fast);//#output enable
	GPIO_Init(RST_port,RST_pin,GPIO_Mode_Out_PP_Low_Fast);//#reset

	//interrupt priorities
	//ITC_SetSoftwarePriority(DMA1_CHANNEL2_3_IRQn,ITC_PriorityLevel_1);
	//ITC_SetSoftwarePriority(TIM2_UPD_OVF_TRG_BRK_IRQn,ITC_PriorityLevel_1);
	//ITC_SetSoftwarePriority(USART1_RX_IRQn,ITC_PriorityLevel_3);
	//ITC_SetSoftwarePriority(EXTI4_IRQn,ITC_PriorityLevel_3);
}

uint8_t loopback(uint8_t a)
{
	uint8_t i,d;
	
	spi_txrx_byte(a);
	for(i=0;i<8;i++) {
		d=spi_txrx_byte((uint8_t)((i+1)*0x11));
	}
	return d;
}

int main()
{
  volatile uint16_t i;
  volatile uint8_t data,q;
  uint16_t r_bufsize=0;
	uint8_t selftestres=A_ERROR;
	
	hardware_init();
	//delay_ms(1000);//1000ms<s>, CKDIV=8 @ start</s>
	
	disable_595_ring();
	reset_595_ring();
	selftestres=loopback(A_OK);
	reset_595_ring();
	enable_595_ring();
	
	//SPI_Cmd(SPI_595, DISABLE);//disable for DMA	
	_asm("rim");
	
	GPIO_Init(GPIOD,GPIO_Pin_7,GPIO_Mode_Out_PP_Low_Fast);//DEBUG

  while(1){
    while(!(rx_in-rx_out));//wait for rx
    data=rxbuf[rx_out]; rx_out++;
    if (CS_port->IDR & CS_pin) {//chip selected, private data
      switch(data) {

        case C_SELF_TEST:
          //selftesting 595 ring
          USART1->DR=selftestres;
          break;

        case C_RESET:
					disable_595_ring();
					reset_595_ring();
					IWDG->KR=0xCC;//enable watchdog 4 software reset
          while(1);
          break;

        case C_SEND_BUF:
        //getting bufsize				
				while(!(rx_in-rx_out));//wait for rx
				r_bufsize=rxbuf[rx_out]; rx_out++;//MSB
        r_bufsize*=0x100;
				while(!(rx_in-rx_out));//wait for rx
				r_bufsize|=rxbuf[rx_out]; rx_out++;//LSB
				if(r_bufsize==light_bufsize){//it is a good size!
					GPIOD->ODR|=(uint8_t)(GPIO_Pin_7);//debug
					wait_tx();
					USART1->DR=A_OK;//ACK
					/*//new-style
					updating=1;//now we r updating unreading buf
					write_buf1=(uint8_t)(read_buf1^0x01);//writebuf1=!readbuf1
					BAM_reset();
					//\new-style*/
          for (i=0; i<r_bufsize; i++) {
						while(!(rx_in-rx_out));//wait for rx
						light_buf[i]=rxbuf[rx_out]; rx_out++;//old style
						/*//new-style
						if (write_buf1) {//write to out_buf1
							if(rxbuf[rx_out]&(1<<0)) out_buf1[0][i/8]|=(uint8_t)(1<<(i%8));
							if(rxbuf[rx_out]&(1<<1)) out_buf1[1][i/8]|=(uint8_t)(1<<(i%8));
							if(rxbuf[rx_out]&(1<<2)) out_buf1[2][i/8]|=(uint8_t)(1<<(i%8));
							if(rxbuf[rx_out]&(1<<3)) out_buf1[3][i/8]|=(uint8_t)(1<<(i%8));
							if(rxbuf[rx_out]&(1<<4)) out_buf1[4][i/8]|=(uint8_t)(1<<(i%8));
							if(rxbuf[rx_out]&(1<<5)) out_buf1[5][i/8]|=(uint8_t)(1<<(i%8));
							if(rxbuf[rx_out]&(1<<6)) out_buf1[6][i/8]|=(uint8_t)(1<<(i%8));
							if(rxbuf[rx_out]&(1<<7)) out_buf1[7][i/8]|=(uint8_t)(1<<(i%8));
						}
						else {//write to out_buf
							if(rxbuf[rx_out]&(1<<0)) out_buf[0][i/8]|=(uint8_t)(1<<(i%8));
							if(rxbuf[rx_out]&(1<<1)) out_buf[1][i/8]|=(uint8_t)(1<<(i%8));
							if(rxbuf[rx_out]&(1<<2)) out_buf[2][i/8]|=(uint8_t)(1<<(i%8));
							if(rxbuf[rx_out]&(1<<3)) out_buf[3][i/8]|=(uint8_t)(1<<(i%8));
							if(rxbuf[rx_out]&(1<<4)) out_buf[4][i/8]|=(uint8_t)(1<<(i%8));
							if(rxbuf[rx_out]&(1<<5)) out_buf[5][i/8]|=(uint8_t)(1<<(i%8));
							if(rxbuf[rx_out]&(1<<6)) out_buf[6][i/8]|=(uint8_t)(1<<(i%8));
							if(rxbuf[rx_out]&(1<<7)) out_buf[7][i/8]|=(uint8_t)(1<<(i%8));
						}
						rx_out++;
						//\new-style*/
          }
					wait_tx();//return OK if we get all data ok
          USART1->DR=A_OK;//ACK
					value2BAM();//put new data to output buffer
					enable_595_ring();
					GPIOD->ODR&=(uint8_t)(~GPIO_Pin_7);//debug
					//updating=0;//we just updated this buffer
        }
        else {
					wait_tx();
          USART1->DR=A_ERROR;//we don't like this data size
        }
				break;

        case C_SET_PARAM:
					while(!(rx_in-rx_out));//wait for rx
					data=rxbuf[rx_out]; rx_out++;//get param
					switch(data) {
						
						case P_LIGHT_CH:
							while(!(rx_in-rx_out));//wait for rx
							data=rxbuf[rx_out]; rx_out++;//get channel num
							if (data<64) {
								USART1->DR=A_OK;//ack
								while(!(rx_in-rx_out));//wait for rx
								light_buf[data]=rxbuf[rx_out]; rx_out++;//set channel value
								value2BAM();
								enable_595_ring();
							}
							else USART1->DR=A_ERROR;//bad channel num
							break;
						
						default:
							USART1->DR=A_ERROR;//dont' know any parameters
							break;
					}
          break;

        case C_GET_PARAM:
					while(!(rx_in-rx_out));//wait for rx
					data=rxbuf[rx_out]; rx_out++;//get param

          switch (data) {
            case P_STATE:
              USART1->DR=A_OK;//send ack
              wait_tx();
              USART1->DR=state;//send current value
              break;

            case P_BUF_REMAIN:
							wait_tx();
              USART1->DR=A_OK;//send ack
              wait_tx();
              USART1->DR=0;//send hi
              wait_tx();
              USART1->DR=64;//send lo
              break;

            case P_BUF_UNDERRUN:
							wait_tx();
              USART1->DR=A_OK;//send ack
              wait_tx();
              USART1->DR=0;
              break;

						case P_LIGHT_CH:
							while(!(rx_in-rx_out));//wait for rx
							data=rxbuf[rx_out]; rx_out++;//get channel num
							if (data<64) {
								wait_tx();
								USART1->DR=A_OK;//ack
								wait_tx();
								USART1->DR=light_buf[data];//send channel value
							}
							else USART1->DR=A_ERROR;//bad channel num
							break;

            default:
							wait_tx();
              USART1->DR=A_ERROR;//dont' know anymore parameters
              break;
          }
          break;

        case C_LIGHT_OFF:
					wait_tx();
					USART1->DR=A_OK;
					disable_595_ring();
					break;

        default://unrecognized command
					wait_tx();
          USART1->DR=A_ERROR;
          break;
      }
    }
    else data=0;//this message is for another head
  }

  return 0;
}