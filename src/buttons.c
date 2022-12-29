#include <stdbool.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <stdint.h>
#include <stdint.h>
#include "buttons.h"

#define DEBOUNCE_NUM_SAMPLES 100
#define DEBOUNCE_WAIT_MS      10

#ifdef SENSOCARD
    #define BUTTON_PORT PORTA
    #define BUTTON_DDR  DDRA
    #define BUTTON_PIN  PINA
    static const uint8_t buttons[NUM_BUTTONS] = { 4, 5, 6, 7 };
#else
    #define BUTTON_PORT PORTB
    #define BUTTON_DDR  DDRB
    #define BUTTON_PIN  PINB
    static const uint8_t buttons[NUM_BUTTONS] = { 2, 3, 4, 5 };
#endif

static uint16_t sample_index;
static uint8_t button_samples[DEBOUNCE_NUM_SAMPLES];
static uint8_t button_mask;
static uint8_t timer_capture;

ISR (PCINT0_vect)
{
    timer_capture = TCNT1;
    button_samples[sample_index] = ((~BUTTON_PIN & button_mask) >> buttons[0]);
    sample_index = (sample_index + 1) % DEBOUNCE_NUM_SAMPLES;
}

uint8_t get_timer_capture()
{
    return timer_capture;
}

void buttons_init()
{
    button_mask = (1 << buttons[0] | 1 << buttons[1] | 1 << buttons[2] | 1 << buttons[3]);
    // Input
    BUTTON_DDR &= ~button_mask;
    // Pull up resistors
    BUTTON_PORT |= button_mask;
    // Start timer, 1024 prescaler
    TCCR1A = 0;
    TCCR1B = (1 << CS12) | (0 << CS11) | (1 << CS10);
    _delay_ms(100);
}

bool is_pressed(uint8_t index)
{
    for (uint16_t i = 0; i < DEBOUNCE_NUM_SAMPLES; i++)
    {
        if (((button_samples[i] >> index) & 1) == 0)
        {
            return false;
        }
    }
    return true;
}

bool is_released(uint8_t index)
{
    for (uint16_t i = 0; i < DEBOUNCE_NUM_SAMPLES; i++)
    {
        if (((button_samples[i] >> index) & 1) == 1)
        {
            return false;
        }
    }
    return true;
}

bool is_any_key_pressed()
{
    uint8_t counter = NUM_BUTTONS;
    bool is_pressed[NUM_BUTTONS] = { [0 ... (NUM_BUTTONS - 1)] = true };
    for (uint16_t i = 0; i < DEBOUNCE_NUM_SAMPLES; i++)
    {
        for (uint8_t j = 0; j < NUM_BUTTONS; j++)
        {
            if (((button_samples[i] >> j) & 1) == 0)
            {
                if (is_pressed[j])
                {
                    counter--;
                }
                is_pressed[j] = false;
            }
        }
    }
    return (counter > 0);
}

void wait_for_no_input()
{
    while (true)
    {
        for (uint16_t i = 0; i < DEBOUNCE_NUM_SAMPLES; i++)
        {
            if (button_samples[i] != 0)
            {
                continue;
            }
        }
        break;
    }
}
