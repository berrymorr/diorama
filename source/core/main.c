

////CORE HEAD

#include "stm8l15x.h"
#include "core_mappings.h"
#include "..\globals.h"
#include "spi_sd.h"
#include "sc16.h"
#include "..\delay.h"

#include "stdlib.h"

//const    GPIO_TypeDef* ENA_P[3]={ENA1_port,ENA2_port,ENA3_port};

volatile uint16_t int_counter=0;
volatile uint32_t sd_addr=0;
volatile uint32_t errors=0;

//volatile uint32_t DEBUGVAR=0;


volatile uint8_t CardType;

//init gpios, timers and uart
void hardware_init(void)
{
	//CLK init
	CLK_SYSCLKSourceSwitchCmd(ENABLE);
  CLK_SYSCLKSourceConfig(CLK_SYSCLKSource_HSE);
  while (CLK_GetSYSCLKSource() != CLK_SYSCLKSource_HSE);
  CLK_SYSCLKDivConfig(CLK_SYSCLKDiv_1);

	//DAC clk init - TIM1CH1
	GPIO_Init(DAC_clk_port,DAC_clk_pin,GPIO_Mode_Out_PP_Low_Fast);
	TIM1_DeInit();
  CLK_PeripheralClockConfig(CLK_Peripheral_TIM1,ENABLE);
	TIM1_TimeBaseInit(1,TIM1_CounterMode_Down,181,TIM1_RCR_RESET_VALUE);
  /*TIM1_OC1Init(TIM1_OCMode_Toggle,TIM1_OutputState_Enable,TIM1_OutputNState_Disable,
              1,TIM1_OCPolarity_High,TIM1_OCNPolarity_Low,
							TIM1_OCIdleState_Reset,TIM1_OCNIdleState_Reset);
  TIM1_CtrlPWMOutputs(ENABLE);*/


  //TIM2
  TIM2_DeInit();
  CLK_PeripheralClockConfig(CLK_Peripheral_TIM2,ENABLE);
  //TIM2_TimeBaseInit(TIM2_Prescaler_16,TIM2_CounterMode_Down,18100); //16M/16/18125=55Hz
	TIM2_TimeBaseInit(TIM2_Prescaler_1,TIM2_CounterMode_Down,725);
  TIM2_ARRPreloadConfig(ENABLE);
  TIM2_ITConfig(TIM2_IT_Update,ENABLE);
  

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

	//ENA breakout init
  GPIO_Init(ENA1_port,ENA1_pin,GPIO_Mode_Out_PP_Low_Fast);
	GPIO_Init(ENA2_port,ENA2_pin,GPIO_Mode_Out_PP_Low_Fast);
	GPIO_Init(ENA3_port,ENA3_pin,GPIO_Mode_Out_PP_Low_Fast);
	
	//#PWON
	GPIO_Init(PWON_port,PWON_pin,GPIO_Mode_Out_PP_Low_Fast);
	//PWOK
	GPIO_Init(PWOK_port,PWOK_pin,GPIO_Mode_In_PU_No_IT);
}


//for heads those waiting 4 buffer data
void purge_uart(uint8_t q)
{
	uint8_t c;
	USART1->SR&=(uint8_t)(~USART_SR_TC);//reset transm.compl. flag
	for (c=0; c<q; c++) {
		while(!(USART1->SR & USART_SR_TXE));
		USART1->DR=A_OK;
	}
	while(!(USART1->SR & USART_SR_TC));//wait till transm.complete
	USART1->SR&=(uint8_t)(~USART_SR_TC);//reset transm.compl. flag
}


