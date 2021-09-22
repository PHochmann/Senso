#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <stddef.h>

#include <avr/io.h>
#include <util/delay.h>
#include <avr/eeprom.h>

#include "display.h"
#include "buzzer.h"

#define NUM_BUTTONS    4
#define HIGHEST_SCORE 99
#define HIGHSCORE_ADDR ((uint8_t*)10)

#define NUM_LEVELS 6
const uint8_t  levels[NUM_LEVELS]  = {   0,   5,  10,  20,  30,  50 };
const uint16_t show_ms[NUM_LEVELS] = { 400, 300, 250, 200, 150, 100 };
const uint16_t wait_ms[NUM_LEVELS] = { 300, 250, 150, 100,  50,  30 };

#define LED_PORT     PORTC
#define LED_DDR      DDRC
#define FIRST_BUTTON 0

#define BUTTON_PORT PORTD
#define BUTTON_DDR  DDRD
#define BUTTON_PIN  PIND
#define FIRST_LED   4

const uint16_t button_notes[NUM_BUTTONS] = { NOTE_E7, NOTE_C7, NOTE_F4, NOTE_D6 };

const uint16_t begin_notes[] = {
    NOTE_E7, NOTE_E7, 0, NOTE_E7, 0, NOTE_C7, NOTE_E7, 0, NOTE_G7
};

const uint16_t lose_notes[] = {
    NOTE_FS4, 0, NOTE_A4, NOTE_CS5, 0, NOTE_A4, 0, NOTE_FS4, NOTE_D4, NOTE_D4
};

const uint8_t led_mask = (1 << FIRST_LED) | (1 << (FIRST_LED + 1)) | (1 << (FIRST_LED + 2)) | (1 << (FIRST_LED + 3));
const uint8_t button_mask = (1 << FIRST_BUTTON) | (1 << (FIRST_BUTTON + 1)) | (1 << (FIRST_BUTTON + 2)) | (1 << (FIRST_BUTTON + 3));

void leds_init(uint8_t mask)
{
    // Output
    LED_DDR |= mask;
    // Turn off
    LED_PORT |= mask;
}

void buttons_init(uint8_t mask)
{
    // Input
    BUTTON_DDR &= ~mask;
    // Pull up resistors
    BUTTON_PORT |= mask;
}

bool is_input()
{
    return (BUTTON_PIN & button_mask) != button_mask;
}

void my_delay(int ms)
{
    while (ms > 0)
    {  
        _delay_ms(10);
        ms -= 10;
    }
}

void play_begin_animation()
{
    uint8_t curr_display_mask = 1;
    uint8_t curr_led_mask = led_mask ^ 0b10101010;
    for (size_t i = 0; i < sizeof(begin_notes) / sizeof(begin_notes[0]); i++)
    {
        display_send(~curr_display_mask);
        display_send(~curr_display_mask);
        display_show();
        LED_PORT |= led_mask;
        LED_PORT &= ~(curr_led_mask & led_mask);
        play_freq(begin_notes[i]);

        if (i % 2 == 0) curr_led_mask ^= led_mask;
        curr_display_mask = ((curr_display_mask << 1) & 0b111111) | ((curr_display_mask >> 5) & 1);
        _delay_ms(150);
    }
    LED_PORT |= led_mask;
    silent();
    display_clear();
    _delay_ms(1500);
}

void play_lose_animation()
{
    uint8_t curr_display_mask = 0b10011111;
    uint8_t curr_led_mask = 0b00001001;

    for (size_t i = 0; i < sizeof(lose_notes) / sizeof(lose_notes[0]); i++)
    {
        display_send(curr_display_mask);
        display_send(curr_display_mask);
        display_show();
        LED_PORT |= led_mask;
        LED_PORT &= ~(curr_led_mask & led_mask);
        play_freq(lose_notes[i]);

        curr_display_mask = (curr_display_mask >> 1) | (1 << 7);
        if (i % 2 == 0) curr_led_mask ^= 0b00001111;
        _delay_ms(170);
    }

    LED_PORT |= led_mask;
    silent();
    display_clear();
    _delay_ms(1500);
}

void show_button(size_t index, uint16_t ms)
{
    LED_PORT &= ~(1 << (FIRST_LED + index));
    play_freq(button_notes[index]);
    my_delay(ms);
    LED_PORT |= 1 << (FIRST_LED + index);
    silent();
}

/*
Returns: Achieved score
*/
uint8_t play_game(int seed)
{
    play_begin_animation();

    // Initialise score
    display_show_number(0);

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
            show_button(seq[j], curr_show_ms);
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
                    if ((BUTTON_PIN & (1 << (FIRST_BUTTON + k))) == 0)
                    {
                        if (k != seq[j])
                        {
                            // Wrong button - Game over
                            show_button(seq[j], 1000);
                            _delay_ms(500);
                            play_lose_animation();
                            return i;
                        }
                        else
                        {
                            // Correct button
                            show_button(250, curr_show_ms);
                            correct = true;
                            break;
                        }
                    }
                }
                if (correct) break;
            }
        }

        // Sequence was correct
        // Update score
        display_show_number(i + 1);
        // Wait until all buttons are released
        while (is_input());
        _delay_ms(500);
    }

    // Game is won!
    return HIGHEST_SCORE;
}

int main()
{
    DDRD = 0xFF;
    while(true)
    {
        PORTD ^= 0xFF;
        my_delay(1000);
    }

    buttons_init(button_mask);
    leds_init(led_mask);

    display_init();
    buzzer_init();

    // Erase highscore if both outer buttons are pressed on startup
    if (((BUTTON_PIN & button_mask) >> FIRST_BUTTON) == 0b0110)
    {
        play_freq(440);
        display_send(0);
        display_send(0);
        display_show();
        _delay_ms(2000);
        silent();
        display_clear();
        if (is_input())
        {
            eeprom_write_byte(HIGHSCORE_ADDR, 0);
            while (is_input());
        }
    }

    // Start timer to get random values
    TCCR0A = 0;
    TCCR0B = (0 << CS02) | (0 << CS01) | (1 << CS00);

    uint8_t highscore = eeprom_read_byte(HIGHSCORE_ADDR);
    while (true)
    {
        bool start = is_input();

        display_show_number(highscore);
        _delay_ms(500);
        start |= is_input();
        display_clear();
        _delay_ms(500);
        start |= is_input();

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
