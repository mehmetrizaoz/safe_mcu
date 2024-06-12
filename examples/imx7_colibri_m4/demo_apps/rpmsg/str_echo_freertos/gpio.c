#include "gpio.h"
#include "gpio_pins.h"
#include "debug_console_imx.h"
#include "comm_skb.h"
#include "timer.h"
#include "MCIMX7D_M4.h"
#include "device_imx.h"
#include "pin_mux.h"
#include "debug_console_imx.h"

static bool on = false;
uint8_t encoder_button = 1;
bool encoder_input_a = 1;
bool encoder_input_b = 0;
uint8_t rotary_encoder_event_for_skb = 0;
uint8_t rotary_encoder_event_for_a7 = 0;

void init_gpio(void){   
    i2s_init(I2S2_BASE);

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

void i2s_init(I2S_Type *base){
    base->TCSR = 0;
    base->TCSR |= I2S_TCSR_SR_MASK;  //softwae reset
    base->TCSR |= I2S_TCSR_TE(0);  //transmit disable
    base->TCSR |= I2S_TCSR_STOPE(0); //transmitter disabled in stop mode
    base->TCSR |= I2S_TCSR_DBGE(0);  //transmitter disabled in debug mode  
    base->TCSR |= I2S_TCSR_BCE(1); //transmit bit clock enable
    base->TCSR |= I2S_TCSR_FR(1);  //fifo reset    
    base->TCSR &= ~I2S_TCSR_SR_MASK;  //
    base->TCSR |= I2S_TCSR_WSIE(0); 
    base->TCSR |= I2S_TCSR_SEIE(0);
    base->TCSR |= I2S_TCSR_FEIE(0);
    base->TCSR |= I2S_TCSR_FWIE(0);
    base->TCSR |= I2S_TCSR_FWDE(0);
    base->TCSR |= I2S_TCSR_FRDE(0);

    base->TCR1 = 0;   
    base->TCR1 |= I2S_TCR1_TFW(15); //transmit fifo watermark

    base->TCR2 = 0;
    base->TCR2 |= I2S_TCR2_SYNC(0); //asynchronous mode
    base->TCR2 |= I2S_TCR2_BCS(0);  //use the normal bit clock source
    base->TCR2 |= I2S_TCR2_BCI(0);  //no effect
    base->TCR2 |= I2S_TCR2_MSEL(0); //master clock 0-3
    base->TCR2 |= I2S_TCR2_BCP(1);  //rising edge bit clock polarity
    base->TCR2 |= I2S_TCR2_BCD(1);  //bit clock is generated in master mode   
    base->TCR2 |= I2S_TCR2_DIV(110);//divide audio master clock to (0xff + 1) * 2

    base->TCR3 = 0;
    base->TCR3 |= I2S_TCR3_TCE(1);  //transmit data channel n is enabled
    base->TCR3 |= I2S_TCR3_WDFL(0); //which word sets the start of word flag - 1   

    base->TCR4 = 0;
    base->TCR4 |= I2S_TCR4_FRSZ(0); //number of words in each frame - 1
    base->TCR4 |= I2S_TCR4_SYWD(15);//length of the frame sync in number of bit clocks - 1
    base->TCR4 |= I2S_TCR4_MF(1);   //msb transmitted first
    // base->TCR4 |= I2S_TCR4_FSE(0);  //
    base->TCR4 |= I2S_TCR4_FSP(0);  //frame sync is active high
    base->TCR4 |= I2S_TCR4_FSD(1);  //frame sync is generated in master mode    
        
    base->TCR5 = 0;
    base->TCR5 |= I2S_TCR5_WNW(31);  //number of bits in first word - 1
    base->TCR5 |= I2S_TCR5_W0W(31);  //number of bits in each word - 1
    base->TCR5 |= I2S_TCR5_FBT(0x1f);//sai first bit shifted, msb first

    base->TMR  = 0; //no masking
    base->TCSR |= I2S_TCSR_TE(1); //transmitter enable  
    base->TCR3 |= I2S_TCR3_TCE(1);
}

void fill_sai(I2S_Type *base){    
    PRINTF("TFR  : %.4x\r\n", base->TFR[0]);
    PRINTF("TCSR : %.4x\r\n", base->TCSR);
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
        // read_rotary_encoder();
        fill_sai(I2S2_BASE);


        //todo: send encoder changes to a7
        //todo: send encoder changes to skb
        vTaskDelay(1000);

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