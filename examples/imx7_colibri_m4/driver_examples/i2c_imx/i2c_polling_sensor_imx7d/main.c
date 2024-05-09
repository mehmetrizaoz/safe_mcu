#include <stdint.h>
#include <stdbool.h>
#include "board.h"
#include "debug_console_imx.h"
#include "i2c_imx.h"
#include "gpio_imx.h"
#include "gpio_pins.h"

typedef struct _led
{
    uint8_t port;
    uint8_t pin;
}led;

led led1 = {.pin = 0x20, .port = 1};
led led2 = {.pin = 0x40, .port = 1};
led led3 = {.pin = 0x80, .port = 1};
led led4 = {.pin = 0x01, .port = 2};
led led5 = {.pin = 0x02, .port = 2};
led led6 = {.pin = 0x04, .port = 2};

bool encoder_input_a;
bool encoder_input_b;
static uint8_t txBuffer[5];
static uint8_t rxBuffer[7];
static uint8_t cmdBuffer[5];
uint8_t port1_leds = 0;
uint8_t port2_leds = 0;

static bool I2C_MasterSendDataPolling(I2C_Type *base,
                                      const uint8_t *cmdBuff,
                                      uint32_t cmdSize,
                                      const uint8_t *txBuff,
                                      uint32_t txSize);

static bool I2C_MasterReceiveDataPolling(I2C_Type *base,
                                         const uint8_t *cmdBuff,
                                         uint32_t cmdSize,
                                         uint8_t *rxBuff,
                                         uint32_t rxSize);
uint8_t b0, b1;
void read_buttons(void){
    cmdBuffer[0] = BOARD_I2C_TCA6424_1_ADDR << 1;
    cmdBuffer[1] = 0x80; //with auto increment
    cmdBuffer[2] = (BOARD_I2C_TCA6424_1_ADDR << 1) + 1;
    I2C_MasterReceiveDataPolling(BOARD_I2C_BASEADDR, cmdBuffer, 3, rxBuffer, 3);    
    // PRINTF("\tbuttons:%08b %08b\r\n", rxBuffer[0], rxBuffer[1] & 0x07);

    if(b0 != rxBuffer[0] || b1 != rxBuffer[1]){
        PRINTF("expander buttons:%08b %08b\r\n", rxBuffer[0], rxBuffer[1] & 0x07);
        b0 = rxBuffer[0];
        b1 = rxBuffer[1];        
    }

    for(long i=0; i<50000; i++);
}

uint8_t p2, p1, p0;
void read_rotary_switch(void){
    cmdBuffer[0] = BOARD_I2C_TCA6424_2_ADDR << 1;
    cmdBuffer[1] = 0x80; //with auto increment
    cmdBuffer[2] = (BOARD_I2C_TCA6424_2_ADDR << 1) + 1;
    I2C_MasterReceiveDataPolling(BOARD_I2C_BASEADDR, cmdBuffer, 3, rxBuffer, 3);    
    
    if(p0 != rxBuffer[0] || p1 != rxBuffer[1] || p2 != rxBuffer[2]){
        PRINTF("rotary switch:\t%08b %08b %08b\r\n", rxBuffer[0], rxBuffer[1], rxBuffer[2]); //expander
        p0 = rxBuffer[0];
        p1 = rxBuffer[1];
        p2 = rxBuffer[2];
    }

    // for(long i=0; i<20000000; i++);
    for(long i=0; i<100000; i++);
}

