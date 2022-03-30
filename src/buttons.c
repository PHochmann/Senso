#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdint.h>
#include "buttons.h"

#ifdef SENSOCARD
    #define BUTTON_PORT PORTA
    #define BUTTON_DDR  DDRA
    #define BUTTON_PIN  PINA
    const uint8_t buttons[NUM_BUTTONS] = { 4, 5, 6, 7 };
#else
    #define BUTTON_PORT PORTB
    #define BUTTON_DDR  DDRB
    #define BUTTON_PIN  PINB
    const uint8_t buttons[NUM_BUTTONS] = { 2, 3, 4, 5 };
#endif

uint8_t button_mask;
uint8_t timer_capture;

ISR (PCINT0_vect)
{
    timer_capture = TCNT1;
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

    // Speciality: Button presses are used to get random values
    // Enable pin state change interrupt for button pins
    #ifdef SENSOCARD
        GIMSK = 1 << PCIE0;
    #else
        PCICR = 1 << PCIE0;
    #endif
    
    PCMSK0 = (1 << PCINT7 | 1 << PCINT6 | 1 << PCINT5 | 1 << PCINT4);
    sei();
    // Start timer to get random values, no prescaler
    TCCR1A = 0;
    TCCR1B = (0 << CS12) | (0 << CS11) | (1 << CS10);
}

uint8_t get_input()
{
    return (~BUTTON_PIN & button_mask) >> buttons[0];
}

void wait_for_no_input()
{
    while (get_input() != 0);
}