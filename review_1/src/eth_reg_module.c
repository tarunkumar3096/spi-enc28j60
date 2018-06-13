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
#include <enc28j60_reg_map.h>
#include <eth_reg_module.h>
#include <cntrl_reg_oper.h>
#include <ss_gpio.h>
#include <vf51_module.h>

extern int fd;
static uint16_t nxt_packet_ptr;
#define MAX_FRAMELEN 0x0080
#define RX_BUF_START 0x0000
#define RX_BUF_END 0x0fff
#define TX_BUF_START 0x1200
#define TX_BUF_END 0x1FF0

void read_eth_buf(uint8_t *data, uint8_t data_len)
{
	uint8_t tx[data_len + 1];
	
	tx[0] = SPI_RBM;      

	ss_set_state(SS_LOW);
	transfer(fd, tx, data, data_len);
	ss_set_state(SS_HIGH);	
}

void write_eth_buf(uint8_t *data, uint8_t data_len)
{
	int i = 0;
	uint8_t tx[data_len + 1];
	uint8_t rx[data_len + 1];
	
	tx[0] = SPI_WBM;
	
	for (i = 1; i <= data_len; i++)
		tx[i] = data[i - 1];

	ss_set_state(SS_LOW);
	transfer(fd, tx, rx, data_len);
	ss_set_state(SS_HIGH);
}

void tx_init(void)
{
	//set tx start
	write_register(REG_ETXSTL, (TX_BUF_START & 0xff));
	write_register(REG_ETXSTH, (TX_BUF_START >> 8));

	//set tx end
	write_register(REG_ETXNDL, (TX_BUF_END & 0xff));
	write_register(REG_ETXNDH, (TX_BUF_END >> 8));	
}

void rx_init(void)
{
	nxt_packet_ptr = RX_BUF_START;

	//set RX buffer start  
	write_register(REG_ERXSTL, (RX_BUF_START & 0xff));
	write_register(REG_ERXSTH, (RX_BUF_START >> 8));

	//set recieve pointer address
	write_register(REG_ERXRDPTL, ((RX_BUF_START) & 0xff));
	write_register(REG_ERXRDPTH, ((RX_BUF_START) >> 8));

	//set rx buffer end
	write_register(REG_ERXNDL, (RX_BUF_END & 0xff));
	write_register(REG_ERXNDH, (RX_BUF_END >> 8));

	//packet filter
	write_register(REG_ERXFCON, BIT_ERXFCON_UCEN| BIT_ERXFCON_PMEN | BIT_ERXFCON_CRCEN);
}

void mac_init(uint8_t *mac_addr)
{
	
	write_register(REG_MACON2, BIT_MACON2_MARST);

	//bring mac out of reset
	write_register(REG_MACON2, 0x00);
	
	//mac recieve bit enable  
	write_register(REG_MACON1, BIT_MACON1_TXPAUS
		       | BIT_MACON1_RXPAUS
		       | BIT_MACON1_MARXEN);

	//enable automatic padding
	set_bit_reg(REG_MACON3, BIT_MACON3_TXCRCEN
		    | BIT_MACON3_PADCFG0
		    | BIT_MACON3_FRMLNEN | BIT_MACON3_FULDPX);

	//set frame inter-frame gap(non-back-to-back)
	write_register(REG_MAIPGL, NBTB_MAIPGL);

	//set frame inter-frame gap(back-to-back)
	write_register(REG_MABBIPG, FULL_DUPLEX_MODE);

	//set maximum packet size
	write_register(REG_MAMXFLL, MAX_FRAMELEN & 0XFF); 
	write_register(REG_MAMXFLL, MAX_FRAMELEN >> 8);

	//write mac address
	write_register(REG_MAADR5, mac_addr[5]);
	write_register(REG_MAADR4, mac_addr[4]);
	write_register(REG_MAADR3, mac_addr[3]);
	write_register(REG_MAADR2, mac_addr[2]);
	write_register(REG_MAADR1, mac_addr[1]);
	write_register(REG_MAADR0, mac_addr[0]);

	//no loop back transmitted t'''frame
	write_register(REG_MIREGADR, PHY_PHCON2);
	write_register(REG_MIWRL, 0X00);
	write_register(REG_MIWRH, BIT_PHCON2_HDLDIS);

	//enable interrupts
	set_bit_reg(REG_EIE, BIT_EIE_INTIE | BIT_EIE_PKTIE);

	set_bit_reg(REG_ECON2, BIT_ECON2_AUTOINC);
	//enable packet reception
	set_bit_reg(REG_ECON1, BIT_ECON1_RXEN);
  
}