uint8_t eb;
void read_rotary_encoder(){
    if(eb != GPIO_ReadPinInput(BOARD_GPIO_ENCODER_BTN->base, BOARD_GPIO_ENCODER_BTN->pin)){
        PRINTF("encoder button :%d\r\n", GPIO_ReadPinInput(BOARD_GPIO_ENCODER_BTN->base, BOARD_GPIO_ENCODER_BTN->pin));
        eb = GPIO_ReadPinInput(BOARD_GPIO_ENCODER_BTN->base, BOARD_GPIO_ENCODER_BTN->pin);
    }

    if(GPIO_ReadPinInput(BOARD_GPIO_ENCODER_INPUT_A->base, BOARD_GPIO_ENCODER_INPUT_A->pin) == gpioPinSet){
        if(encoder_input_a == false){
            if(GPIO_ReadPinInput(BOARD_GPIO_ENCODER_INPUT_B->base, BOARD_GPIO_ENCODER_INPUT_B->pin) == gpioPinClear){
                PRINTF("3 TURNED LEFT\r\n");
            }
            else{
                PRINTF("4 TURNED RIGHT\r\n");
            }
        }
        encoder_input_a = true;
    }
    else{        
        if (encoder_input_a == true){
            if(GPIO_ReadPinInput(BOARD_GPIO_ENCODER_INPUT_B->base, BOARD_GPIO_ENCODER_INPUT_B->pin) == gpioPinSet){
                PRINTF("1 TURNED LEFT\r\n");
            }
            else{
                PRINTF("2 TURNED RIGHT\r\n");
            }
        }
        encoder_input_a = false;
    }

    if(GPIO_ReadPinInput(BOARD_GPIO_ENCODER_INPUT_B->base, BOARD_GPIO_ENCODER_INPUT_B->pin) == gpioPinSet){
        if(encoder_input_b == false){
            if(GPIO_ReadPinInput(BOARD_GPIO_ENCODER_INPUT_A->base, BOARD_GPIO_ENCODER_INPUT_A->pin) == gpioPinSet){
                PRINTF("7 TURNED LEFT\r\n");
            }
            else{
                PRINTF("8 TURNED RIGHT\r\n");
            }
        }
        encoder_input_b = true;
    }
    else{        
        if (encoder_input_b == true){
            if(GPIO_ReadPinInput(BOARD_GPIO_ENCODER_INPUT_A->base, BOARD_GPIO_ENCODER_INPUT_A->pin) == gpioPinClear){
                PRINTF("5 TURNED LEFT\r\n");
            }
            else{
                PRINTF("6 TURNED RIGHT\r\n");
            }
        }
        encoder_input_b = false;
    }
}

uint8_t mmm = 0;
void setPin(led led, bool value){
    uint8_t port_leds;
//-----------------------------------------------------    
    cmdBuffer[0] = BOARD_I2C_TCA6424_1_ADDR << 1;
    cmdBuffer[1] = 0x8C; //with auto increment
    cmdBuffer[2] = (BOARD_I2C_TCA6424_1_ADDR << 1) + 1;
    I2C_MasterReceiveDataPolling(BOARD_I2C_BASEADDR, cmdBuffer, 3, rxBuffer, 3);
    if(rxBuffer[0] == 0xff && rxBuffer[1] == 0x1f && rxBuffer[2] == 0xc0){
        PRINTF(".");
        if(mmm == 1){
            PRINTF("aaa");
        }
    }
    else{
        mmm = 1;
        PRINTF("%.2x %.2x %.2x\r\n", rxBuffer[0], rxBuffer[1], rxBuffer[2]);
        cmdBuffer[0] = BOARD_I2C_TCA6424_1_ADDR << 1;
        cmdBuffer[1] = 0x8C;
        cmdBuffer[2] = 0xFF; //0-7 input
        cmdBuffer[3] = 0x1F; //0-4 pins input, 5-7 pins output
        cmdBuffer[4] = 0xC0; //0-5 output, 6-7 input
        I2C_MasterSendDataPolling(BOARD_I2C_BASEADDR, cmdBuffer, 5, txBuffer, 0);        
    }  
//-----------------------------------------------------
    if(led.port == 1){
        port_leds = port1_leds;
    }
    else{
        port_leds = port2_leds;
    }
    PRINTF("port: %d pin: %d value: %08b \r\n", led.port, led.pin, port_leds);

    cmdBuffer[0] = BOARD_I2C_TCA6424_1_ADDR << 1;
    cmdBuffer[1] = 0x04 + led.port;
    if(value == true){
        cmdBuffer[2] = port_leds | led.pin;
    }
    else{        
        cmdBuffer[2] = port_leds & (~led.pin);
    }
    
    port_leds = cmdBuffer[2];
    I2C_MasterSendDataPolling(BOARD_I2C_BASEADDR, cmdBuffer, 3, txBuffer, 0);

    if(led.port == 1){
        port1_leds = port_leds;
    }
    else{
        port2_leds = port_leds;
    }

    for(long i=0; i<20000000; i++);
}

