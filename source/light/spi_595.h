#ifndef __SPI_595_H
#define __SPI_595_H

//controlling 74HC595 through SPI

#include "stm8l15x.h"
#include "..\globals.h"
#include "light_mappings.h"

uint8_t spi_txrx_byte(uint8_t data);
void send_595_ring(uint8_t arr[light_bufsize/8]);

#define enable_595_ring() OE_port->ODR&=(uint8_t)(~OE_pin)
#define disable_595_ring() OE_port->ODR|=(uint8_t)(OE_pin)
#define latch_595_ring() ST_clk_port->ODR|=(uint8_t)(ST_clk_pin); _asm("nop"); ST_clk_port->ODR&=(uint8_t)(~ST_clk_pin)
#define reset_595_ring() RST_port->ODR&=(uint8_t)(~RST_pin); latch_595_ring(); RST_port->ODR|=(uint8_t)(RST_pin)

#endif /* __SPI_595_H */
