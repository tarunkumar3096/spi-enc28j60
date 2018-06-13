#ifndef TAP_H
#define TAP_H

int set_mac(int tap_fd, char *dev_name, char *mac);
int tap_open(char *dev_name);
int tap_read(int tap_fd, unsigned char *data, unsigned int len);
int tap_write(int tap_fd, unsigned char *data, unsigned int len);

#endif
