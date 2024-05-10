#include "board.h"

#define GPIO_DEBOUNCE_DELAY    (100000)

#define ROTARY_ENCODER_LEFT  0x31
#define ROTARY_ENCODER_RIGHT 0x32
#define ROTARY_ENCODER_PRESS 0x33

extern uint8_t rotary_encoder_event_for_skb;
extern uint8_t rotary_encoder_event_for_a7;

extern void init_gpio(void);
extern void task_ledToggle(void *pvParameters);
extern void read_rotary_encoder();
