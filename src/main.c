#define F_CPU 16000000L
#include <stdbool.h>
#include <stdint.h>
#include <avr/io.h>
#include <util/delay.h>
#include <stddef.h>

#include "display.h"

int main()
{
    display_init();
    display_clear();

    int i = 0;


    uint8_t curr_mask = 1;
    while (true)
    {
        display_send(~curr_mask);
        display_send(~curr_mask);
        display_show();
        _delay_ms(100);
        curr_mask = ((curr_mask << 1) & 0b111111) | ((curr_mask >> 5) & 1);
    }


    while(true)
    {
        display_show_number(i);
        _delay_ms(1000);
        i++;
        if (i == 100)
        {
            for (int i = 0; i < 5; i++)
            {
                display_send(0b00001001); // P
                display_send(0b00001100); // H
                display_show();
                _delay_ms(500);
                display_clear();
                _delay_ms(500);
            }
            i = 0;
        }
    }

    return 0;
}
