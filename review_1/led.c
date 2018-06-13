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

int main()
{
	ioctl_init();
	ss_gpio_init();

	uint8_t rev_id;
	
	read_register(REG_EREVID, &rev_id);
	printf("Revision ID = %x\n", rev_id);	
	
	write_register(REG_MIREGADR, PHY_PHLCON);
	
	write_register(REG_MIWRL, 0xa0);
	write_register(REG_MIWRH, 0x0b);

	return 0;
}
