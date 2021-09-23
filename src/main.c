#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <stddef.h>

#include <avr/io.h>
#include <util/delay.h>
#include <avr/eeprom.h>

#ifndef SENSOCARD
    #include "display.h"
#endif

#include "buzzer.h"

#define MIN(a, b) (((a) <= (b)) ? (a) : (b))

#define NUM_BUTTONS    4
#define HIGHEST_SCORE 25
#define HIGHSCORE_ADDR ((uint8_t*)10)

#define NUM_LEVELS 5
const uint8_t levels[NUM_LEVELS]   = {   0,   4,   8,  15,  20 };
const uint16_t show_ms[NUM_LEVELS] = { 400, 300, 250, 200, 150 };
const uint16_t wait_ms[NUM_LEVELS] = { 300, 250, 200, 150, 100 };

#ifdef SENSOCARD
    #define LED_PORT    PORTA
    #define LED_DDR     DDRA
    const uint8_t LEDs[NUM_BUTTONS] = { 0, 1, 2, 3 };

    #define BUTTON_PORT PORTA
    #define BUTTON_DDR  DDRA
    #define BUTTON_PIN  PINA
    const uint8_t buttons[NUM_BUTTONS] = { 4, 5, 6, 7 };
#else
    #define LED_PORT    PORT?
    #define LED_DDR     DDR?
    const uint8_t LEDs[NUM_BUTTONS] = { ?, ?, ?, ? };

    #define BUTTON_PORT PORT?
    #define BUTTON_DDR  DDR?
    #define BUTTON_PIN  PIN?
    const uint8_t buttons[NUM_BUTTONS] = { ?, ?, ?, ? };
#endif


uint8_t led_mask;
uint8_t button_mask;

/* Music */

#define TEMPO (140)
#define WHOLENOTE ((60000 * 2) / TEMPO)

const uint16_t button_notes[NUM_BUTTONS] = { NOTE_A5, NOTE_A4, NOTE_F4, NOTE_D6 };

const uint16_t begin_notes[] = {
    NOTE_E7, NOTE_E7, 0, NOTE_E7,
    0, NOTE_C7, NOTE_E7, 0,
    NOTE_G7 };

const uint16_t lose_notes[] = {
    NOTE_FS4, 0, NOTE_A4, NOTE_CS5, 0, NOTE_A4, 0, NOTE_FS4,
    NOTE_D4, NOTE_D4 };

const uint16_t win_notes[] = {
    NOTE_E5, 4, NOTE_B4, 8, NOTE_C5, 8, NOTE_D5, 4, NOTE_C5, 8, NOTE_B4, 8,
    NOTE_A4, 4, NOTE_A4, 8, NOTE_C5, 8, NOTE_E5, 4, NOTE_D5, 8, NOTE_C5, 8,
    NOTE_B4, -4, NOTE_C5, 8, NOTE_D5, 4, NOTE_E5, 4,
    NOTE_C5, 4, NOTE_A4, 4, NOTE_A4, 8, NOTE_A4, 4, NOTE_B4, 8, NOTE_C5, 8
};

uint8_t get_input()
{
    return (~BUTTON_PIN & button_mask) >> buttons[0];
}

void wait_for_no_input()
{
    while (get_input() != 0);
}

void set_leds(uint8_t states)
{
    states &= led_mask;
    LED_PORT = (LED_PORT & ~led_mask) | (~states << LEDs[0]);
}

void my_delay(int ms)
{
    while (ms > 0)
    {  
        _delay_ms(10);
        ms -= 10;
    }
}

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

void play_tune(const uint16_t *notes, size_t length, bool includes_times, uint8_t led_pattern)
{
    #ifndef SENSOCARD
    uint16_t display = 1;
    #endif

    uint8_t leds = led_pattern;
    for (size_t i = 0; i < length; i++)
    {
        #ifndef SENSOCARD
        display_send(~display);
        display_send(~display);
        display_show();
        #endif

        if (i % 2 == 0)
        {
            leds = ~leds;
            set_leds(leds);
        }

        uint16_t noteDuration;
        if (includes_times)
        {
            int divider = notes[2 * i + 1];
            noteDuration = 0;
            if (divider > 0)
            {
                noteDuration = WHOLENOTE / divider;
            }
            else
            {
                noteDuration = WHOLENOTE / abs(divider);
                noteDuration *= 1.5f;
            }
        }
        else
        {
            noteDuration = WHOLENOTE / 4;
        }

        play_freq(notes[2 * i]);
        my_delay(noteDuration * 0.9f);
        silent();
        my_delay(noteDuration * 0.1f);
    }
    set_leds(0);
    #ifndef SENSOCARD
    display_clear();
    #endif
    _delay_ms(1500);
}