//self-test head, getting errors
uint32_t head_self_test(GPIO_TypeDef *ENA_port,uint8_t ENA_pin)
{
	volatile uint32_t res=0;
	uint16_t tmr;
	uint8_t temp;
	
	//purge_uart(32);

	ENA_port->ODR|=(uint8_t)(ENA_pin);
	while(!(ENA_port->IDR & ENA_pin));//wait till pin go high

	wait_tx();
	USART1->DR=C_SELF_TEST;
	//wait for rx
	for (tmr=0xFFFF; tmr && (!(USART1->SR & USART_SR_RXNE)); tmr--);
	if (tmr) {//we get something
		temp=USART1->DR;
		if (temp!=A_OK) {
			if ((temp==A_ERROR)|(temp==0x00)|(temp==0xFF)) res=0x01;//we get error response
			else res=(uint32_t)(temp);//we get some device error
		}
	}
	else {
		res=0x01;//we get just timeout, no byte
	}


	ENA_port->ODR&=(uint8_t)(~ENA_pin);
	//shifting to appropriate position
	//if (ENA_port==ENA1_port) res<<=0;//smoke
	if ((ENA_port==ENA2_port)&&(ENA_pin==ENA2_pin)) res<<=16;//light
	if ((ENA_port==ENA3_port)&&(ENA_pin==ENA3_pin)) res<<=20;//sound

	while(ENA_port->IDR & ENA_pin);//wait till pin go low

	return res;
}


//remaining bytes in buffer
uint16_t get_buf_remain(GPIO_TypeDef* ENA_port,uint8_t ENA_pin)
{
	uint16_t res;
	
	ENA_port->ODR|=(uint8_t)(ENA_pin);
	while(!(ENA_port->IDR & ENA_pin));//wait till pin go high
	USART1->DR=C_GET_PARAM;
	while(!(USART1->SR & USART_SR_TXE));//wait for tx
	USART1->DR=P_BUF_REMAIN;
	
	while(!(USART1->SR & USART_SR_RXNE));//wait for rx
	if (USART1->DR==A_OK)	{
		while(!(USART1->SR & USART_SR_RXNE));//wait for rx
		res=USART1->DR;//MSB
		res<<=8;
		while(!(USART1->SR & USART_SR_RXNE));//wait for rx
		res|=USART1->DR;//LSB
	}
	else res=0xFFFF;

	ENA_port->ODR&=(uint8_t)(~ENA_pin);//put CS low after all is received
	while(ENA_port->IDR & ENA_pin);//check if pin gone low
	return res;
}


//get head's underruns count
uint8_t get_buf_underrun(GPIO_TypeDef* ENA_port,uint8_t ENA_pin)
{
	uint8_t res;
	
	ENA_port->ODR|=(uint8_t)(ENA_pin);
	while(!(ENA_port->IDR & ENA_pin));//wait till pin go high
	USART1->DR=C_GET_PARAM;
	while(!(USART1->SR & USART_SR_TXE));//wait for tx
	USART1->DR=P_BUF_UNDERRUN;
	
	while(!(USART1->SR & USART_SR_RXNE));//wait for rx
	if (USART1->DR==A_OK)	{
		while(!(USART1->SR & USART_SR_RXNE));//wait for rx
		res=USART1->DR;
	}
	else res=0xFF;
	
	ENA_port->ODR&=(uint8_t)(~ENA_pin);//put CS low after all is received
	while(ENA_port->IDR & ENA_pin);//check if pin gone low
	return res;
}


//get current head's state
uint8_t get_state(GPIO_TypeDef* ENA_port,uint8_t ENA_pin)
{
	uint8_t res;
	
	ENA_port->ODR|=(uint8_t)(ENA_pin);
	while(!(ENA_port->IDR & ENA_pin));//wait till pin go high
	USART1->DR=C_GET_PARAM;
	while(!(USART1->SR & USART_SR_TXE));//wait for tx
	USART1->DR=P_STATE;
	
	while(!(USART1->SR & USART_SR_RXNE));//wait for rx
	if (USART1->DR==A_OK)	{
		while(!(USART1->SR & USART_SR_RXNE));//wait for rx
		res=USART1->DR;
	}
	else res=S_ERROR;
	
	ENA_port->ODR&=(uint8_t)(~ENA_pin);//put CS low after all is received
	while(ENA_port->IDR & ENA_pin);//check if pin gone low
	return res;
}


//reset head by watchdog through uart
void send_reset(GPIO_TypeDef* ENA_port,uint8_t ENA_pin)
{
	ENA_port->ODR|=(uint8_t)(ENA_pin);
	while(!(ENA_port->IDR & ENA_pin));//wait till pin go high
	USART1->SR&=(uint8_t)(~USART_SR_TC);//reset transm.compl. flag
	USART1->DR=C_RESET;
	while(!(USART1->SR & USART_SR_TC));//wait for transm.complete

	ENA_port->ODR&=(uint8_t)(~ENA_pin);//put CS low after all is received
	while(ENA_port->IDR & ENA_pin);//check if pin gone low
}


