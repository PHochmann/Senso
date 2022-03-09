#pragma once
#include <stdint.h>

#define NUM_BUTTONS 4

void buttons_init();
uint8_t get_input();
void wait_for_no_input();
uint8_t get_timer_capture();
