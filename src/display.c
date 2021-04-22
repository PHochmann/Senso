#include <stddef.h>
#include <stdint.h>
#include <avr/io.h>
#include "display.h"

#define DISPLAY_DDR  DDRD
#define DISPLAY_PORT PORTD
#define DIN_PIN      2
#define CLK_PIN      3
#define LE_PIN       4

const uint8_t display_lookup[10] = {
    0xc0, 0xf9, 0xa4, 0xb0, 0x99, 0x92, 0x82, 0xf8, 0x80, 0x90
};

void display_init()
{
    DISPLAY_DDR |= (1 << CLK_PIN) | (1 << DIN_PIN) | (1 << LE_PIN);
    DISPLAY_PORT |= 1 << LE_PIN;
    DISPLAY_PORT &= ~(1 << CLK_PIN);
}

void display_send(uint8_t bits)
{
    for (int i = 7; i >= 0; i--)
    {
        DISPLAY_PORT &= ~(1 << DIN_PIN);
        DISPLAY_PORT |= ((bits >> i) & 1)  << DIN_PIN;
        DISPLAY_PORT |= 1 << CLK_PIN;
        DISPLAY_PORT &= ~(1 << CLK_PIN);
    }
}

void display_show()
{
    DISPLAY_PORT &= ~(1 << LE_PIN);
    DISPLAY_PORT |= 1 << LE_PIN;
}

// number from 0 to 99
void display_show_number(uint8_t number)
{
    uint8_t t = (number / 10) % 10;
    display_send(display_lookup[number % 10]);
    display_send(t != 0 ? display_lookup[t] : 0xFF);
    display_show();
}

void display_clear()
{
    display_send(0xFF);
    display_send(0xFF);
    display_show();
}
