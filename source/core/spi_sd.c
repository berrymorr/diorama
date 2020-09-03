/*
  ******************************************************************************
  * @file    spi_sd.c
  * @author  berrymorr
  * @version v0.1
  ******************************************************************************
*/
#include "spi_sd.h"
#include "..\globals.h"
#include "sc16.h"

extern volatile uint32_t DEBUGVAR;

//disable sd's spi and all spi's pins
void sd_deinit(void)
{
  SPI_Cmd(SD_SPI, DISABLE);
  CLK_PeripheralClockConfig(SD_SPI_CLK, DISABLE);
  GPIO_Init(SD_SPI_SCK_GPIO_PORT, SD_SPI_SCK_PIN, GPIO_Mode_In_FL_No_IT);
  GPIO_Init(SD_SPI_MISO_GPIO_PORT, SD_SPI_MISO_PIN, GPIO_Mode_In_FL_No_IT);
  GPIO_Init(SD_SPI_MOSI_GPIO_PORT, SD_SPI_MOSI_PIN, GPIO_Mode_In_FL_No_IT);
  GPIO_Init(SD_CS_GPIO_PORT, SD_CS_PIN, GPIO_Mode_In_FL_No_IT);
	GPIO_Init(SD_CD_GPIO_PORT, SD_CD_PIN, GPIO_Mode_In_FL_No_IT);
}

//enable sd's spi and all spi's pins
void sd_spi_init(void)
{
  CLK_PeripheralClockConfig(SD_SPI_CLK, ENABLE);
	GPIO_Init(SD_SPI_SCK_GPIO_PORT,SD_SPI_SCK_PIN,GPIO_Mode_Out_PP_High_Fast);
	GPIO_Init(SD_SPI_MISO_GPIO_PORT,SD_SPI_MISO_PIN,GPIO_Mode_In_PU_No_IT);
	GPIO_Init(SD_SPI_MOSI_GPIO_PORT,SD_SPI_MOSI_PIN,GPIO_Mode_Out_PP_High_Fast);
	GPIO_Init(SD_CD_GPIO_PORT,SD_CD_PIN,GPIO_Mode_In_PU_No_IT);
	GPIO_Init(SD_CS_GPIO_PORT, SD_CS_PIN, GPIO_Mode_Out_PP_High_Fast);
	//init at 100..400 kHz first for switching card to spi mode Prescaler_64
	SPI_Init(SD_SPI, SPI_FirstBit_MSB, SPI_BaudRatePrescaler_64, SPI_Mode_Master,
           SPI_CPOL_High, SPI_CPHA_2Edge, SPI_Direction_2Lines_FullDuplex,
           SPI_NSS_Soft, 0x07);
  SPI_Cmd(SD_SPI, ENABLE);
}

//init sd card in spi mode and force blocksize to 512
uint8_t sd_init(void)
{
	uint8_t n=0, cmd=0, ty=0, ocr[4]={0};
  uint16_t tmr=0;

	sd_spi_init();
	
	if (SD_CD_GPIO_PORT->IDR & SD_CD_PIN) return STA_NODISK;
	
	//========put to spi mode
	sd_cs_high();
	//send dummy byte 0xFF (MOSI=high for 80 clocks cycles)
  for (tmr = 10; tmr; tmr--) sd_txrx_byte(SD_DUMMY_BYTE);
  sd_cs_low();
	for (tmr = 600; tmr; tmr--) sd_txrx_byte(SD_DUMMY_BYTE);
	
  if (sd_sendcmd(CMD(0),0) == 1) {      //Enter Idle state
    if (sd_sendcmd(CMD(8), 0x1AA) == 1) {  //SDv2
      for (n = 0; n < 4; n++) ocr[n] = sd_txrx_byte(SD_DUMMY_BYTE);//Get trailing return value of R7 resp
      if (ocr[2] == 0x01 && ocr[3] == 0xAA) {//The card can work at vdd range of 2.7-3.6V
        for (tmr = 25000; tmr && sd_sendcmd(ACMD(41), 1UL << 30); tmr--);//Wait for leaving idle state (ACMD41 with HCS bit)
        if (tmr && sd_sendcmd(CMD(58), 0) == 0) {//Check CCS bit in the OCR
          for (n = 0; n < 4; n++) ocr[n] = sd_txrx_byte(SD_DUMMY_BYTE);
          ty = (uint8_t)((ocr[0] & 0x40) ? CT_SD2 | CT_BLOCK : CT_SD2);  //SDv2
        }
      }
    } else {              //SDv1 or MMC
      if (sd_sendcmd(ACMD(41), 0) <= 1)   {
        ty = CT_SD1; cmd = ACMD(41);  //SDv1
      } else {
        ty = CT_MMC; cmd = CMD(1);  //MMCv3
      }
      for (tmr = 25000; tmr && sd_sendcmd(cmd, 0); tmr--) ;  //Wait for leaving idle state
      if (!tmr || sd_sendcmd(CMD(16),SD_BLOCK_SIZE) != 0) { //Set R/W block length to 512
        ty = 0;
      }
    }
  }
  CardType = ty;
  //release_spi();

	SPI1->CR1&=0b11000111;//reset spi prescaler to 2
	return (uint8_t)(ty ? 0 : (uint8_t)(STA_NOINIT));
}

