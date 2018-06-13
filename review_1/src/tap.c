#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <net/if.h>
#include <linux/if_tun.h>
#include <linux/ip.h>
#include <net/if_arp.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <signal.h>
#include <error.h>
#include <errno.h>

#define MAC_LEN 6

int set_mac(int tap_fd, char *dev_name, char *mac)
{
	struct ifreq ifr;

	memset(&ifr, 0, sizeof(struct ifreq));
	strncpy(ifr.ifr_name, dev_name, IFNAMSIZ);
	memcpy(ifr.ifr_hwaddr.sa_data, mac, MAC_LEN);

	ifr.ifr_hwaddr.sa_family = ARPHRD_ETHER;
	if (ioctl(tap_fd, SIOCSIFHWADDR, (void *)&ifr) == -1) {
		perror("Cannot set MAC address");
		return -1;
	}
	return 0;
}

static int _tap_alloc(char *id, int flags)
{
	struct ifreq ifr;
	int fd, err;
	char *clonedev = "/dev/net/tun";

	fd = open(clonedev, O_RDWR);
	if (fd < 0) {
		perror("Opening /dev/net/tun Failed");
		return fd;
	}

	memset(&ifr, 0, sizeof(ifr));

	ifr.ifr_flags = flags;

	if (*id)
		strncpy(ifr.ifr_name, id, IFNAMSIZ);

	err = ioctl(fd, TUNSETIFF, (void *)&ifr);
	if (err < 0) {
		perror("ioctl(TUNSETIFF) Error");
		close(fd);
		return err;
	}

	strcpy(id, ifr.ifr_name);

	return fd;
}

int tap_open(char *dev_name)
{
	return _tap_alloc(dev_name, IFF_TAP | IFF_NO_PI);
}

int tap_read(int tap_fd, unsigned char *data, unsigned int len)
{
	return read(tap_fd, data, len);
}

int tap_write(int tap_fd, unsigned char *data, unsigned int len)
{
	return write(tap_fd, data, len);
}
