/**
  ******************************************************************************
  * @file    spi_sd.h
  * @author  berrymorr
  * @version v0.1
  * @brief   This file contains all the functions prototypes for the stm8_eval_spi_sd
  *          firmware driver.
  *          You can easily tailor this driver to any other development board,
  *          by just adapting the defines for hardware resources and
  *          SD_LowLevel_Init() function.
  *
  *          +-------------------------------------------------------+
  *          |                     Pin assignment                    |
  *          +-------------------------+---------------+-------------+
  *          |  STM8 SPI Pins         |     SD        |    Pin      |
  *          +-------------------------+---------------+-------------+
  *          | SD_SPI_CS_PIN           |   ChipSelect  |    1        |
  *          | SD_SPI_MOSI_PIN / MOSI  |   DataIn      |    2        |
  *          |                         |   GND         |    3 (0 V)  |
  *          |                         |   VDD         |    4 (3.3 V)|
  *          | SD_SPI_SCK_PIN / SCLK   |   Clock       |    5        |
  *          |                         |   GND         |    6 (0 V)  |
  *          | SD_SPI_MISO_PIN / MISO  |   DataOut     |    7        |
  *          +-------------------------+---------------+-------------+
	******************************************************************************
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __SPI_SD_H
#define __SPI_SD_H

/* Includes ------------------------------------------------------------------*/
#include "stm8l15x.h"
#include "core_mappings.h"

/**
  * @brief  SD SPI Interface pins
  */
#define SD_SPI														SPI1
#define SD_SPI_CLK												CLK_Peripheral_SPI1

#define SD_SPI_SCK_PIN										SD_CLK_pin
#define SD_SPI_SCK_GPIO_PORT							SD_CLK_port

#define SD_SPI_MISO_PIN										SD_DO_pin
#define SD_SPI_MISO_GPIO_PORT							SD_DO_port

#define SD_SPI_MOSI_PIN										SD_DI_pin
#define SD_SPI_MOSI_GPIO_PORT							SD_DI_port

#define SD_CS_PIN													SD_CS_pin
#define SD_CS_GPIO_PORT										SD_CS_port

#define SD_CD_PIN													SD_Sb_pin
#define SD_CD_GPIO_PORT										SD_Sb_port


typedef enum {
  res_OK = 0,			//0: Function succeeded
  res_ERROR,			//1: Disk error
  res_STRERR,			//2: Seream error
  res_NOTRDY,			//3: Not ready
  res_PARERR,			//4: Invalid parameter
	res_EOS,				//5: end of stream
	res_HDENY				//6: some head don't wanna accept buffer
} DRESULT;

#define STA_NOINIT    (uint8_t)0x01  /* Drive not initialized */
#define STA_NODISK    (uint8_t)0x02  /* No medium in the drive */

/* Card type flags (CardType) */
#define CT_MMC        (uint8_t)0x01  /* MMC ver 3 */
#define CT_SD1        (uint8_t)0x02  /* SD ver 1 */
#define CT_SD2        (uint8_t)0x04  /* SD ver 2 */
#define CT_SDC        (CT_SD1|CT_SD2)  /* SD */
#define CT_BLOCK      (uint8_t)0x08  /* Block addressing */


#define frame_head			(uint32_t)(0x3355AACC)
#define frame_tail			(uint16_t)(0xCCAA)


#define SD_BLOCK_SIZE	0x200	//never, NEVER CHANGE IT!

#define SD_DUMMY_BYTE	0xFF	//never, NEVER CHANGE IT!

#define CMD(a) a+0x40
#define ACMD(b) b+0xC0

/*
CMD0		GO_IDLE_STATE
CMD1		SEND_OP_COND
CMD8		SEND_IF_COND
CMD16		SET_BLOCKLEN
CMD17		READ_SINGLE_BLOCK
CMD55		APP_CMD
CMD58		READ_OCR
ACMD41	SEND_OP_COND (SD Card)
*/

//Select SD Card: ChipSelect pin low
#define sd_cs_low()     SD_CS_GPIO_PORT->ODR &= (uint8_t)(~SD_CS_PIN)

//Deselect SD Card: ChipSelect pin high
#define sd_cs_high()    SD_CS_GPIO_PORT->ODR |= (uint8_t)(SD_CS_PIN)


extern uint8_t CardType;


void sd_deinit(void);
void sd_spi_init(void);
uint8_t sd_init(void);
uint8_t sd_readblock(uint8_t* pbuf,uint32_t lba);
uint8_t sd_multiread_start(uint32_t plba);
uint8_t sd_multiread_next(void);
uint8_t sd_multiread_stop(void);
uint8_t sd_sendcmd(uint8_t cmd, uint32_t arg);
uint8_t sd_txrx_byte(uint8_t data);


#endif
