#include <avr/io.h>
#include "leds.h"

#ifdef SENSOCARD
    #define LED_PORT    PORTA
    #define LED_DDR     DDRA
    const uint8_t LEDs[4] = { 0, 1, 2, 3 };
#else
    #define LED_PORT    PORTC
    #define LED_DDR     DDRC
    const uint8_t LEDs[4] = { 2, 3, 4, 5 };
#endif

uint8_t led_mask;

void leds_init()
{
    led_mask = (1 << LEDs[0]) | (1 << LEDs[1]) | (1 << LEDs[2]) | (1 << LEDs[3]);
    // Output
    LED_DDR |= led_mask;
    // Turn off
    LED_PORT |= led_mask;
}

void set_leds(uint8_t states)
{
    LED_PORT = (LED_PORT & ~led_mask) | (~states << LEDs[0]);
}
