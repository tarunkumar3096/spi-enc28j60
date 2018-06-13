#include <stdint.h>

void read_eth_buf(uint8_t *data, uint8_t data_len);
void write_eth_buf(uint8_t *data, uint8_t datalen);
void tx_init(void);
void rx_init(void);
void mac_init(uint8_t *mac_addr);
void transmit_data(uint8_t *packet, uint8_t len);
uint16_t receive_data(uint8_t *rec_buf, uint16_t max_len);
