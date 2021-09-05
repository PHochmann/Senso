#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <stddef.h>

#include <avr/io.h>
#include <util/delay.h>
#include <avr/eeprom.h>

#include "buzzer.h"

#define NUM_BUTTONS    4
#define HIGHEST_SCORE 25
#define HIGHSCORE_ADDR ((uint8_t*)10)

#define NUM_LEVELS 6
const uint8_t levels[NUM_LEVELS]   = {   0,   5,  10,  20,  30,  50 };
const uint16_t show_ms[NUM_LEVELS] = { 400, 300, 250, 200, 150, 100 };
const uint16_t wait_ms[NUM_LEVELS] = { 300, 250, 150, 100,  50,  30 };

#define LED_PORT    PORTA
#define LED_DDR     DDRA
const uint8_t LEDs[NUM_BUTTONS] = { 0, 1, 2, 3 };

#define BUTTON_PORT PORTA
#define BUTTON_DDR  DDRA
#define BUTTON_PIN  PINA
const uint8_t buttons[NUM_BUTTONS] = { 4, 5, 6, 7 };

const uint16_t button_notes[NUM_BUTTONS] = { NOTE_A4, NOTE_C7, NOTE_F4, NOTE_D6 };

#define BEGIN_LENGTH 12
const uint16_t begin_notes[BEGIN_LENGTH] = {
    NOTE_E7, NOTE_E7, 0, NOTE_E7,
    0, NOTE_C7, NOTE_E7, 0,
    NOTE_G7, 0, 0,  0 };

#define LOSE_LENGTH 10
const uint16_t lose_notes[LOSE_LENGTH] = {
    NOTE_FS4, 0, NOTE_A4, NOTE_CS5, 0, NOTE_A4, 0, NOTE_FS4,
    NOTE_D4, NOTE_D4 };

uint8_t led_mask;
uint8_t button_mask;

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

void play_tune(const uint16_t *notes, size_t length)
{
    uint8_t leds = 0b1001;
    for (size_t i = 0; i < length; i++)
    {
        if (i % 2 == 0)
        {
            leds = ~leds;
            set_leds(leds);
        }
        play_freq(notes[i]);
        _delay_ms(170);
    }
    silent();
    set_leds(0);
    _delay_ms(1500);
}

/*
Returns: Achieved score
*/
uint8_t play_game(int seed)
{
    play_tune(begin_notes, BEGIN_LENGTH);

    // Generate random sequence
    srandom(seed);
    uint8_t seq[HIGHEST_SCORE];
    for (size_t i = 0; i < HIGHEST_SCORE; i++)
    {
        seq[i] = random() % NUM_BUTTONS;
    }

    // Check for correct presses
    uint8_t next_lvl = 0;
    uint16_t curr_show_ms = 0;
    uint16_t curr_wait_ms = 0;
    for (size_t i = 0; i < HIGHEST_SCORE; i++)
    {
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
                            play_tune(lose_notes, LOSE_LENGTH);
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
        // Wait until all buttons are released
        while (get_input() != 0);
        _delay_ms(500);
    }

    // Game is won!
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
    TCCR1B = (0 << CS02) | (0 << CS01) | (1 << CS00);

    uint8_t highscore = eeprom_read_byte(HIGHSCORE_ADDR);
    while (true)
    {
        set_leds(highscore);
        if (get_input() != 0)
        {
            uint8_t score = play_game(TCNT1);
            if (score > highscore)
            {
                highscore = score;
                eeprom_write_byte(HIGHSCORE_ADDR, highscore);
            }
        }
    }
    
    return 0;
}