void transmit_data(uint8_t *packet, uint8_t len)
{
	//check no transmit in progress
	uint8_t status;

	while (1) {
		read_register(REG_ECON1, &status);
		if ((status & BIT_ECON1_TXRTS) == 0)
			break;
		read_register(REG_EIR, &status);
		if (status & BIT_EIR_TXERIF) {
			set_bit_reg(REG_ECON1, BIT_ECON1_TXRST);
			clear_bit_reg(REG_ECON1, 0x00);
		}
	}

	//set write pointer
	write_register(REG_EWRPTL, TX_BUF_START & 0XFF);
	write_register(REG_EWRPTH, TX_BUF_START >> 8);

	//set tx end pointer
	write_register(REG_ETXNDL, (TX_BUF_START + len) & 0XFF);
	write_register(REG_ETXNDH, (TX_BUF_START + len) >> 8);
	
	//copy data to buffer
	write_eth_buf(packet, len);

	//start transmission
	set_bit_reg(REG_ECON1, BIT_ECON1_TXRTS);
 }

uint16_t receive_data(uint8_t *rec_buf, uint16_t max_len)
{
	uint8_t rsv[6];
	uint8_t count;		
	uint8_t wr_addr;
	uint16_t packet_ptr;
	uint16_t rx_data_len;
	uint16_t rx_stat;	
	
	//check if packet in buffer
	read_register(REG_EPKTCNT, &count);
	if (count == 0)
		return 0;
	
	//set read pointer
	write_register(REG_ERDPTL, nxt_packet_ptr & 0xff);
	write_register(REG_ERDPTH, nxt_packet_ptr >> 8);

	//Reading receive status
	read_eth_buf(rsv, 6);

	//read next pointer
	nxt_packet_ptr = rsv[1];
	nxt_packet_ptr |= rsv[0] << 8;

	read_register(REG_ERXWRPTL, &wr_addr);
	packet_ptr = wr_addr;
	read_register(REG_ERXWRPTH, &wr_addr);
	packet_ptr |= wr_addr << 8;

	if (packet_ptr != nxt_packet_ptr)
		nxt_packet_ptr = packet_ptr;
	
	//read packet rx_data_len
	rx_data_len = rsv[3];
	rx_data_len |= rsv[2] << 8;

	//remove crc count
	if (rx_data_len > 0)
		rx_data_len = rx_data_len - 4;
	else 
		return 0;

	//read receive status
	rx_stat = rsv[5];
	rx_stat |= rsv[4] << 8;

	//limit obtained length
	if (rx_data_len > (max_len - 1))
		rx_data_len = max_len - 1;

	//check crc and symbol errors
	if (rx_stat & 0x10) 
		return 0;
	
	read_eth_buf(rec_buf, rx_data_len);

	write_register(REG_ERXRDPTL, nxt_packet_ptr & 0xff);
	write_register(REG_ERXRDPTH, nxt_packet_ptr >> 8);

	write_register(REG_ERDPTL, nxt_packet_ptr & 0xff);
	write_register(REG_ERDPTH, nxt_packet_ptr >> 8);	
  
	//decrement packet
	set_bit_reg(REG_ECON2, BIT_ECON2_PKTDEC);
	
	return rx_data_len;
}