//read 512 bytes sector to buf
uint8_t sd_readblock(uint8_t* pbuf,uint32_t plba)
{
	uint32_t tmr=0, lba=plba;
	uint8_t res=res_ERROR,rc=0;
	
	if (!(CardType & CT_BLOCK)) lba *=SD_BLOCK_SIZE;//
  if (sd_sendcmd(CMD(17), lba) == 0) {  //CMD17 - READ_SINGLE_BLOCK
    tmr = 30000;
    do {              //Wait for data packet in timeout of 100ms
      rc = sd_txrx_byte(SD_DUMMY_BYTE);
    } while (rc == 0xFF && --tmr);
    if (rc == 0xFE) {
      for (tmr = 0; tmr < SD_BLOCK_SIZE; tmr++) {
				//get data
				*pbuf = sd_txrx_byte(SD_DUMMY_BYTE);
				pbuf++;
			}
			sd_txrx_byte(SD_DUMMY_BYTE);//get 2 bytes of crc into /dev/null
			sd_txrx_byte(SD_DUMMY_BYTE);
      res = res_OK;
    }
  }
return res;
}


//start multi block read 2 buf
uint8_t sd_multiread_start(uint32_t plba)
{
	extern uint32_t sd_addr;

	uint32_t tmr=0, lba=sd_addr;
	uint8_t res=res_ERROR;

	if (!(CardType & CT_BLOCK)) lba *=SD_BLOCK_SIZE;
  if (sd_sendcmd(CMD(18), lba) == 0) res=res_OK;
	return res;
}

	

