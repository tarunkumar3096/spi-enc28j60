#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <error.h>
#include <errno.h>
#include <ss_gpio.h>

#define SS_GPIO_PIN "64"
#define SS_GPIO_DIRC_PATH "/sys/class/gpio/gpio64/direction"
#define SS_GPIO_VALUE_PATH "/sys/class/gpio/gpio64/value"

void ss_gpio_init(void)
{
	int exportfd, directionfd;
 
	exportfd = open("/sys/class/gpio/export", O_WRONLY);
	if (exportfd < 0)
		error(1, errno, "%s", "Cannot open GPIO to export\n");
 
	write(exportfd, SS_GPIO_PIN, 3);
	close(exportfd);
 
	directionfd = open(SS_GPIO_DIRC_PATH, O_RDWR);
	if (directionfd < 0)
		error(1, errno, "%s", "Cannot open GPIO direction\n");
 
	write(directionfd, "out", 4);
	close(directionfd);
}

void ss_set_state(int value)
{
	int valuefd;
 
	valuefd = open(SS_GPIO_VALUE_PATH, O_RDWR);
	if (valuefd < 0)
		error(1, errno, "%s", "Cannot open GPIO value\n");

	if (value == SS_HIGH)
		write(valuefd,"1", 2);
	else
		write(valuefd,"0", 2);
	close(valuefd);

}
