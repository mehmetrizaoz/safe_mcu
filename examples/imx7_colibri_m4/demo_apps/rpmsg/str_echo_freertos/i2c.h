
#include "stdint.h"
#include "board.h"

typedef struct _led
{
    uint8_t port;
    uint8_t pin;
}led;

extern led led1;
extern led led2;
extern led led3;
extern led led4;
extern led led5;
extern led led6;

extern uint8_t port0_switches;
extern uint8_t port1_switches;

extern bool switch_event_for_skb;
extern bool rotary_switch_event_for_skb;
extern bool switch_event_for_a7;
extern bool rotary_switch_event_for_a7;

extern uint8_t rotary_switch_p2, rotary_switch_p1, rotary_switch_p0;

extern void read_switches(void);
extern void read_rotary_switch(void);
extern void read_rotary_switch(void);
extern void init_tca6424();
extern void task_button_card_process(void *pvParameters);