//multi block read 2 flat buffer
uint8_t sd_multiread_next(void)
{
	extern volatile uint32_t sd_addr;
	extern volatile uint32_t errors;
	uint32_t tmr=30000;
	uint8_t res=res_ERROR,rc=0;
	
	volatile uint32_t vu32=0;
	uint16_t vu16=0;
	
	//DEBUG
	//DEBUGVAR++;
	
  do {              //Wait for data packet in timeout of 100ms
    rc = sd_txrx_byte(SD_DUMMY_BYTE);
  } while (rc == 0xFF && --tmr);
  if (rc == 0xFE) {//reading one sector
		
		//read frame's head - 4 bytes
		for (tmr=0; tmr<4; tmr++) {
			vu32<<=8;
			vu32|=(uint32_t)(sd_txrx_byte(SD_DUMMY_BYTE));
		}
		
		//end of stream label
		if (vu32 == 0xFFFFFFFF) return res_EOS;
		
		if (vu32!=frame_head) {
			sc16_log(E_SDFORMAT);
			return res_ERROR;
		}

		//===========AUDIO===========
		//checking if audio head ready to get its data.
		//don't check if audio head is ok cauze it's fatal error,
		//which'll fail post()
		ENA3_port->ODR|=(uint8_t)(ENA3_pin);
		while(!(ENA3_port->IDR & ENA3_pin));//wait till pin go high
		USART1->DR=C_SEND_BUF;
		while(!(USART1->SR & USART_SR_TXE));//wait for tx
		USART1->DR=hi(400);
		while(!(USART1->SR & USART_SR_TXE));//wait for tx
		USART1->DR=lo(400);
		
		while(!(USART1->SR & USART_SR_RXNE));//wait for rx
		if (USART1->DR!=A_OK) {//audio head won't get buf. wut a pity!
			sc16_log(E_SOUNDBUF);
			return res_ERROR;
		}

		//get'n'send audio buffer - 400 bytes
		for (tmr=0; tmr<400; tmr++) {
			while(!(USART1->SR & USART_SR_TXE));//wait for tx
			USART1->DR=sd_txrx_byte(SD_DUMMY_BYTE);
		}
		//get ACK, chip unselect
		while(!(USART1->SR & USART_SR_RXNE));//wait for rx
		if (USART1->DR!=A_OK) {//audio head got buf w/errors. wut a pity!
			ENA3_port->ODR&=(uint8_t)(~ENA3_pin);//put CS low anyway
			while(ENA3_port->IDR & ENA3_pin);//wait till pin go low
			sc16_log(E_SOUNDBUF);
			return res_ERROR;
		}
		
		ENA3_port->ODR&=(uint8_t)(~ENA3_pin);//put CS low anyway
		while(ENA3_port->IDR & ENA3_pin);//wait till pin go low




		//===========LIGHT===========
		//checking if light head ready to get its data
		if (!(errors & E_LIGHT)) {//if light head is present
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
				return res_ERROR;
			}
		}

		//get'n'send light buffer - 64 bytes
		for (tmr=0; tmr<64; tmr++) {
			while(!(USART1->SR & USART_SR_TXE));//wait for tx
			rc=sd_txrx_byte(SD_DUMMY_BYTE);
			USART1->DR=rc;//if light head is present
		}
		//get ACK, chip unselect
		if (!(errors & E_LIGHT)) {//if light head is present
			while(!(USART1->SR & USART_SR_RXNE));//wait for rx
			if (USART1->DR!=A_OK) {//light head got buf w/errors. wut a pity!
				ENA2_port->ODR&=(uint8_t)(~ENA2_pin);//put CS low anyway
				sc16_log(E_LIGHTBUF);
				return res_ERROR;
			}

			ENA2_port->ODR&=(uint8_t)(~ENA2_pin);//put CS low anyway
			while(ENA2_port->IDR & ENA2_pin);//wait till pin go low
		}



		//===========SMOKE===========
		//checking if smoke head ready to get its data
		if (!(errors & E_SMOKE)) {//if smoke head is present
			ENA1_port->ODR|=(uint8_t)(ENA1_pin);
			while(!(ENA1_port->IDR & ENA1_pin));//wait till pin go high
			USART1->DR=C_SEND_BUF;
			while(!(USART1->SR & USART_SR_TXE));//wait for tx
			USART1->DR=hi(8);//one step
			while(!(USART1->SR & USART_SR_TXE));//wait for tx
			USART1->DR=lo(8);
			
			while(!(USART1->SR & USART_SR_RXNE));//wait for rx
			if (USART1->DR!=A_OK) {//smoke head won't get buf. wut a pity!
				sc16_log(E_SMOKEBUF);
				return res_ERROR;
			}
		}

		//get'n'send smoke buffer - 8 bytes
		for (tmr=0; tmr<8; tmr++) {
			while(!(USART1->SR & USART_SR_TXE));//wait for tx
			rc=sd_txrx_byte(SD_DUMMY_BYTE);
			USART1->DR=rc;//if smoke head is present
		}
		//get ACK, chip unselect
		if (!(errors & E_SMOKE)) {//if smoke head is present
			while(!(USART1->SR & USART_SR_RXNE));//wait for rx
			if (USART1->DR!=A_OK) {//smoke head got buf w/errors. wut a pity!
				ENA1_port->ODR&=(uint8_t)(~ENA1_pin);//put CS low anyway
				sc16_log(E_SMOKEBUF);
				return res_ERROR;
			}

			ENA1_port->ODR&=(uint8_t)(~ENA1_pin);//put CS low anyway
			while(ENA1_port->IDR & ENA1_pin);//wait till pin go low
		}

		//get 1 byte for PD3 and PB0
		rc=sd_txrx_byte(SD_DUMMY_BYTE);
		if (rc & 0x01) PD3_on();
		else PD3_off();

		if (rc & 0x02) PB0_on();
		else PB0_off();
		
		if (rc & 0x04) PB1_off();//amp #ena to low
		else PB1_on();
		
		//get 33 reserved bytes, don't care wut is it
		for (tmr=0; tmr<33; tmr++) sd_txrx_byte(SD_DUMMY_BYTE);


		//read frame's tail - 2 bytes
		vu16=(uint16_t)(sd_txrx_byte(SD_DUMMY_BYTE)<<8);
		vu16|=(uint16_t)sd_txrx_byte(SD_DUMMY_BYTE);

		if (vu16!=frame_tail) {
			sc16_log(E_SDFORMAT);
			return res_ERROR;
		}

		//get 2 bytes of crc into /dev/null
		sd_txrx_byte(SD_DUMMY_BYTE);
		sd_txrx_byte(SD_DUMMY_BYTE);
		
		//sector successfully readed!
		if (!(CardType & CT_BLOCK)) sd_addr+=SD_BLOCK_SIZE;
		else sd_addr++;
		return res_OK;
  }

