#include "gpio.h"
#include "gpio_pins.h"
#include "board.h"
#include "debug_console_imx.h"
#include "comm_skb.h"

static bool on = false;

void GPIO_Ctrl_InitLedPin(void){   
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
}

void ledToggle(void *pvParameters){
    uint32_t i, j, debounce;

#if ENABLE_GPT3==1    
    static uint8_t flag = 0;
#endif

    for (;;){
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
        on = !on;

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

#if ENABLE_GPT3==1
        if (flag == 0){
            PRINTF("timer initialize\r\n");
            Hw_Timer_Init();
            flag = 1;
        }
#endif
    }
}