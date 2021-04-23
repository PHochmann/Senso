#define F_CPU 16000000L
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <avr/io.h>
#include <util/delay.h>
#include <stddef.h>

#include "display.h"

#define NUM_BUTTONS    4
#define HIGHEST_SCORE 99

#define LED_PORT    PORTC
#define LED_DDR     DDRC
const uint8_t LEDs[NUM_BUTTONS] = { 0, 1, 2, 3 };

#define BUTTON_PORT PORTD
#define BUTTON_DDR  DDRD
#define BUTTON_PIN  PIND
const uint8_t buttons[NUM_BUTTONS] = { 5, 4, 3, 2 };

#define BUZZER_DDR  DDRC
#define BUZZER      4
#define TONE1       
#define TONE2
#define TONE3
#define TONE4

// Buzzer is A4 -> PORTC, Pin 4

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
    BUTTON_DDR &= ~(mask);
    // Pull up resistors
    BUTTON_PORT |= mask;
}

void buzzer_init()
{

}

/*
Returns: Achieved score
*/
uint8_t play_game(int seed)
{
    // Show game begin animation
    uint8_t curr_mask = 1;
    for (size_t i = 0; i < 12; i++)
    {
        display_send(~curr_mask);
        display_send(~curr_mask);
        display_show();
        _delay_ms(100);
        curr_mask = ((curr_mask << 1) & 0b111111) | ((curr_mask >> 5) & 1);
    }

    // Initialise score
    display_show_number(0);

    // Generate random sequence
    srand(seed);
    uint8_t seq[HIGHEST_SCORE];
    for (size_t i = 0; i < HIGHEST_SCORE; i++)
    {
        seq[i] = rand() % NUM_BUTTONS;
    }

    // Check for correct presses
    for (size_t i = 0; i < HIGHEST_SCORE; i++)
    {
        // Repeat previous sequence
        for (size_t j = 0; j <= i; j++)
        {
            LED_PORT &= ~(1 << LEDs[seq[j]]);
            _delay_ms(250);
            LED_PORT |= 1 << LEDs[seq[j]];
            _delay_ms(100);
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
                    if ((BUTTON_PIN & (1 << buttons[k])) == 0)
                    {
                        if (k != seq[j])
                        {
                            // Wrong button - Game over
                            LED_PORT &= ~(1 << LEDs[seq[j]]);
                            _delay_ms(500);
                            LED_PORT |= 1 << LEDs[seq[j]];
                            return i;
                        }
                        else
                        {
                            // Correct button
                            LED_PORT &= ~(1 << LEDs[seq[j]]);
                            _delay_ms(250);
                            LED_PORT |= 1 << LEDs[seq[j]];
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
    }

    // Game is won!
    return HIGHEST_SCORE;
}

int main()
{
    display_init();

    const uint8_t led_mask = (1 << LEDs[0]) | (1 << LEDs[1]) | (1 << LEDs[2]) | (1 << LEDs[3]);
    const uint8_t button_mask = (1 << buttons[0] | 1 << buttons[1] | 1 << buttons[2] | 1 << buttons[3]);
    buttons_init(button_mask);
    leds_init(led_mask);


    uint8_t highscore = 0;

    while (true)
    {
        display_show_number(highscore);
        _delay_ms(200);
        display_clear();
        _delay_ms(200);

        if ((BUTTON_PIN & button_mask) != button_mask)
        {
            uint8_t score = play_game(0);
            if (score > highscore) highscore = score;
        }
    }

    while (true);
}
