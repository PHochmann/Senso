#pragma once
#include <stdint.h>
#include <stdbool.h>

#define NUM_BUTTONS 4

void buttons_init();
uint8_t get_timer_capture();
bool is_pressed(uint8_t index);
bool is_released(uint8_t index);
void wait_for_no_input();
bool is_any_key_pressed();
