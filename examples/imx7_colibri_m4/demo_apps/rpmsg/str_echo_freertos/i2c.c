#include "FreeRTOS.h"
#include "semphr.h"
#include "i2c.h"
#include "i2c_imx.h"
#include "gpio_pins.h"

led led1 = {.pin = 0x20, .port = 1};
led led2 = {.pin = 0x40, .port = 1};
led led3 = {.pin = 0x80, .port = 1};
led led4 = {.pin = 0x01, .port = 2};
led led5 = {.pin = 0x02, .port = 2};
led led6 = {.pin = 0x04, .port = 2};

uint8_t port1_leds = 0;
uint8_t port2_leds = 0;

bool switch_event = false;
bool rotary_switch_event = false;
uint8_t rotary_switch_p2, rotary_switch_p1, rotary_switch_p0;

uint8_t port0_switches = 0;
uint8_t port1_switches = 0;

static uint8_t txBuffer[5];
static uint8_t rxBuffer[7];
static uint8_t cmdBuffer[5];

static SemaphoreHandle_t xSemaphore_i2c;

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

void task_button_card_process(void *pvParameters){
    for (;;){
        read_switches();
        read_rotary_switch();
        vTaskDelay(50);
    }
}

void init_tca6424(){
    xSemaphore_i2c = xSemaphoreCreateBinary();
    xSemaphoreGive(xSemaphore_i2c);

    i2c_init_config_t i2cInitConfig = {
        .baudRate     = 400000u,
        .slaveAddress = 0x00
    };   
    
    i2cInitConfig.clockRate = get_i2c_clock_freq(BOARD_I2C_BASEADDR);
    I2C_Init(BOARD_I2C_BASEADDR, &i2cInitConfig);
    I2C_Enable(BOARD_I2C_BASEADDR);    
    GPIO_WritePinOutput(BOARD_GPIO_I2C2_EN->base, BOARD_GPIO_I2C2_EN->pin, gpioPinSet);
    for(long i=0; i<1000; i++);

    xSemaphoreTake(xSemaphore_i2c, portMAX_DELAY);

    cmdBuffer[0] = BOARD_I2C_TCA6424_1_ADDR << 1;
    cmdBuffer[1] = 0x8C;
    cmdBuffer[2] = 0xFF; //0-7 input
    cmdBuffer[3] = 0x1F; //0-4 pins input, 5-7 pins output
    cmdBuffer[4] = 0xC0; //0-5 output, 6-7 input
    I2C_MasterSendDataPolling(BOARD_I2C_BASEADDR, cmdBuffer, 5, txBuffer, 0);
    for(long i=0; i<1000; i++);

    cmdBuffer[0] = BOARD_I2C_TCA6424_1_ADDR << 1;
    cmdBuffer[1] = 0x84;
    cmdBuffer[2] = 0x00;//
    cmdBuffer[3] = 0x00;
    cmdBuffer[4] = 0x00;
    I2C_MasterSendDataPolling(BOARD_I2C_BASEADDR, cmdBuffer, 5, txBuffer, 0);
    for(long i=0; i<1000; i++);

    cmdBuffer[0] = BOARD_I2C_TCA6424_2_ADDR << 1;
    cmdBuffer[1] = 0x8C;
    cmdBuffer[2] = 0xFF; //all ports input
    cmdBuffer[3] = 0xFF; 
    cmdBuffer[4] = 0xFF;
    I2C_MasterSendDataPolling(BOARD_I2C_BASEADDR, cmdBuffer, 5, txBuffer, 0);
    for(long i=0; i<1000; i++);

    xSemaphoreGive(xSemaphore_i2c);
}

