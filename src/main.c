#define F_CPU 16000000L
#include <stdbool.h>
#include <stdint.h>
#include <avr/io.h>
#include <util/delay.h>
#include <stddef.h>

#define DISPLAY_DDR        DDRD
#define DISPLAY_PORT       PORTD
#define DISPLAY_DATA_PIN   2
#define DISPLAY_CLOCK_PIN  3
#define DISPLAY_ENABLE_PIN 4

const uint8_t display_lookup[10] = {
    0xc0, 0xf9, 0xa4, 0xb0, 0x99,
    0x92, 0x82, 0xf8, 0x80, 0x90
};

void display_init()
{
    DISPLAY_DDR |= (1 << DISPLAY_CLOCK_PIN) | (1 << DISPLAY_DATA_PIN) | (1 << DISPLAY_ENABLE_PIN);
    DISPLAY_PORT &= ~(1 << DISPLAY_CLOCK_PIN | 1 << DISPLAY_ENABLE_PIN);
}

void display_send(uint8_t bits)
{
    //for (size_t i = 0; i < 8; i++)
    for (int i = 7; i >= 0; i--)
    {
        DISPLAY_PORT &= ~(1 << DISPLAY_DATA_PIN);
        DISPLAY_PORT |= ((bits >> i) & 1)  << DISPLAY_DATA_PIN;
        DISPLAY_PORT |= 1 << DISPLAY_CLOCK_PIN;
        DISPLAY_PORT &= ~(1 << DISPLAY_CLOCK_PIN);
    }
}

void display_number(int number)
{
    int t = (number / 10) % 10;
    display_send(display_lookup[number % 10]);
    display_send(t != 0 ? display_lookup[t] : 0xFF);

    DISPLAY_PORT &= ~(1 << DISPLAY_ENABLE_PIN);
    DISPLAY_PORT |= 1 << DISPLAY_ENABLE_PIN;
}

void display_clear()
{
    display_send(0b10101010);
    display_send(0);
    DISPLAY_PORT &= ~(1 << DISPLAY_ENABLE_PIN);
    DISPLAY_PORT |= 1 << DISPLAY_ENABLE_PIN;
}

int main()
{
    display_init();
    display_clear();

    int i = 0;
    while(true)
    {
        display_number(i);
        _delay_ms(200);
        i++;
        if (i == 100) i = 0;
    }

    return 0;
}