void init_tca6424(){
    i2c_init_config_t i2cInitConfig = {
        .baudRate     = 400000u,
        .slaveAddress = 0x00
    };   
    
    i2cInitConfig.clockRate = get_i2c_clock_freq(BOARD_I2C_BASEADDR);
    I2C_Init(BOARD_I2C_BASEADDR, &i2cInitConfig);
    I2C_Enable(BOARD_I2C_BASEADDR);    
    GPIO_WritePinOutput(BOARD_GPIO_I2C2_EN->base, BOARD_GPIO_I2C2_EN->pin, gpioPinSet);
    for(long i=0; i<5000; i++);

    cmdBuffer[0] = BOARD_I2C_TCA6424_1_ADDR << 1;
    cmdBuffer[1] = 0x8C;
    cmdBuffer[2] = 0xFF; //0-7 input
    cmdBuffer[3] = 0x1F; //0-4 pins input, 5-7 pins output
    cmdBuffer[4] = 0xC0; //0-5 output, 6-7 input
    I2C_MasterSendDataPolling(BOARD_I2C_BASEADDR, cmdBuffer, 5, txBuffer, 0);
    for(long i=0; i<5000; i++);

    cmdBuffer[0] = BOARD_I2C_TCA6424_1_ADDR << 1;
    cmdBuffer[1] = 0x84;
    cmdBuffer[2] = 0x00;//
    cmdBuffer[3] = 0x00;
    cmdBuffer[4] = 0x00;
    I2C_MasterSendDataPolling(BOARD_I2C_BASEADDR, cmdBuffer, 5, txBuffer, 0);
    for(long i=0; i<5000; i++);

    cmdBuffer[0] = BOARD_I2C_TCA6424_2_ADDR << 1;
    cmdBuffer[1] = 0x8C;
    cmdBuffer[2] = 0xFF; //all ports input
    cmdBuffer[3] = 0xFF; 
    cmdBuffer[4] = 0xFF;
    I2C_MasterSendDataPolling(BOARD_I2C_BASEADDR, cmdBuffer, 5, txBuffer, 0);
    for(long i=0; i<5000; i++);
}

