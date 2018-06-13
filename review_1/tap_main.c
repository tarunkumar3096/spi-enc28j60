#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <error.h>
#include <tap.h>
#include <ss_gpio.h>
#include <eth_reg_module.h>
#include <enc28j60_reg_map.h>
#include <vf51_module.h>
#include <cntrl_reg_oper.h>

void str_cat(uint8_t *src, uint8_t *des, int offset, int len)
{
	int i = offset;
	int j;

	//  for (i = 0; src[i] != '\0'; i++);


	for (j = 0; j < len; j++, i++) {
		printf("dest = %x\n", des[j]);
		src[i] = des[j];
	}
}

#define DATA_SIZE 60

int main()
{
	uint8_t mac[6] = {0x31, 0x32, 0x33, 0x34, 0x35, 0x36};
	uint8_t tx_data[DATA_SIZE];
	uint8_t rx_data[64];
	int fd;
	int i;
	uint8_t oui;
	uint16_t tx_len;
	uint16_t rx_len;
	char dev_name[122] = "aadhi";

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

	fd = tap_open(dev_name);
	set_mac(fd, dev_name, (char *)mac);

	memset(rx_data, 0, sizeof(rx_data));
	
	while (1) {
		tx_len = tap_read(fd, tx_data, sizeof(tx_data));
		if (tx_len > 0) {
			printf("\nread len = %d\n", tx_len);

			printf("Ping transmitted\n");			
			for (i = 0; i < tx_len; i++)
				printf("%x ", tx_data[i]);
			transmit_data(tx_data, tx_len);

			sleep(2);

			rx_len = receive_data(rx_data, sizeof(rx_data));

			if (rx_len > 0) {
				printf("Ping Received\n");
				printf("wr len = %d\n", rx_len);

				for (i = 0; i < rx_len; i++)
					printf("%x ", rx_data[i]);		

				tap_write(fd, rx_data, rx_len);
			}
		}
	}		
	return 0;
}