//power-on self-test
uint8_t post(void)
{
	uint16_t tmr=0;
	uint8_t temp=0,cnt=0;

	errors=0;//reset all errors
	
	//sure that all is unselected
	ena1_off();
	ena2_off();
	ena3_off();
	SD_CS_port->ODR|=(uint8_t)(SD_CS_pin);


	//sound
	cnt=0;
	send_reset(ENA3_port,ENA3_pin);
	delay_ms(150);
	
	while (head_self_test(ENA3_port,ENA3_pin)!=0) {
		sc16_log(E_SOUND);
		purge_uart(0xFF);
		purge_uart(0xFF);
		send_reset(ENA3_port,ENA3_pin);
		delay_ms(500);
	}
	//thatz enough - we r on da same pcb & vcc
	for (tmr=0;tmr<10;tmr++) errors|=head_self_test(ENA3_port,ENA3_pin);


	//ATX
	cnt=50;//5s timeout for ATX start
	power_on();
	while ((!power_is_ok())&(--cnt)) delay_ms(100);
	if (cnt==0) {
		errors|=E_ATX;//atx failed
		power_off();
		sc16_log(E_ATX);
		while(1);//stop
	}
	
	delay_ms(250);//wait while devices powers on

	
	//smoke
	cnt=0;
	do {
		errors&=(uint32_t)(~E_SMOKE);//reset flag
		errors|=head_self_test(ENA1_port,ENA1_pin);//try 1nce again
		cnt++;
		delay_ms(100);
	}
	while ((cnt<3)&&(errors & E_SMOKE));
	
	if (!(errors & E_SMOKE)) {
		for (tmr=0;tmr<256;tmr++)//we gotta smoke!
		errors|=head_self_test(ENA1_port,ENA1_pin);//let's test it hard!
	}
	else sc16_log(E_SMOKE);


	//light
	cnt=0;
	light_test:
	for (tmr=0;tmr<256;tmr++) errors|=head_self_test(ENA2_port,ENA2_pin);

	if (errors & (E_LIGHT|E_595|E_LIGHTBUF)) {//on any light error
		sc16_log(errors & (E_LIGHT|E_595|E_LIGHTBUF));
		power_off();
		delay_ms(1000);
		power_on();
		while (!power_is_ok());//wait 4 power on
		cnt++;
		if (cnt<6) {
			errors&=(uint32_t)(~(E_LIGHT|E_595|E_LIGHTBUF));
			delay_ms(200);//wait till light start
			goto light_test;
		}
	}


	//SD card
	tmr=4;//what if it'll init?
	do {
		sd_deinit();
		tmr--;
	}
	while(tmr && (temp=sd_init()));
	
	if (temp==STA_NODISK) errors|=E_SD;
	if (temp==STA_NOINIT) errors|=E_SDINIT;

	if (!(errors & E_FATALMASK)) return A_OK;
	else return A_ERROR;
}


//short post. get sure that nothing changed
uint8_t fast_post(void)
{
	uint32_t new_errors=errors;
	uint8_t temp,tmr;
	send_reset(ENA3_port,ENA3_pin);//2 clear sound buffer
	delay_ms(150);
	new_errors|=head_self_test(ENA3_port,ENA3_pin);//sound
	new_errors|=head_self_test(ENA2_port,ENA2_pin);//light
	if (!(errors & E_SMOKE)) new_errors|=head_self_test(ENA1_port,ENA1_pin);//smoke
	
	//SD
	tmr=4;//what if it'll init?
	do {
		sd_deinit();
		tmr--;
	}	while(tmr && (temp=sd_init()));
	if (temp==STA_NODISK) new_errors|=E_SD;
	if (temp==STA_NOINIT) new_errors|=E_SDINIT;

	
	if (new_errors==errors) return A_OK;
	else {
		errors=new_errors;
		return A_ERROR;
	}
	//put nothin' here. it'll never execute
}


