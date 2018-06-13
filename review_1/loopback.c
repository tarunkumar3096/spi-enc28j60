#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <error.h>
#include <tap.h>
#include <ss_gpio.h>
#include <eth_reg_module.h>
#include <enc28j60_reg_map.h>
#include <vf51_module.h>
#include <cntrl_reg_oper.h>

void packet_merge(uint8_t *src, uint8_t *des, int offset, int len)
{
	int i = offset;
	int j;
 
	for (j = 0; j < len; j++, i++) {
		src[i] = des[j];
	}
}

#define DATA_SIZE 60

int main()
{
	uint8_t mac[6] = {0x31, 0x32, 0x33, 0x34, 0x35, 0x36};
	uint8_t tx_data[DATA_SIZE]; 
	uint8_t len[2] = {0, 2};
	uint8_t data_buf[4];
	uint8_t rx_len;
	uint8_t rx_data[1500];
	int i;
	uint8_t oui;
	
	ioctl_init();
	ss_gpio_init();
	tx_init();
	rx_init();

	write_register(REG_MIREGADR, PHY_PHID1);

	read_phy_reg(REG_MIRDH, &oui);
	mac[0] = oui;
	read_phy_reg(REG_MIRDL, &oui);
	mac[1] = oui;

	write_register(REG_MIREGADR, PHY_PHID2);
	read_phy_reg(REG_MIRDH, &oui);
	mac[2] = oui;

	mac_init(mac);

	while(1) {
		memset(tx_data , 0, sizeof(tx_data));
		tx_data[0] = 0x06 | 0x01;
		printf("Enter Data to transmit\n");
		fgets((char *)data_buf, sizeof(data_buf), stdin);	

		packet_merge(tx_data, mac, 1, 6);
		packet_merge(tx_data, mac, 7, 6);
		packet_merge(tx_data, len, 13, 2);
		packet_merge(tx_data, data_buf, 15, sizeof(data_buf));

		printf("\nTransmit_data\n");		
		for ( i = 0; i < 64; i++)
			printf("%x ", tx_data[i]);	

		transmit_data(tx_data, DATA_SIZE);
		       
		memset(rx_data, 0, sizeof(rx_data));
		receive_data(rx_data, 64);

		printf("\nReceived_data\n");		
		for ( i = 0; i < 64; i++)
			printf("%x ", rx_data[i]);
	
		rx_len = rx_data[15];

		printf("\ndata length = %x\n", rx_len);

		printf("Data: ");
		for ( i = 16; i < rx_len + 15; i++)
			printf("%c ", rx_data[i]);

		printf("\n\n");
	}

	return 0;
}