return res_NOTRDY;//card didn't return data token
}


uint8_t sd_multiread_stop(void)
{
	extern uint16_t bi_out,bi_in;
	extern uint8_t sd_buf[sd_bufsize];
	extern uint32_t sd_addr;

	uint32_t tmr=0;
	uint8_t rc=0,n;

	//send CMD12
	sd_cs_high(); //deselect card
	//GPIOC->ODR&=(uint8_t)(~GPIO_Pin_7);
	sd_txrx_byte(SD_DUMMY_BYTE);
	sd_cs_low();  //select card
	sd_txrx_byte(SD_DUMMY_BYTE);
	
	//send command and argument
  sd_txrx_byte(CMD(12));
	sd_txrx_byte(0);
	sd_txrx_byte(0);
	sd_txrx_byte(0);
	sd_txrx_byte(0);
	
	//send fake crc
	sd_txrx_byte(1);
	
	//get stuff byte
	sd_txrx_byte(SD_DUMMY_BYTE);
	
	//wait'n'get response
	n=10;
	do
	{
		rc=sd_txrx_byte(SD_DUMMY_BYTE);
	}
	while ((rc & 0x80) && --n);


	tmr = 30000;
  do {              //Wait till busy
    rc = sd_txrx_byte(SD_DUMMY_BYTE);
  } while (rc != 0xFF && --tmr);
	
	if (!tmr) return res_ERROR;//timeout waiting till busy
	else return res_OK;
}


//Send command to the SD card
uint8_t sd_sendcmd(uint8_t cmd, uint32_t arg)
{
	uint8_t n,res;
	if (cmd&0x80) //work with ACMD
	{
		cmd&=0x7F;
		res=sd_sendcmd(CMD(55),0);
		if (res>1) return res;
	}
	
	//select card and wait for ready
	sd_cs_high(); //deselect card
	//GPIOC->ODR&=(uint8_t)(~GPIO_Pin_7);
	sd_txrx_byte(SD_DUMMY_BYTE);
	sd_cs_low();  //select card
	sd_txrx_byte(SD_DUMMY_BYTE);
	
	//send command and argument
  sd_txrx_byte(cmd);
	sd_txrx_byte((uint8_t)(arg >> 24));
	sd_txrx_byte((uint8_t)(arg >> 16));
	sd_txrx_byte((uint8_t)(arg >> 8));
	sd_txrx_byte((uint8_t)(arg));
	
	//send crc
	n=1;
	if (cmd==CMD(0)) n=0x95;	//valid CRC for CMD0
	if (cmd==CMD(8)) n=0x87;	//valid CRC for CMD8(0x1AA)
	sd_txrx_byte(n);
	
	//get response
	n=10;
	do
	{
		res=sd_txrx_byte(SD_DUMMY_BYTE);
	}
	while ((res & 0x80) && --n);
	return res;
}


//send a byte to sd
uint8_t sd_txrx_byte(uint8_t data)
{
  //Wait until the transmit buffer is empty
	while (!(SD_SPI->SR&(uint8_t)SPI_FLAG_TXE)) {
	}
  SD_SPI->DR=data;
	
	//Wait to receive a byte
	while (!(SD_SPI->SR&(uint8_t)SPI_FLAG_RXNE)) {
	}
	//Return the byte read from the SPI bus
	return SD_SPI->DR;
}
