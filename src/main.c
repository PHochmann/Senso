#include <stdbool.h>
#include <avr/power.h>
#include <util/delay.h>

#include "buzzer.h"
#ifndef SENSOCARD
    #include "display.h"
#endif
#include "highscore.h"
#include "game.h"
#include "leds.h"
#include "buttons.h"

#define MIN(a, b) (((a) <= (b)) ? (a) : (b))

bool delay_and_wait(int ms)
{
    while (ms > 0)
    {  
        _delay_ms(10);
        ms -= 10;
        if (get_input() != 0) return true;
    }
    return false;
}

int main()
{
    // Power reduction, disable USI and ADC
    ADCSRA &= ~(1 << ADEN);
    ACSR |= 1 << ACD;
    PRR &= ~(1 << PRUSI | 1 << PRADC);

    // Initialize registers and external hardware
    buttons_init();
    leds_init();
    buzzer_init();
    #ifndef SENSOCARD
        display_init();
    #endif

    // Play startup beep and diagnostics
    set_leds(0b1111);
    play_freq(640);
    _delay_ms(500);
    silent();
    set_leds(0);

    // Erase highscore if secret combination is pressed on startup
    if (get_input() == 0b1001)
    {
        clear_highscore();
        wait_for_no_input();
    }

    uint8_t highscore = read_highscore();

    while (true)
    {
        bool start = false;
        
        #ifdef SENSOCARD
            set_leds(MIN(pow(2, NUM_BUTTONS) - 1, highscore));
        #else
            display_show_number(highscore);
        #endif

        start |= delay_and_wait(500);

        #ifdef SENSOCARD
            set_leds(0);
        #else
            display_clear();
        #endif

        start |= delay_and_wait(500);

        if (start)
        {
            play_game();
            highscore = read_highscore();
        }
    }
    
    return 0;
}