void init_rotary_encoder(){
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

int main(void){
    hardware_init();
    init_tca6424();
    init_rotary_encoder();
    while(1){
        setPin(led1, true);
        setPin(led2, true);
        setPin(led3, true);
        setPin(led4, true);
        setPin(led5, true);
        setPin(led6, true);
        setPin(led1, false);
        setPin(led2, false);        
        setPin(led3, false);                  
        setPin(led4, false);        
        setPin(led5, false);        
        setPin(led6, false);
        read_buttons();
        read_rotary_switch();
        read_rotary_encoder();
    }
}

static bool I2C_MasterSendDataPolling(I2C_Type *base,
                                      const uint8_t *cmdBuff,
                                      uint32_t cmdSize,
                                      const uint8_t *txBuff,
                                      uint32_t txSize)
{
    if (I2C_GetStatusFlag(base, i2cStatusBusBusy))
        return false;

    /* Set I2C work under Tx mode */
    I2C_SetDirMode(base, i2cDirectionTransmit);

    /* Switch to Master Mode and Send Start Signal. */
    I2C_SetWorkMode(base, i2cModeMaster);

    /* Send first byte */
    if (0 != cmdSize)
    {
        I2C_WriteByte(base, *cmdBuff++);
        cmdSize--;
    }
    else
    {
        I2C_WriteByte(base, *txBuff++);
        txSize--;
    }

    while (1){
        /* Wait I2C transmission status flag assert.  */
        while (!I2C_GetStatusFlag(base, i2cStatusInterrupt));

        /* Clear I2C transmission status flag. */
        I2C_ClearStatusFlag(base, i2cStatusInterrupt);

        /* Transmit complete. */
        if ((I2C_GetStatusFlag(base, i2cStatusReceivedAck)) ||
            ((0 == txSize) && (0 == cmdSize)))
        {
            /* Switch to Slave mode and Generate a Stop Signal. */
            I2C_SetWorkMode(base, i2cModeSlave);

            /* Switch back to Rx direction. */
            I2C_SetDirMode(base, i2cDirectionReceive);
            // PRINTF("...\r\n");
            return true;
        }
        else
        {
            if (0 != cmdSize)
            {
                I2C_WriteByte(base, *cmdBuff++);
                cmdSize--;
            }
            else
            {
                I2C_WriteByte(base, *txBuff++);
                txSize--;
            }
        }
    }
}

static bool I2C_MasterReceiveDataPolling(I2C_Type *base,
                                         const uint8_t *cmdBuff,
                                         uint32_t cmdSize,
                                         uint8_t *rxBuff,
                                         uint32_t rxSize)
{
    uint32_t currentDir = i2cDirectionReceive;

    /* Clear I2C interrupt flag to avoid spurious interrupt */
    I2C_ClearStatusFlag(base, i2cStatusInterrupt);

    if (I2C_GetStatusFlag(base, i2cStatusBusBusy))
    {
        return false;
    }

    /* Set I2C work under Tx mode */
    I2C_SetDirMode(base, i2cDirectionTransmit);

    /* Switch to Master Mode and Send Start Signal. */
    I2C_SetWorkMode(base, i2cModeMaster);

    if (0 != cmdSize)
    {
        currentDir = i2cDirectionTransmit;
        if (1 == cmdSize)
            I2C_SendRepeatStart(base);
        I2C_WriteByte(base, *cmdBuff++);
        cmdSize--;
    }
    else
    {
        /* Change to receive state. */
        I2C_SetDirMode(base, i2cDirectionReceive);

        if (1 == rxSize)
            /* Send Nack */
            I2C_SetAckBit(base, false);
        else
            /* Send Ack */
            I2C_SetAckBit(base, true);
        /* dummy read to clock in 1st byte */
        *rxBuff = I2C_ReadByte(base);
    }

    while (1)
    {
        /* Wait I2C transmission status flag assert.  */
        while (!I2C_GetStatusFlag(base, i2cStatusInterrupt));

        /* Clear I2C transmission status flag. */
        I2C_ClearStatusFlag(base, i2cStatusInterrupt);

        if (i2cDirectionTransmit == currentDir)
        {
            if (0 < cmdSize)
            {
                if (I2C_GetStatusFlag(base, i2cStatusReceivedAck))
                {
                    /* Switch to Slave mode and Generate a Stop Signal. */
                    I2C_SetWorkMode(base, i2cModeSlave);

                    /* Switch back to Rx direction. */
                    I2C_SetDirMode(base, i2cDirectionReceive);
                    return false;
                }
                else
                {
                    if (1 == cmdSize)
                        I2C_SendRepeatStart(base);
                    I2C_WriteByte(base, *cmdBuff++);
                    cmdSize--;
                }
            }
            else
            {
                /* Change to receive state. */
                I2C_SetDirMode(base, i2cDirectionReceive);
                currentDir = i2cDirectionReceive;

                if (1 == rxSize)
                    /* Send Nack */
                    I2C_SetAckBit(base, false);
                else
                    /* Send Ack */
                    I2C_SetAckBit(base, true);
                /* dummy read to clock in 1st byte */
                *rxBuff = I2C_ReadByte(base);
            }
        }
        else
        {
            /* Normal read operation. */
            if (2 == rxSize)
                /* Send Nack */
                I2C_SetAckBit(base, false);
            else
                /* Send Nack */
                I2C_SetAckBit(base, true);

            if (1 == rxSize)
                /* Switch back to Tx direction to avoid additional I2C bus read. */
                I2C_SetDirMode(base, i2cDirectionTransmit);
            *rxBuff = I2C_ReadByte(base);
            rxBuff++;
            rxSize--;

            /* receive finished. */
            if (0 == rxSize)
            {
                /* Switch to Slave mode and Generate a Stop Signal. */
                I2C_SetWorkMode(base, i2cModeSlave);

                /* Switch back to Rx direction. */
                I2C_SetDirMode(base, i2cDirectionReceive);

                return true;
            }
        }
    }
}
