void soft_reset(void);
void setting_bank(uint16_t reg_addr);
void write_register(uint16_t reg_addr, int data);
void read_register(uint16_t reg_addr, uint8_t *data);
void set_bit_reg(uint16_t reg_addr, uint8_t bit_loc);
void clear_bit_reg(uint16_t reg_addr, uint8_t bit_loc);
void write_phy_reg(uint16_t reg_addr, uint8_t data);
void read_phy_reg(uint16_t reg_addr, uint8_t *data);
