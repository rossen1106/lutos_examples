/**
 * @file s24_eeprom.c
 * @brief 24xx I2C interface EEPROM Read/Write Module
 *
 * 24 series I2C interface EEPROM Read/Write Library.
 * 
 * Configurable definitions:
 *  # I2C_PORT_NUMBER		Which I2C port to use
 *  # EEPROM_ADDRESS_TOP	EEPROM's highest available address
 *  # EEPROM_CACHE_SIZE		EEPROM's internal cache size (byte), usually equal to page size
 *  # CTRL_CODE				EEPROM's control code
 *  # CHIP_SELECT			EEPROM's chip select address
 *  # WP_PORT				GPIO port number of Write protection control pin
 *  # WP_PIN				GPIO pin number of Write protection control pin
 *
 * @author Li-Yeh Liu <rosssen@viewmove.com>
 * @version 1.0.0
 * @date 2013/12/19
 */

#include <LPC17xx.h>
#include "gpio.h"
#include "i2c_m.h"
#include "s24_eeprom.h"

#define I2C_PORT_NUMBER			0

#define EEPROM_ADDRESS_TOP		0xFFF
#define EEPROM_CACHE_SIZE		32

#define CTRL_CODE		0xA0
#define CHIP_SELECT		0x00
#define WP_PORT			3
#define WP_PIN			26

#define SLA_WR			(CTRL_CODE | CHIP_SELECT)
#define SLA_RD			(CTRL_CODE | CHIP_SELECT | 0x01)
#define WP_ENABLE()		gpio_write(WP_PORT, WP_PIN, 1);
#define WP_DISABLE()	gpio_write(WP_PORT, WP_PIN, 0);

/**
 * @brief Initialize Function
 *
 */
void
s24_eeprom_init(void)
{
	i2c_init(I2C_PORT_NUMBER, 100000);
	gpio_config(WP_PORT, WP_PIN, GPIO_OUT);
	WP_ENABLE();
	eeprom_is_ready();	// sample the WP=1 pin state
}

uint8_t eeprom_is_ready(void)
{
	uint8_t ret = 0;
	i2c_send_start(I2C_PORT_NUMBER);
	if( RET_ACK == i2c_send_SLA(I2C_PORT_NUMBER,SLA_WR) ) ret = 1;
	i2c_send_stop(I2C_PORT_NUMBER);
	return ret;
}

uint8_t eeprom_read_byte(uint32_t address)
{
	uint8_t val;
	i2c_send_start(I2C_PORT_NUMBER);
	if( RET_ACK == i2c_send_SLA(I2C_PORT_NUMBER,SLA_WR) )
	{
		#if EEPROM_ADDRESS_TOP > 0xFF
		i2c_send_data(I2C_PORT_NUMBER,(address & EEPROM_ADDRESS_TOP)>>8);
		#endif
		i2c_send_data(I2C_PORT_NUMBER,(address)&0xFF);
		i2c_repeat_start(I2C_PORT_NUMBER);
		i2c_send_SLA(I2C_PORT_NUMBER,SLA_RD);
		val = i2c_get_data_with_nak(I2C_PORT_NUMBER);
	}
	i2c_send_stop(I2C_PORT_NUMBER);
	return val;
}

uint16_t eeprom_read_word(uint32_t address)
{
	uint16_t val, tmp;
	i2c_send_start(I2C_PORT_NUMBER);
	if( RET_ACK == i2c_send_SLA(I2C_PORT_NUMBER,SLA_WR) )
	{
		#if EEPROM_ADDRESS_TOP > 0xFF
		i2c_send_data(I2C_PORT_NUMBER,(address & EEPROM_ADDRESS_TOP)>>8);
		#endif
		i2c_send_data(I2C_PORT_NUMBER,(address)&0xFF);
		i2c_repeat_start(I2C_PORT_NUMBER);
		i2c_send_SLA(I2C_PORT_NUMBER,SLA_RD);
		val = i2c_get_data_with_ack(I2C_PORT_NUMBER);
		tmp = i2c_get_data_with_nak(I2C_PORT_NUMBER);
		val |= (tmp<<8);
	}
	i2c_send_stop(I2C_PORT_NUMBER);
	return val;
}

uint32_t eeprom_read_dword(uint32_t address)
{
	uint32_t val, tmp;
	i2c_send_start(I2C_PORT_NUMBER);
	if( RET_ACK == i2c_send_SLA(I2C_PORT_NUMBER,SLA_WR) )
	{
		#if EEPROM_ADDRESS_TOP > 0xFF
		i2c_send_data(I2C_PORT_NUMBER,(address & EEPROM_ADDRESS_TOP)>>8);
		#endif
		i2c_send_data(I2C_PORT_NUMBER,(address)&0xFF);
		i2c_repeat_start(I2C_PORT_NUMBER);
		i2c_send_SLA(I2C_PORT_NUMBER,SLA_RD);
		val = i2c_get_data_with_ack(I2C_PORT_NUMBER);
		tmp = i2c_get_data_with_ack(I2C_PORT_NUMBER);
		val |= (tmp<<8);
		tmp = i2c_get_data_with_ack(I2C_PORT_NUMBER);
		val |= (tmp<<16);
		tmp = i2c_get_data_with_nak(I2C_PORT_NUMBER);
		val |= (tmp<<24);
	}
	i2c_send_stop(I2C_PORT_NUMBER);
	return val;
}