//power on frum standby
uint8_t system_start(void)
{
	power_on();
	while (!power_is_ok());//wait 4 power on
	if (fast_post()!=A_OK) {
		power_off();
		delay_ms(1000);
		if (post()!=A_OK) return A_ERROR;
	}
	return A_OK;
}
//play from sd_addr
uint8_t play(void)
{
	uint8_t rc=0;

	//uint16_t tmp=0;//debug

	/*purge_uart(16);
	send_reset(ENA3_port,ENA3_pin);//sound
	send_reset(ENA2_port,ENA2_pin);//light
	//send_reset(ENA1_port,ENA1_pin);//smoke
	delay_ms(1500);*/

	/*tmp=get_buf_remain(ENA3_port,ENA3_pin);//debug
	rc=get_state(ENA3_port,ENA3_pin);
	tmp++; tmp--;
	tmp+=rc;*/
	if (system_start()!=A_OK) return A_ERROR;

	if (sd_multiread_start(sd_addr)!=res_OK) return A_ERROR;
	
	rc=sd_multiread_next();//read 1st time
	if (rc!=res_OK) return A_ERROR;

	ena1_off();
	ena2_off();
	ena3_off();

	//start dac clk
	TIM2_Cmd(ENABLE);
	//TIM1_Cmd(ENABLE);//to tick simultaneously

	int_counter=0;//useful!
	rc=res_OK;
	//read, read, read!
	while (rc==res_OK) {
		while(int_counter>0);	//wait till audio buffer
													//till wut? till audio buffer.
		//while(1024-get_buf_remain(ENA3_port,ENA3_pin)<400);

		rc=sd_multiread_next();
		if (!(GPIOD->IDR & GPIO_Pin_1)) {
			rc=res_EOS;
			delay_ms(100);
			while (!(GPIOD->IDR & GPIO_Pin_1));
		}
	}

	//here comez stop routines
	//sure that all ENAs r unselected
	ENA1_port->ODR&=(uint8_t)(~ENA1_pin);
	ENA2_port->ODR&=(uint8_t)(~ENA2_pin);
	ENA3_port->ODR&=(uint8_t)(~ENA3_pin);
	while(ENA1_port->ODR & ENA1_pin);
	while(ENA2_port->ODR & ENA2_pin);
	while(ENA3_port->ODR & ENA3_pin);
	
	//stop ticking
	TIM2_Cmd(DISABLE);
	//to deafault state
	PB1_on();//amp #ena to high
	PD3_off();
	PB0_off();

	if (rc==res_EOS) {
		rc=sd_multiread_stop();//end of stream happened
		return A_OK;
	}
	else return A_ERROR;//must be checked by post()
//no operators here, it'll never execute
}


//read system settings frum sector 0
uint8_t read_system_settings(void)
{
	//‚˚ÌÂÒÚË ‚ spi_sd.c?
	//all sector 0 - 512 bytes of settings!
	return A_OK;
}


//ceil lighting without sd
void ceil_light(void)
{
	uint8_t i;

	if (system_start()!=A_OK) return;

	while(1){
	ENA2_port->ODR|=(uint8_t)(ENA2_pin);
	while(!(ENA2_port->IDR & ENA2_pin));//wait till pin go high
	USART1->DR=C_SEND_BUF;
	while(!(USART1->SR & USART_SR_TXE));//wait for tx
	USART1->DR=hi(64);//one step
	while(!(USART1->SR & USART_SR_TXE));//wait for tx
	USART1->DR=lo(64);
			
	while(!(USART1->SR & USART_SR_RXNE));//wait for rx
	if (USART1->DR!=A_OK) {//light head won't get buf. wut a pity!
		sc16_log(E_LIGHTBUF);
		return;
	}

	for (i=0;i<10;i++) {//1W ceil ledz
		while(!(USART1->SR & USART_SR_TXE));//wait for tx
		USART1->DR=(uint8_t)(160+((uint8_t)(rand())/64));
	}

	for (i=10;i<64;i++) {//othr ledz
		while(!(USART1->SR & USART_SR_TXE));//wait for tx
		USART1->DR=0;
	}

	//get ACK, chip unselect
	while(!(USART1->SR & USART_SR_RXNE));//wait for rx
	if (USART1->DR!=A_OK) {//light head got buf w/errors. wut a pity!
		ENA2_port->ODR&=(uint8_t)(~ENA2_pin);//put CS low anyway
		sc16_log(E_LIGHTBUF);
		return;
	}

	ENA2_port->ODR&=(uint8_t)(~ENA2_pin);//put CS low anyway
	while(ENA2_port->IDR & ENA2_pin);//wait till pin go low
	
	delay_ms(250);
	}
}


