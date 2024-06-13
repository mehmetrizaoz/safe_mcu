#include "board.h"
#include "debug_console_imx.h"
#include "gpt.h"
#include "timer.h"
#include "MCIMX7D_M4.h"


#include "gpio.h"
#include "gpio_pins.h"
#include "debug_console_imx.h"
#include "comm_skb.h"
#include "timer.h"
#include "MCIMX7D_M4.h"
#include "device_imx.h"
#include "pin_mux.h"
#include "debug_console_imx.h"


#if ENABLE_GPT3 == 1

void Hw_Timer_Init(void)
{
    RDC_SetPdapAccess(RDC, BOARD_GPTA_RDC_PDAP, 0xff, false, false);
    CCM_UpdateRoot(CCM, BOARD_GPTA_CCM_ROOT, ccmRootmuxGptOsc24m, 0, 0);
    CCM_EnableRoot(CCM, BOARD_GPTA_CCM_ROOT);
    CCM_ControlGate(CCM, BOARD_GPTA_CCM_CCGR, ccmClockNeededRunWait);

    uint32_t freqA;
    gpt_init_config_t config = {
        .freeRun     = false,
        .waitEnable  = false,
        .stopEnable  = false,
        .dozeEnable  = false,
        .dbgEnable   = false,
        .enableMode  = true
    };

    GPT_Init(BOARD_GPTA_BASEADDR, &config);
    GPT_SetClockSource(BOARD_GPTA_BASEADDR, gptClockSourceOsc);
    GPT_SetOscPrescaler(BOARD_GPTA_BASEADDR, 1);
    GPT_SetPrescaler(BOARD_GPTA_BASEADDR, 1);

    freqA = 93;
    GPT_SetOutputCompareValue(BOARD_GPTA_BASEADDR, gptOutputCompareChannel1, freqA);
    NVIC_SetPriority(BOARD_GPTA_IRQ_NUM, 3);
    NVIC_EnableIRQ(BOARD_GPTA_IRQ_NUM);
    GPT_SetIntCmd(BOARD_GPTA_BASEADDR, gptStatusFlagOutputCompare1, true);
    GPT_Enable(BOARD_GPTA_BASEADDR);
}


// uint32_t arr[20] = {65536, 63796, 58764, 50972, 41248, 30624, 20228, 11162, 4390, 630, 280, 3379, 9597, 18275, 28491, 39160, 49151, 57403, 63041};

void BOARD_GPTA_HANDLER(void)
{
    /*
    static uint32_t index = 0;
    // static uint32_t counter = 0

    if(((I2S_Type *)I2S2_BASE)->TCSR & I2S_TCSR_FEF(1)){
        ((I2S_Type *)I2S2_BASE)->TCSR |= I2S_TCSR_FEF(1);
        // PRINTF("clear FEF\r\n");
    }

    // if(counter < 32000){
        // counter++;
        ((I2S_Type *)I2S2_BASE)->TCR3 |= I2S_TCR3_TCE(1);    
        ((I2S_Type *)I2S2_BASE)->TDR[0] = arr[index];

        if(index == 19)
            index = 0;
        else
            index++;
    // }
    */
    

    // PRINTF("%d\r\n", myCounter++);
    // CCM_ANALOG_PLL_AUDIO |= CCM_ANALOG_PLL_AUDIO_POWERDOWN_MASK;
    // PRINTF("rdc_mda1: %.8x\r\n", GPIO_DR_REG(((GPIO_Type *)0x303d0204)));
    // PRINTF("rdc_pdap15: %.8x\r\n", GPIO_DR_REG(((GPIO_Type *)0x303d043c)));
    // PRINTF("rdc_semaphore1_gate15: %.8x\r\n", GPIO_DR_REG(((GPIO_Type *)0x303b000f)));
    // PRINTF("rdc_pdap10: %.8x\r\n", GPIO_DR_REG(((GPIO_Type *)0x303d0428)));
    // PRINTF("rdc_semaphore1_gate10: %.8x\r\n", GPIO_DR_REG(((GPIO_Type *)0x303b000a)));
    GPT_ClearStatusFlag(BOARD_GPTA_BASEADDR, gptStatusFlagOutputCompare1);
}

#endif