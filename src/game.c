#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include <util/delay.h>
#include "buzzer.h"
#include "highscore.h"
#include "game.h"
#include "leds.h"
#include "buttons.h"

#ifndef SENSOCARD
    #include "display.h"
#endif

#define NUM_BUTTONS    4
#define HIGHEST_SCORE 25
#define NUM_LEVELS     5
const uint8_t levels[NUM_LEVELS]   = {   0,   4,   8,  15,  20 };
const uint16_t show_ms[NUM_LEVELS] = { 400, 300, 250, 200, 150 };
const uint16_t wait_ms[NUM_LEVELS] = { 300, 250, 200, 150, 100 };
#define NOTE_DURATION 215

const uint16_t button_notes[NUM_BUTTONS] = { NOTE_A5, NOTE_A4, NOTE_F4, NOTE_D6 };

const uint16_t begin_notes[] = {
    NOTE_E7, NOTE_E7, 0, NOTE_E7,
    0, NOTE_C7, NOTE_E7, 0,
    NOTE_G7 };

const uint16_t lose_notes[] = {
    NOTE_FS4, 0, NOTE_A4, NOTE_CS5, 0, NOTE_A4, 0, NOTE_FS4,
    NOTE_D4, NOTE_D4 };

void my_delay(int ms)
{
    while (ms > 0)
    {  
        _delay_ms(10);
        ms -= 10;
    }
}

void play_tune(const uint16_t *notes, size_t length, uint8_t led_pattern)
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
        display <<= 1;
        #endif

        if (i % 2 == 0)
        {
            leds = ~leds;
            set_leds(leds);
        }

        play_freq(notes[2 * i]);
        my_delay(NOTE_DURATION * 0.9f);
        silent();
        my_delay(NOTE_DURATION * 0.1f);
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
void play_game()
{
    play_tune(begin_notes, sizeof(begin_notes) / sizeof(uint16_t), 0b1001);

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
        seq[i] = (get_timer_capture() + i) % NUM_BUTTONS;

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
                            write_highscore(i);
                            set_leds(1 << seq[j]);
                            play_freq(button_notes[seq[j]]);
                            _delay_ms(1000);
                            set_leds(0);
                            silent();
                            _delay_ms(500);
                            play_tune(lose_notes, sizeof(lose_notes) / sizeof(uint16_t), 0b1010);
                            return;
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
    write_highscore(HIGHEST_SCORE);
}