//light testing
void demo(void)
{
	uint8_t i;
	uint8_t lbuf[light_bufsize];

	if (system_start()!=A_OK) return;

	while(1){
	for (i=0;i<64;i++) {
		lbuf[i]=(uint8_t)(i*4);
	}
	
	while(GPIOD->IDR & GPIO_Pin_1){
	ENA2_port->ODR|=(uint8_t)(ENA2_pin);
	while(!(ENA2_port->IDR & ENA2_pin));//wait till pin go high
	USART1->DR=C_SEND_BUF;
	while(!(USART1->SR & USART_SR_TXE));//wait for tx
	USART1->DR=hi(64);//one step
	while(!(USART1->SR & USART_SR_TXE));//wait for tx
	USART1->DR=lo(64);
	while(!(USART1->SR & USART_SR_RXNE));//wait for rx
	if (USART1->DR!=A_OK) {//light head won't get buf. wut a pity!
		sc16_log(E_LIGHTBUF);
		return;
	}
	for (i=0;i<64;i++) {
		while(!(USART1->SR & USART_SR_TXE));//wait for tx
		USART1->DR=lbuf[i];
		lbuf[i]++;
	}
	//get ACK, chip unselect
	while(!(USART1->SR & USART_SR_RXNE));//wait for rx
	if (USART1->DR!=A_OK) {//light head got buf w/errors. wut a pity!
		ENA2_port->ODR&=(uint8_t)(~ENA2_pin);//put CS low anyway
		sc16_log(E_LIGHTBUF);
		return;
	}
	
	while(!(USART1->SR & USART_SR_TC));//wait till transm.complete
	ENA2_port->ODR&=(uint8_t)(~ENA2_pin);//put CS low anyway
	while(ENA2_port->IDR & ENA2_pin);//wait till pin go low
	}
	
	power_off();
	
	delay_ms(100);
	while(!(GPIOD->IDR & GPIO_Pin_1));//wait4releaze
	delay_ms(100);
	
	IWDG->KR=0xCC;//enable watchdog 4 software reset
	while(1);
	}
}

