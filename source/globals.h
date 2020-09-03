#ifndef __GLOBALS_H
#define __GLOBALS_H


//should use only on STM8!
#define hi(Val) ((uint8_t)(Val>>8))
#define lo(Val) ((uint8_t)(Val))
//remember of byte order!


#include "stm8l15x.h"

#define dac_bufsize ((uint16_t)(1024))
#define dac_bufmask ((uint16_t)(dac_bufsize-1))
#define dac_bufremain ((uint16_t)((bi_in-bi_out)&dac_bufmask))

#define sd_bufsize ((uint16_t)(512))
#define sd_bufmask ((uint16_t)(dac_bufsize-1))
#define sd_bufremain ((uint16_t)((bi_in-bi_out)&sd_bufmask))

#define light_bufsize ((uint8_t)(64))
#define light_bufmask ((uint8_t)(light_bufsize-1))
#define light_bufremain ((uint8_t)((bi_in-bi_out)&dac_bufmask))


#define uart_baud (uint32_t)(460800)//(921600)

//commands
#define C_SELF_TEST			0xAA		//no args. return OK if passed, else ERROR
#define C_RESET					0xEE		//no args. return OK if passed
#define C_SEND_BUF			0x11		//uint16_t buf_size, then – return OK if ready, then – data, then – return OK or ERROR
#define C_SET_PARAM			0x12
#define C_GET_PARAM			0x13
#define C_LIGHT_OFF			0x14

//acks
#define A_OK						0x55
#define A_ERROR					0xFF

//params
#define P_DAC						0x01		//DAC current value
#define P_STATE					0x02		//return only
#define P_BUF_REMAIN		0x03		//return only, set through C_SEND_BUF
#define P_BUF_UNDERRUN	0x04		//
#define P_LIGHT_CH			0x05		//channel value

//states
#define S_PWRON					0x00
#define S_RESET					0x01
#define S_STOP					0x02
#define S_PLAY					0x03
#define S_ERROR					A_ERROR

//errors
#define E_NOERROR				(uint32_t)(0x00000000)	//we're happy, Vincent
#define E_UNKNOWN				(uint32_t)(0xFFFFFFFF)	//apocalypse happend
#define E_FATALMASK			(uint32_t)(0x00131300)	//snd, any light, atx, any sd

#define E_SOUND					(uint32_t)(0x00100000)	//no response of sound subsystem
	#define E_SOUNDBUF		(uint32_t)(0x00200000)	//error accepting buffer

#define E_LIGHT					(uint32_t)(0x00010000)	//no response of light subsystem
	#define E_595					(uint32_t)(0x00020000)	//error in shift registers chain
	#define E_LIGHTBUF		(uint32_t)(0x00040000)	//error accepting buffer

#define E_ATX						(uint32_t)(0x00001000) //no PWOK when #PWON

#define E_SD						(uint32_t)(0x00000100) //no card
	#define E_SDINIT			(uint32_t)(0x00000200) //can not init card
	#define E_SDFORMAT		(uint32_t)(0x00000400) //bad data reading
	
#define E_SMOKE					(uint32_t)(0x00000001)	//no response of smoke subsystem
	#define E_HEATER			(uint32_t)(0x00000002)	//incorrect heater temp
	#define E_AMB_TEMP		(uint32_t)(0x00000004)	//incorrect ambient temp
	#define E_LO_LIQUID		(uint32_t)(0x00000008)	//low smoke liquid level in tank
	#define E_PUMP				(uint32_t)(0x00000010)	//low smoke liquid level in heater (pump malfunction?)
	#define E_SMOKEBUF		(uint32_t)(0x00000020)	//error accepting buffer
	#define E_SMOKERSRVD	(uint32_t)(0x00000040)	//never uz 2 prevent matching 0x55

#endif //__GLOBALS_H