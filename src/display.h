#include <stdint.h>

extern const uint8_t display_lookup[10];

// Normal use
void display_init();
void display_show_number(uint8_t number);
void display_clear();

// Used to display something else manually
void display_send(uint8_t bits);
void display_show();
