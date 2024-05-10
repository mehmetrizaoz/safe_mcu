#include "gpio.h"
#include "gpio_pins.h"
#include "debug_console_imx.h"
#include "comm_skb.h"
#include "timer.h"

static bool on = false;
uint8_t encoder_button = 1;
bool encoder_input_a = 1;
bool encoder_input_b = 0;
uint8_t rotary_encoder_event_for_skb = 0;
uint8_t rotary_encoder_event_for_a7 = 0;

void init_gpio(void){   
    gpio_init_config_t ledCtrlInitConfig = {
        .pin = BOARD_GPIO_LEDCTRL_CONFIG->pin,
        .direction = gpioDigitalOutput,
        .interruptMode = gpioNoIntmode
    };
    // PRINTF("ledCtrlInitConfig.muxReg %.8x\r\n", BOARD_GPIO_LEDCTRL_CONFIG->muxReg);
    // PRINTF("ledCtrlInitConfig.padReg %.8x\r\n", BOARD_GPIO_LEDCTRL_CONFIG->padReg);
    GPIO_Init(BOARD_GPIO_LEDCTRL_CONFIG->base, &ledCtrlInitConfig);  

    gpio_init_config_t led1InitConfig = {
        .pin = BOARD_GPIO_LED1_CONFIG->pin,
        .direction = gpioDigitalOutput,
        .interruptMode = gpioNoIntmode
    };
    // PRINTF("led1InitConfig.muxReg %.8x\r\n", BOARD_GPIO_LED1_CONFIG->muxReg);
    // PRINTF("led1InitConfig.padReg %.8x\r\n", BOARD_GPIO_LED1_CONFIG->padReg);
    GPIO_Init(BOARD_GPIO_LED1_CONFIG->base, &led1InitConfig);

    gpio_init_config_t keyInitConfig = {
        .pin = BOARD_GPIO_KEY_CONFIG->pin,
        .direction = gpioDigitalInput,
        .interruptMode = gpioNoIntmode,
    };
    // PRINTF("keyInitConfig.muxReg %.8x\r\n", BOARD_GPIO_KEY_CONFIG->muxReg);
    // PRINTF("keyInitConfig.padReg %.8x\r\n", BOARD_GPIO_KEY_CONFIG->padReg);
    GPIO_Init(BOARD_GPIO_KEY_CONFIG->base, &keyInitConfig);

    //button card encoder pins
    gpio_init_config_t rotary_encoder_pin = {
        .pin = BOARD_GPIO_ENCODER_BTN->pin,
        .direction = gpioDigitalInput,
        .interruptMode = gpioNoIntmode
    };
    GPIO_Init(BOARD_GPIO_ENCODER_BTN->base, &rotary_encoder_pin); 

    gpio_init_config_t rotary_encoder_enable_pin = {
        .pin = BOARD_GPIO_ENCODER_READ_ENABLE->pin,
        .direction = gpioDigitalOutput,
        .interruptMode = gpioNoIntmode
    };
    GPIO_Init(BOARD_GPIO_ENCODER_READ_ENABLE->base, &rotary_encoder_enable_pin);
    GPIO_WritePinOutput(BOARD_GPIO_ENCODER_READ_ENABLE->base, BOARD_GPIO_ENCODER_READ_ENABLE->pin, gpioPinSet);

    gpio_init_config_t encoder_input_b = {
        .pin = BOARD_GPIO_ENCODER_INPUT_B->pin,
        .direction = gpioDigitalInput,
        .interruptMode = gpioNoIntmode
    };
    GPIO_Init(BOARD_GPIO_ENCODER_INPUT_B->base, &encoder_input_b); 

    gpio_init_config_t encoder_input_a = {
        .pin = BOARD_GPIO_ENCODER_INPUT_A->pin,
        .direction = gpioDigitalInput,
        .interruptMode = gpioNoIntmode
    };
    GPIO_Init(BOARD_GPIO_ENCODER_INPUT_A->base, &encoder_input_a); 
}

