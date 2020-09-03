

//controlling 74HC595 through SPI

#include "stm8l15x_spi.h"
#include "spi_595.h"
#include "stm8l15x.h"
#include "..\globals.h"
#include "light_mappings.h"

#include "..\delay.h"


//send a byte to 595 ring
uint8_t spi_txrx_byte(uint8_t data)
{
  //Wait until the transmit buffer is empty
  while (!(SPI_595->SR&(uint8_t)SPI_FLAG_TXE)) {
  }
  SPI_595->DR=data;

  //Wait to receive a byte
  while (!(SPI_595->SR&(uint8_t)SPI_FLAG_RXNE)) {
  }
  //Return the byte read from the SPI bus
  return SPI_595->DR;
}

void send_595_ring(uint8_t arr[light_bufsize/8])
{
	uint8_t q;

	for (q=0; q<(light_bufsize/8); q++) {
		while(!(SPI_595->SR&SPI_FLAG_TXE));
		SPI_595->DR=arr[q];
	}
	
	while(SPI_595->SR&SPI_FLAG_BSY);
	latch_595_ring();
}

void reset_595_ring(void)
{
     RST_port->ODR&=(uint8_t)(~RST_pin);//put it to reset
     RST_port->ODR|=(uint8_t)(RST_pin);//release reset
}