uint16_t eeprom_read_block(uint32_t address, void *dst, uint16_t size)
{
	uint8_t *val=dst;
	uint16_t i, len=(size-1);
	i2c_send_start(I2C_PORT_NUMBER);
	if( (RET_ACK == i2c_send_SLA(I2C_PORT_NUMBER,SLA_WR)) && (size > 0) )
	{
		#if EEPROM_ADDRESS_TOP > 0xFF
		i2c_send_data(I2C_PORT_NUMBER,(address & EEPROM_ADDRESS_TOP)>>8);
		#endif
		i2c_send_data(I2C_PORT_NUMBER,(address)&0xFF);
		i2c_repeat_start(I2C_PORT_NUMBER);
		i2c_send_SLA(I2C_PORT_NUMBER,SLA_RD);
		for(i=0; i<len; i++)
		{
			val[i] = i2c_get_data_with_ack(I2C_PORT_NUMBER);
		}
		val[i++] = i2c_get_data_with_nak(I2C_PORT_NUMBER);
	}
	i2c_send_stop(I2C_PORT_NUMBER);
	return i;
}

void eeprom_write_byte(uint32_t address, uint8_t value)
{
	WP_ENABLE();
	i2c_send_start(I2C_PORT_NUMBER);
	if( RET_ACK == i2c_send_SLA(I2C_PORT_NUMBER,SLA_WR) )
	{
		#if EEPROM_ADDRESS_TOP > 0xFF
		i2c_send_data(I2C_PORT_NUMBER,(address & EEPROM_ADDRESS_TOP)>>8);
		#endif
		i2c_send_data(I2C_PORT_NUMBER,(address)&0xFF);
		i2c_send_data(I2C_PORT_NUMBER,value);
	}
	WP_DISABLE();		// WP=0
	i2c_send_stop(I2C_PORT_NUMBER);
	eeprom_is_ready();	// delay for proper latch of the WP=0 pin state during the stop condition of last write cycle
	WP_ENABLE();		// WP=1
	eeprom_is_ready();	// sample the WP=1 pin state
}

uint8_t eeprom_write_page(uint32_t address, void *src, uint8_t size)
{
	uint8_t i=0, *value=src;
	i2c_send_start(I2C_PORT_NUMBER);
	if( RET_ACK == i2c_send_SLA(I2C_PORT_NUMBER,SLA_WR) )
	{
		#if EEPROM_ADDRESS_TOP > 0xFF
		i2c_send_data(I2C_PORT_NUMBER,(address & EEPROM_ADDRESS_TOP)>>8);
		#endif
		i2c_send_data(I2C_PORT_NUMBER,(address)&0xFF);
		for(i=0; i<size; i++)
		{
			i2c_send_data(I2C_PORT_NUMBER,value[i]);
		}
	}
	WP_DISABLE();		// WP=0
	i2c_send_stop(I2C_PORT_NUMBER);
	eeprom_is_ready();	// delay for proper latch of the WP=0 pin state during the stop condition of last write cycle
	WP_ENABLE();		// WP=1
	eeprom_is_ready();	// sample the WP=1 pin state
	return i;
}

uint16_t eeprom_write_block(uint32_t address, void *src, uint16_t size)
{
	uint8_t len, *data=src;
	uint16_t cnt=0, rest=size;
	uint32_t sa=address, ta;
	while( rest > 0 )
	{
		ta = (sa & (~(EEPROM_CACHE_SIZE-1))) + EEPROM_CACHE_SIZE;
		len = ta - sa;
		if( len > rest ) len = rest;
		eeprom_busy_wait();
		cnt += eeprom_write_page(sa, data, len);
		rest -= len;
		sa += len;
		data += len;
	}
	return cnt;
}

void eeprom_write_word(uint32_t address, uint16_t value)
{
	eeprom_write_block(address, &value, 2);
	/*WP_ENABLE();
	i2c_send_start();
	if( RET_ACK == i2c_send_SLA(SLA_WR) )
	{
		i2c_send_data((address>>8)&0x0F);
		i2c_send_data((address)&0x0F);
		i2c_send_data(value&0xFF);
		i2c_send_data((value>>8)&0xFF);
	}
	i2c_send_stop();
	WP_DISABLE();*/
}

void eeprom_write_dword(uint32_t address, uint32_t value)
{
	eeprom_write_block(address, &value, 4);
	/*WP_ENABLE();
	i2c_send_start();
	if( RET_ACK == i2c_send_SLA(SLA_WR) )
	{
		i2c_send_data((address>>8)&0x0F);
		i2c_send_data((address)&0x0F);
		i2c_send_data(value&0xFF);
		i2c_send_data((value>>8)&0xFF);
		i2c_send_data((value>>16)&0xFF);
		i2c_send_data((value>>24)&0xFF);
	}
	i2c_send_stop();
	WP_DISABLE();*/
}