/*
Returns: Achieved score
*/
uint8_t play_game()
{
    play_tune(begin_notes, sizeof(begin_notes) / sizeof(uint16_t), false, 0b1001);

    #ifndef SENSOCARD
    // Initialise score
    display_show_number(0);
    #endif

    // Random sequence
    uint8_t seq[HIGHEST_SCORE];

    // Check for correct presses
    uint8_t next_lvl = 0;
    uint16_t curr_show_ms = 0;
    uint16_t curr_wait_ms = 0;
    for (size_t i = 0; i < HIGHEST_SCORE; i++)
    {
        // Generate next number
        seq[i] = TCNT1 % NUM_BUTTONS;

        if (next_lvl < NUM_LEVELS && levels[next_lvl] == i)
        {
            next_lvl++;
            curr_show_ms = show_ms[next_lvl - 1];
            curr_wait_ms = wait_ms[next_lvl - 1];
        }

        // Repeat previous sequence
        for (size_t j = 0; j <= i; j++)
        {
            set_leds(1 << seq[j]);
            play_freq(button_notes[seq[j]]);
            my_delay(curr_show_ms);
            set_leds(0);
            silent();
            my_delay(curr_wait_ms);
        }

        // Now check if player repeats the sequence
        for (size_t j = 0; j <= i; j++)
        {
            // Check player input
            while (true)
            {
                bool correct = false;
                for (size_t k = 0; k < NUM_BUTTONS; k++)
                {
                    // Check if button is pressed
                    if ((get_input() & (1 << k)) != 0)
                    {
                        if (k != seq[j])
                        {
                            // Wrong button - Game over
                            set_leds(1 << seq[j]);
                            play_freq(button_notes[seq[j]]);
                            _delay_ms(1000);
                            set_leds(0);
                            silent();
                            _delay_ms(500);
                            play_tune(lose_notes, sizeof(lose_notes) / sizeof(uint16_t), false, 0b1010);
                            return i;
                        }
                        else
                        {
                            // Correct button
                            set_leds(1 << seq[j]);
                            play_freq(button_notes[seq[j]]);
                            my_delay(curr_show_ms);
                            set_leds(0);
                            silent();
                            correct = true;
                            break;
                        }
                    }
                }
                if (correct) break;
            }
        }

        // Sequence was correct
        #ifndef SENSOCARD
        // Show score
        display_show_number(i + 1);
        #endif
        // Wait until all buttons are released
        wait_for_no_input();
        _delay_ms(500);
    }

    // Game is won!
    play_tune(win_notes, sizeof(win_notes) / sizeof(uint16_t) / 2, true, 0b1111);
    return HIGHEST_SCORE;
}

inline void leds_init()
{
    led_mask = (1 << LEDs[0]) | (1 << LEDs[1]) | (1 << LEDs[2]) | (1 << LEDs[3]);
    // Output
    LED_DDR |= led_mask;
    // Turn off
    LED_PORT |= led_mask;
}

inline void buttons_init()
{
    button_mask = (1 << buttons[0] | 1 << buttons[1] | 1 << buttons[2] | 1 << buttons[3]);
    // Input
    BUTTON_DDR &= ~button_mask;
    // Pull up resistors
    BUTTON_PORT |= button_mask;
}

int main()
{
    buttons_init();
    leds_init();
    buzzer_init();

    set_leds(0b1111);
    play_freq(640);
    _delay_ms(500);
    silent();

    if (get_input() == 0b1001)
    {
        eeprom_write_byte(HIGHSCORE_ADDR, 0);
        wait_for_no_input();
    }

    // Start timer to get random values
    TCCR1A = 0;
    TCCR1B = (0 << CS12) | (0 << CS11) | (1 << CS10);

    uint8_t highscore = eeprom_read_byte(HIGHSCORE_ADDR);

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
            uint8_t score = play_game(TCNT0);
            if (score > highscore)
            {
                highscore = score;
                eeprom_write_byte(HIGHSCORE_ADDR, highscore);
            }
        }
    }
    
    return 0;
}
