#include <stdint.h>
#include <avr/eeprom.h>
#include "highscore.h"
#define HIGHSCORE_ADDR ((uint8_t*)10)

uint8_t read_highscore()
{
    return eeprom_read_byte(HIGHSCORE_ADDR);
}

void write_highscore(uint8_t score)
{
    if (score > read_highscore())
    {
        eeprom_write_byte(HIGHSCORE_ADDR, score);
    }
}

void clear_highscore()
{
    eeprom_write_byte(HIGHSCORE_ADDR, 0);
}
