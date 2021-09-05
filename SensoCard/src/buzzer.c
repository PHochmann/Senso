#include <stdint.h>
#include <stdlib.h>
#include <avr/io.h>
#include "buzzer.h"

#define BUZZER_DDR       DDRB
#define BUZZER           2
#define PRESCALER        64
#define TICKS_PER_SECOND (F_CPU / PRESCALER)

void play_freq(uint16_t hz)
{
    TCCR0B = (0 << WGM02) | (0 << CS02) | (1 << CS01) | (1 << CS00); 
    if (hz == 0)
    {
        silent(0);
    }
    else
    {
        OCR0A = TICKS_PER_SECOND / hz;
    }
}

void silent()
{
    TCCR0B = (0 << CS02) | (0 << CS01) | (0 << CS00); 
}

void buzzer_init()
{
    BUZZER_DDR |= (1 << BUZZER);
    TCCR0A = (0 << COM0A1) | (1 << COM0A0) | (1 << WGM01) | (0 << WGM00);   // COM2B1 (output to OC2B) ; WGMode 7 Fast PWM (part 1)
}
