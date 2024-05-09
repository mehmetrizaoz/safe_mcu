#include "board.h"

#define GPIO_DEBOUNCE_DELAY    (100000)

#define ROTARY_ENCODER_LEFT  1
#define ROTARY_ENCODER_RIGHT 2
#define ROTARY_ENCODER_PRESS 4

extern uint8_t rotary_encoder_event;

extern void init_gpio(void);
extern void task_ledToggle(void *pvParameters);
extern void read_rotary_encoder();
