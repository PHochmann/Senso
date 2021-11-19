#include <stdint.h>
#include <stdlib.h>
#include <avr/io.h>
#include "buzzer.h"

#ifdef SENSOCARD
    #define PRESCALER        64
#else
    #define PRESCALER        256 // Higher clock speed causes overflow with lower prescalers
#endif

#define TICKS_PER_SECOND     (F_CPU / PRESCALER)

#if NOTE_D4 < (TICKS_PER_SECOND / 255)
    #error "Buzzer can't output low frequency. Adjust prescaler."
#endif

#ifdef SENSOCARD
    #define BUZZER_DDR       DDRB
    #define BUZZER           2
#else
    #define BUZZER_DDR       DDRD
    #define BUZZER           6
#endif

void play_freq(uint16_t hz)
{
    if (hz == 0)
    {
        silent();
    }
    else
    {
        #if PRESCALER == 64
            TCCR0B = (0 << CS02) | (1 << CS01) | (1 << CS00); // 64
        #else
            #if PRESCALER == 256
                TCCR0B = (1 << CS02) | (0 << CS01) | (0 << CS00); // 256
            #else
                #error "Unknown prescaler"
            #endif
        #endif

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
    silent();
}