void read_rotary_switch(void){
    xSemaphoreTake(xSemaphore_i2c, portMAX_DELAY);

    cmdBuffer[0] = BOARD_I2C_TCA6424_2_ADDR << 1;
    cmdBuffer[1] = 0x80; //with auto increment
    cmdBuffer[2] = (BOARD_I2C_TCA6424_2_ADDR << 1) + 1;
    I2C_MasterReceiveDataPolling(BOARD_I2C_BASEADDR, cmdBuffer, 3, rxBuffer, 3);    
    if(rotary_switch_p0 != (rxBuffer[0] | 0x01) || rotary_switch_p1 != rxBuffer[1] || rotary_switch_p2 != rxBuffer[2]){        
        rotary_switch_p0 = rxBuffer[0] | 0x01;
        rotary_switch_p1 = rxBuffer[1];
        rotary_switch_p2 = rxBuffer[2];
        rotary_switch_event = true;
        // PRINTF("rotary switch:\t%08b %08b %08b\r\n", rotary_switch_p0, rotary_switch_p1, rotary_switch_p2); //expander
    }    
    for(long i=0; i<1000; i++);
    xSemaphoreGive(xSemaphore_i2c);
}

void read_switches(void){
    xSemaphoreTake(xSemaphore_i2c, portMAX_DELAY);
    cmdBuffer[0] = BOARD_I2C_TCA6424_1_ADDR << 1;
    cmdBuffer[1] = 0x80; //with auto increment
    cmdBuffer[2] = (BOARD_I2C_TCA6424_1_ADDR << 1) + 1;
    I2C_MasterReceiveDataPolling(BOARD_I2C_BASEADDR, cmdBuffer, 3, rxBuffer, 3);
    if( port0_switches != rxBuffer[0] || port1_switches != (rxBuffer[1] & 0x07) ){        
        port0_switches = rxBuffer[0];
        port1_switches = rxBuffer[1] & 0x07;
        if(port1_switches != 0 || port0_switches != 0){
            switch_event = true;
        }
        // PRINTF("sw's:%08b %08b\r\n", port0_switches, port1_switches);
    }
    for(long i=0; i<1000; i++);
    xSemaphoreGive(xSemaphore_i2c);
}

void setPin(led led, bool value){
    uint8_t port_leds;
    xSemaphoreTake(xSemaphore_i2c, portMAX_DELAY);
//-----------------------------------------------------    
    // cmdBuffer[0] = BOARD_I2C_TCA6424_1_ADDR << 1;
    // cmdBuffer[1] = 0x8C; //with auto increment
    // cmdBuffer[2] = (BOARD_I2C_TCA6424_1_ADDR << 1) + 1;
    // I2C_MasterReceiveDataPolling(BOARD_I2C_BASEADDR, cmdBuffer, 3, rxBuffer, 3);
    // if(rxBuffer[0] == 0xff && rxBuffer[1] == 0x1f && rxBuffer[2] == 0xc0){
    //     PRINTF(".");
    //     if(mmm == 1){
    //         PRINTF("aaa");
    //     }
    // }
    // else{
    //     for(long i=0; i<1000; i++);
    //     mmm = 1;
    //     cmdBuffer[0] = BOARD_I2C_TCA6424_1_ADDR << 1;
    //     cmdBuffer[1] = 0x8C;
    //     cmdBuffer[2] = 0xFF; //0-7 input
    //     cmdBuffer[3] = 0x1F; //0-4 pins input, 5-7 pins output
    //     cmdBuffer[4] = 0xC0; //0-5 output, 6-7 input
    //     I2C_MasterSendDataPolling(BOARD_I2C_BASEADDR, cmdBuffer, 5, txBuffer, 0);        
    // }  
    // for(long i=0; i<1000; i++);
//-----------------------------------------------------
    if(led.port == 1){
        port_leds = port1_leds;
    }
    else{
        port_leds = port2_leds;
    }    

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

    for(long i=0; i<1000; i++);
    xSemaphoreGive(xSemaphore_i2c);
}

bool I2C_MasterSendDataPolling(I2C_Type *base,
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

bool I2C_MasterReceiveDataPolling(I2C_Type *base,
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
