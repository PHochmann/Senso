#include <stdint.h>
#include <stdlib.h>
#include <avr/io.h>
#include "buzzer.h"

#define BUZZER_DDR  DDRD
#define BUZZER      3

void play_freq(uint16_t freq)
{
    if (freq == 0)
    {
        OCR2A = 0;
        OCR2B = 0;
    }
    else
    {
        OCR2A = 15625 / freq;
        OCR2B = 15625 / freq / 2;
    }
}

void silent()
{
    play_freq(0);
}

void buzzer_init()
{
    BUZZER_DDR |= (1 << BUZZER);
    OCR2B = 0;
    TCCR2A = (1 << COM2B1) | (1 << WGM21) | (1 << WGM20);   // COM2B1 (output to OC2B) ; WGMode 7 Fast PWM (part 1)
    TCCR2B = (1 << WGM22) | (1 << CS22) | (1 << CS21) | (1 << CS20); 
}
