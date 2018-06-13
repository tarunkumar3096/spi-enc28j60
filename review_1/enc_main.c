#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <error.h>
#include <errno.h>
#include <tap.h>
#include <ss_gpio.h>
#include <eth_reg_module.h>
#include <enc28j60_reg_map.h>
#include <vf51_module.h>
#include <cntrl_reg_oper.h>
	
extern int fd;

int main()
{
	uint8_t mac[6] = {0x00, 0x83, 0x14, 0x44, 0x55, 0x66};
	uint8_t tap_fd;
	int child;
	char *device = "Tap0";
	uint8_t ret_read_len;
	uint8_t ret_write_len;
	uint8_t read_data[100];
	uint8_t write_data[100];

	soft_reset();

	ioctl_init();		
	ss_gpio_init();
	tx_init();
	rx_init();
	mac_init(mac);
	
	tap_fd = tap_open(device);
	if (tap_fd < 0)
		error(1, errno, "%s", "Unable to open fd");
	
	if (set_mac(tap_fd, device, mac) == -1)
		error(1, 0, "%s", "Unable to set mac Address");
	
	child = fork();

	switch (child) {
	case -1:
		error(1, errno, "error in creating child");
	case 0:	
		while (1) {
			ret_read_len = tap_read(tap_fd, read_data, sizeof(read_data));
			transmit_data(read_data, ret_read_len);
		}
	default:
		while (1) {
			ret_write_len = receive_data(write_data, sizeof(write_data));
			tap_write(tap_fd, write_data, ret_write_len); 
		}			
	}

	
	
	return 0;
}