void read_rotary_encoder(){
    // PRINTF("%d", GPIO_ReadPinInput(BOARD_GPIO_ENCODER_BTN->base, BOARD_GPIO_ENCODER_BTN->pin));
    // PRINTF("%d", GPIO_ReadPinInput(BOARD_GPIO_ENCODER_INPUT_A->base, BOARD_GPIO_ENCODER_INPUT_A->pin));
    // PRINTF("%d\r\n", GPIO_ReadPinInput(BOARD_GPIO_ENCODER_INPUT_B->base, BOARD_GPIO_ENCODER_INPUT_B->pin));

    if(encoder_button != GPIO_ReadPinInput(BOARD_GPIO_ENCODER_BTN->base, BOARD_GPIO_ENCODER_BTN->pin)){
        encoder_button = GPIO_ReadPinInput(BOARD_GPIO_ENCODER_BTN->base, BOARD_GPIO_ENCODER_BTN->pin);
        if(encoder_button == 0 /*pressed*/){
            rotary_encoder_event_for_skb = ROTARY_ENCODER_PRESS;
            rotary_encoder_event_for_a7 = ROTARY_ENCODER_PRESS;
        }
    }

    if(GPIO_ReadPinInput(BOARD_GPIO_ENCODER_INPUT_A->base, BOARD_GPIO_ENCODER_INPUT_A->pin) == gpioPinSet){
        if(encoder_input_a == false){
            if(GPIO_ReadPinInput(BOARD_GPIO_ENCODER_INPUT_B->base, BOARD_GPIO_ENCODER_INPUT_B->pin) == gpioPinClear){
                rotary_encoder_event_for_skb = ROTARY_ENCODER_LEFT;
                rotary_encoder_event_for_a7 = ROTARY_ENCODER_LEFT;
            }
            else{
                rotary_encoder_event_for_skb = ROTARY_ENCODER_RIGHT;
                rotary_encoder_event_for_a7 = ROTARY_ENCODER_RIGHT;
            }
        }
        encoder_input_a = true;
    }
    else{        
        if (encoder_input_a == true){
            if(GPIO_ReadPinInput(BOARD_GPIO_ENCODER_INPUT_B->base, BOARD_GPIO_ENCODER_INPUT_B->pin) == gpioPinSet){
                rotary_encoder_event_for_skb = ROTARY_ENCODER_LEFT;
                rotary_encoder_event_for_a7 = ROTARY_ENCODER_LEFT;
            }
            else{
                rotary_encoder_event_for_skb = ROTARY_ENCODER_RIGHT;
                rotary_encoder_event_for_a7 = ROTARY_ENCODER_RIGHT;
            }
        }
        encoder_input_a = false;
    }

    if(GPIO_ReadPinInput(BOARD_GPIO_ENCODER_INPUT_B->base, BOARD_GPIO_ENCODER_INPUT_B->pin) == gpioPinSet){
        if(encoder_input_b == false){
            if(GPIO_ReadPinInput(BOARD_GPIO_ENCODER_INPUT_A->base, BOARD_GPIO_ENCODER_INPUT_A->pin) == gpioPinSet){
                rotary_encoder_event_for_skb = ROTARY_ENCODER_LEFT;
                rotary_encoder_event_for_a7 = ROTARY_ENCODER_LEFT;
            }
            else{
                rotary_encoder_event_for_skb = ROTARY_ENCODER_RIGHT;
                rotary_encoder_event_for_a7 = ROTARY_ENCODER_RIGHT;
            }
        }
        encoder_input_b = true;
    }
    else{        
        if (encoder_input_b == true){
            if(GPIO_ReadPinInput(BOARD_GPIO_ENCODER_INPUT_A->base, BOARD_GPIO_ENCODER_INPUT_A->pin) == gpioPinClear){
                rotary_encoder_event_for_skb = ROTARY_ENCODER_LEFT;
                rotary_encoder_event_for_a7 = ROTARY_ENCODER_LEFT;
            }
            else{
                rotary_encoder_event_for_skb = ROTARY_ENCODER_RIGHT;
                rotary_encoder_event_for_a7 = ROTARY_ENCODER_RIGHT;
            }
        }
        encoder_input_b = false;
    }
}

void task_ledToggle(void *pvParameters){
    uint32_t i, j, debounce;
    for (;;){
        read_rotary_encoder();
        //todo: send encoder changes to a7
        //todo: send encoder changes to skb
        vTaskDelay(20);
        /*
        do{
            debounce = 0;
            while (0 == GPIO_ReadPinInput(BOARD_GPIO_KEY_CONFIG->base, BOARD_GPIO_KEY_CONFIG->pin));

            for (i = 0; i < 3; i++){
                for (j = 0 ; j < GPIO_DEBOUNCE_DELAY; j++){
                    __NOP();
                }

                if (1 == GPIO_ReadPinInput(BOARD_GPIO_KEY_CONFIG->base, BOARD_GPIO_KEY_CONFIG->pin)){
                    debounce++;
                }
            }

            if (debounce > 2){
                break;
            }
        }
        while (1);

        do{
            debounce = 0;
            while (1 == GPIO_ReadPinInput(BOARD_GPIO_KEY_CONFIG->base, BOARD_GPIO_KEY_CONFIG->pin));

            for (i = 0; i < 3; i++){
                for (j = 0 ; j < GPIO_DEBOUNCE_DELAY; j++){
                    __NOP();
                }

                if (0 == GPIO_ReadPinInput(BOARD_GPIO_KEY_CONFIG->base, BOARD_GPIO_KEY_CONFIG->pin)){
                    debounce++;
                }
            }

            if (debounce > 2){
                break;
            }
        }
        while (1);        
        
        GPIO_WritePinOutput(BOARD_GPIO_LEDCTRL_CONFIG->base, BOARD_GPIO_LEDCTRL_CONFIG->pin, gpioPinClear);
        GPIO_WritePinOutput(BOARD_GPIO_LED1_CONFIG->base, BOARD_GPIO_LED1_CONFIG->pin, on ? gpioPinSet : gpioPinClear);
        on = !on;*/

        // PRINTF("data_count:%d\r\n", data_count);
        // PRINTF("skb_rec_buf_rptr:%d\r\n", skb_rec_buf_rptr);
        // PRINTF("skb_rec_buf_wptr:%d\r\n", skb_rec_buf_wptr);
        // for(int i=0; i<MAX_BUF_SIZE;i ++){
        //     PRINTF("%.x ", skb_rec_buf[i]);
        // }
        // PRINTF("skb_tx_buf_len: %d\r\n", skb_tx_buf_len);

        // for(int i=0; i<MAX_BUF_SIZE; i++){
        //     debug_putchar(skb_tx_buf[i]);
        // }        

        // PRINTF("\r\n");
    }
}