//============MMMMMMAAAAAAAAAAAAIIIIIIIIINNNNNNNN===============
int main(void)
{
  volatile uint16_t i;
	uint16_t jopka=0x0190;//400
	uint8_t tmp,dt;


	GPIO_Init(GPIOD,GPIO_Pin_1,GPIO_Mode_In_PU_No_IT);//PD1 4 debug btn
	
	GPIO_Init(GPIOD,GPIO_Pin_3,GPIO_Mode_Out_PP_Low_Fast);//PD3 4 debug blinking

	GPIO_Init(GPIOB,GPIO_Pin_0,GPIO_Mode_Out_PP_Low_Fast);//PB0 4 debug blinking
	
	GPIO_Init(GPIOB,GPIO_Pin_1,GPIO_Mode_Out_PP_High_Fast);//PB1 amp #ena

	hardware_init();
	
	//delay_ms(2000);//wait before 1st start

	post();
/*	if (!(errors & E_SOUND)) PB0_on();
	else PB0_off();
	if (!(errors & E_LIGHT)) PD3_on();
	else PD3_off();*/
	
	if (errors & (E_SD|E_SDINIT)) {
		if (GPIOD->IDR & GPIO_Pin_1) ceil_light();
		else {
			delay_ms(50);
			while(!(GPIOD->IDR & GPIO_Pin_1));
			delay_ms(50);
			demo();
		}
		power_off();//start failed
		while(1);
	}
	power_off();
	

	_asm("rim");

	//main cycle
	while(1){
		while(GPIOD->IDR & GPIO_Pin_1);//wait 4 btn press
		delay_ms(100);
		while(!(GPIOD->IDR & GPIO_Pin_1));//wait 4 btn release
		sd_addr=0;
		if (play()==A_OK) {
/*			PB0_on();//green on
			PD3_off();//red off*/
		}
		else {
/*			PB0_off();//green off
			PD3_on();//red on*/
		}
		power_off();
		delay_ms(2000);
	}

/*	//DEBUUUUUUGGGGG==============================================================
	ena1_off();
	ena2_off();
	ena3_off();
	PYAZDAA:
	jopka=0;
	purge_uart(32);
	send_reset(ENA2_port,ENA2_pin);
	delay_ms(1500);
	//for(tmp=0;tmp<16;tmp++) dt=USART1->DR;
	sd_addr=0;
	tmp=1;
	while(1){
		ena2_on();
		uart_tx(tmp);
		wait_tx();
		PD3_on();
		i=6000;
		uart_rxtmt(dt,i);
		ena2_off();
		if ((i==0)||(dt!=tmp)) {
			jopka++;
			if (jopka<5) continue;//resend
			else break;//power on/off
		}
		else PB0_on();
		tmp++;
		sd_addr++;
		PD3_off();
	}
	
	PB0_off();
	//while(GPIOD->IDR & GPIO_Pin_1);
	power_off();
	delay_ms(1000);
	post();
	goto PYAZDAA;
	//\DEBUUUUUUGGGGG=============================================================
*/




	/*//DEBUUUUUUGGGGG==============================================================
	while(1){
		while(GPIOD->IDR & GPIO_Pin_1);
		delay_ms(500);
		post();
		
		
		PD3_on();
		//checking if light head ready to get its data
		if (!(errors & E_LIGHT)) {//if light head is present
			ena2_on();
			uart_tx(C_SEND_BUF);
			uart_tx(hi(64));//just one step
			uart_tx(lo(64));
			wait_tx();
			
			uart_rx(dt);
			if (dt!=A_OK) {//light head won't get buf. wut a pity!
				_asm("nop");//¡»ƒ¿!¡»ƒ¿!!!
			}
		}

		//get'n'send light buffer - 64 bytes
		for (jopka=0; jopka<64; jopka++) {
			uart_tx((uint8_t)(jopka*4));//if light head is present
		}
		//get ACK, chip unselect
		if (!(errors & E_LIGHT)) {//if light head is present
			uart_rx(dt);
			if (dt!=A_OK) {//light head got buf w/errors. wut a pity!
				ena2_off();//put CS low anyway
				_asm("nop");//¡»ƒ¿!¡»ƒ¿!!!
			}

			ena2_off();//put CS low anyway
		}
		
		PD3_off();
	}
	//\DEBUUUUUUGGGGG=============================================================

*/
	/*//MAIN CYCLE
	//wait 4 keypress
	while(1) {
		while(GPIOD->IDR & GPIO_Pin_1);
		delay_ms(500);
		post();
		sd_addr=0;
		play();
		//power_off();
	}*/
	
	return 0;
	
/*	//wait 4 keypress
	while(GPIOD->IDR & GPIO_Pin_1);
	if (sd_multiread_start(sd_addr)!=res_OK) goto lethargic;

	//fill buffer for 1st time
	if (sd_multiread_next()!=res_OK) goto lethargic;

	i=get_buf_remain(ENA3_port,ENA3_pin);//check buffered bytes

	USART1->DR=BC_START;

	//start timers
	TIM2_Cmd(ENABLE);
	TIM1_Cmd(ENABLE);//to tick simultaneously
	_asm("rim");

	while(1) {
		//wait for flag
		while(!sd_readflag);
		//check if buf have space
		GPIOD->ODR|=(uint8_t)(GPIO_Pin_3);//DEBUG
		i=get_buf_remain(ENA3_port,ENA3_pin);
		GPIOD->ODR&=(uint8_t)(~GPIO_Pin_3);//DEBUG
		if (dac_bufsize-i>SD_BLOCK_SIZE) {//yes, we have!
			i=sd_multiread_next();
			if (i!=res_OK) break;
			else sd_readflag=0;
		}
	}
		

lethargic:
	USART1->DR=BC_STOP;
	while(1);*/
	
	
	
	//while(1){
		//if (i) ENA3_port->ODR|=(uint8_t)(ENA3_pin);
		//while((GPIOD->IDR & GPIO_Pin_1));//wait for btn press
		//delay_ms(500);



		/*ENA3_port->ODR|=(uint8_t)(ENA3_pin);

		jopka=128;
		USART1->DR=C_SEND_BUF;
		while(!(USART1->SR & USART_SR_TXE));//wait for tx
		USART1->DR=hi(jopka);
		while(!(USART1->SR & USART_SR_TXE));//wait for tx
		USART1->DR=lo(jopka);
		
		while(!(USART1->SR & USART_SR_RXNE));//wait for rx
		half_buf=USART1->DR;
		
		if (half_buf==A_OK) {
			for (i=0; i<jopka; i++) {
				while(!(USART1->SR & USART_SR_TXE));//wait for tx
				USART1->DR=(uint8_t)(i);
			}
			while(!(USART1->SR & USART_SR_RXNE));//wait for rx
			half_buf=USART1->DR;
		}

		jopka=255;
		USART1->DR=C_SEND_BUF;
		while(!(USART1->SR & USART_SR_TXE));//wait for tx
		USART1->DR=hi(jopka);
		while(!(USART1->SR & USART_SR_TXE));//wait for tx
		USART1->DR=lo(jopka);
		
		while(!(USART1->SR & USART_SR_RXNE));//wait for rx
		half_buf=USART1->DR;
		
		if (half_buf==A_OK) {
			for (i=0; i<jopka; i++) {
				while(!(USART1->SR & USART_SR_TXE));//wait for tx
				USART1->DR=(uint8_t)(255-i);
			}
			while(!(USART1->SR & USART_SR_RXNE));//wait for rx
			half_buf=USART1->DR;
		}

		ENA3_port->ODR&=(uint8_t)(~ENA3_pin);
		delay_10us(1);//wait to pin low
		USART1->DR=BC_START;

		while((GPIOD->IDR & GPIO_Pin_1));//wait for btn press
		delay_ms(500);

		USART1->DR=BC_PAUSE;

		while((GPIOD->IDR & GPIO_Pin_1));//wait for btn press
		delay_ms(500);
		
		USART1->DR=BC_STOP;*/

		/*USART1->DR=C_SET_PARAM;
		while(!(USART1->SR & USART_SR_TXE));//wait for tx
		USART1->DR=P_DAC;
		while(!(USART1->SR & USART_SR_RXNE));//wait for rx
		half_buf=USART1->DR;
		if (half_buf==A_OK)	USART1->DR=lo(i);*/


		/*USART1->DR=C_GET_PARAM;
		while(!(USART1->SR & USART_SR_TXE));//wait for tx
		USART1->DR=P_DAC;
		while(!(USART1->SR & USART_SR_RXNE));//wait for rx
		half_buf=USART1->DR;
		if (half_buf==A_OK)	{
			while(!(USART1->SR & USART_SR_RXNE));//wait for rx
			half_buf=USART1->DR;
		}

		ENA3_port->ODR&=(uint8_t)(~ENA3_pin);
		USART1->DR=BC_STOP; //stop+bufreset*/
		

		/*ENA3_port->ODR|=(uint8_t)(ENA3_pin);
		
		USART1->DR=C_SEND_BUF;
		while(!(USART1->SR & USART_SR_TXE));//wait for tx
		USART1->DR=hi(jopka);
		while(!(USART1->SR & USART_SR_TXE));//wait for tx
		USART1->DR=lo(jopka);
		
		while(!(USART1->SR & USART_SR_RXNE));//wait for rx
		half_buf=USART1->DR;
		
		if (half_buf==A_OK) {
			for (i=0; i<jopka; i++) {
				while(!(USART1->SR & USART_SR_TXE));//wait for tx
				USART1->DR=(uint8_t)(i);
			}
			half_buf=USART1->DR;
		}*/
		
		/*USART1->DR=C_SELF_TEST;
		//while(!(USART1->SR & USART_SR_TXE));//wait for tx
		while(!(USART1->SR & USART_SR_RXNE));//wait for rx
		half_buf=USART1->DR;*/
		//ENA3_port->ODR&=(uint8_t)(~ENA3_pin);
//	}
	

}