#include <stdint.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/types.h>
#include <linux/spi/spidev.h>
#include <ss_gpio.h>
#include <enc28j60_reg_map.h>
#include <cntrl_reg_oper.h>
#include <vf51_module.h>
extern int fd;

void soft_reset(void)
{
	uint8_t tx[2];
	uint8_t rx[2];

	tx[0] = SPI_SR;
	tx[0] = 0;

	ss_set_state(SS_LOW);
	transfer(fd, tx, rx, 2);
	ss_set_state(SS_HIGH);
}
	
void setting_bank(uint16_t reg_addr)
{
	uint8_t bank = ((reg_addr >> 8) & 0x0F);
	uint8_t prev_data;
	uint8_t rx[2];
	uint8_t tx[2];
	
	tx[0] = SPI_RCR | REG_ECON1;
	tx[1] = 0;
	ss_set_state(SS_LOW);
	transfer(fd, tx, rx, 2);
	ss_set_state(SS_HIGH);
	
	prev_data = rx[1];
	prev_data = (prev_data & ~(0x03));

	tx[0] = SPI_WCR | REG_ECON1;
	tx[1] = prev_data | bank;

	ss_set_state(SS_LOW);
	transfer(fd, tx, rx, 2);
	ss_set_state(SS_HIGH);
}

void write_register(uint16_t reg_addr, int data)
{
	setting_bank(reg_addr);

	uint8_t tx[2];
	uint8_t rx[2];	

	tx[0] = SPI_WCR | (reg_addr & 0xFF);
	tx[1] = data;

	ss_set_state(SS_LOW);
	transfer(fd, tx, rx, 2);
	ss_set_state(SS_HIGH);
}

void read_register(uint16_t reg_addr, uint8_t *data)
{
	setting_bank(reg_addr);
	uint8_t tx[3];
	uint8_t rx[3];
	
	tx[0] = SPI_RCR | (reg_addr & 0xFF);
	
	ss_set_state(SS_LOW);
	transfer(fd, tx, rx, 3);
	ss_set_state(SS_HIGH);
	
	*data = rx[1];
}

void write_phy_reg(uint16_t reg_addr, uint8_t data)
{
	uint8_t status;
	
	write_register(reg_addr, data);

	while(1) {
		read_register(REG_MISTAT, &status);
		if ((status & BIT_MISTAT_BUSY) == 0)
			break;
	}
}

	
void read_phy_reg(uint16_t reg_addr, uint8_t *data)
{
	uint8_t status;
	
	set_bit_reg(REG_MICMD, BIT_MICMD_MIIRD);	
	while(1) {
		read_register(REG_MISTAT, &status);
		if ((status & BIT_MISTAT_BUSY) == 0)
			break;
	}
	clear_bit_reg(REG_MICMD, BIT_MICMD_MIIRD);

	read_register(reg_addr, data);
}
	
void set_bit_reg(uint16_t reg_addr, uint8_t bit_loc)
{
	uint8_t data;
	
	read_register(reg_addr, &data);
	data |= (bit_loc);

	write_register(reg_addr, data);
}

void clear_bit_reg(uint16_t reg_addr, uint8_t bit_loc)
{
	uint8_t data;
	
	read_register(reg_addr, &data);
	data &= ~(bit_loc);

	write_register(reg_addr, data);
}

