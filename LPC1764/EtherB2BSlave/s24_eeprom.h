#ifndef __S24_EEPROM_H
#define __S24_EEPROM_H

#ifdef __cplusplus
extern "C" {
#endif

#define eeprom_busy_wait()		do {} while (!eeprom_is_ready())

void s24_eeprom_init(void);
uint8_t eeprom_is_ready(void);
uint8_t eeprom_read_byte(uint32_t address);
uint16_t eeprom_read_word(uint32_t address);
uint32_t eeprom_read_dword(uint32_t address);
void eeprom_write_byte(uint32_t address, uint8_t value);
void eeprom_write_word(uint32_t address, uint16_t value);
void eeprom_write_dword(uint32_t address, uint32_t value);
uint16_t eeprom_read_block(uint32_t address, void *dst, uint16_t size);
uint16_t eeprom_write_block(uint32_t address, void *src, uint16_t size);

#ifdef __cplusplus
}
#endif

#endif /* __S24_EEPROM_H */
