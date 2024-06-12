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
    PRINTF("555\r\n");
    CCM_UpdateRoot(CCM, BOARD_GPTA_CCM_ROOT, ccmRootmuxGptOsc24m, 0, 0);
    CCM_EnableRoot(CCM, BOARD_GPTA_CCM_ROOT);
    CCM_ControlGate(CCM, BOARD_GPTA_CCM_CCGR, ccmClockNeededRunWait);

    uint32_t freqA;
    gpt_init_config_t config = {
        .freeRun     = false,
        .waitEnable  = true,
        .stopEnable  = true,
        .dozeEnable  = true,
        .dbgEnable   = false,
        .enableMode  = true
    };

    GPT_Init(BOARD_GPTA_BASEADDR, &config);
    GPT_SetClockSource(BOARD_GPTA_BASEADDR, gptClockSourceOsc);
    GPT_SetOscPrescaler(BOARD_GPTA_BASEADDR, 1);
    GPT_SetPrescaler(BOARD_GPTA_BASEADDR, 1);

    freqA = 800 / 2;
    freqA /= 2;

    GPT_SetOutputCompareValue(BOARD_GPTA_BASEADDR, gptOutputCompareChannel1, freqA);
    NVIC_SetPriority(BOARD_GPTA_IRQ_NUM, 3);
    NVIC_EnableIRQ(BOARD_GPTA_IRQ_NUM);
    GPT_SetIntCmd(BOARD_GPTA_BASEADDR, gptStatusFlagOutputCompare1, true);
    GPT_Enable(BOARD_GPTA_BASEADDR);
}

void BOARD_GPTA_HANDLER(void)
{
    static uint32_t myCounter = 0;
    static bool on = false;
    GPIO_WritePinOutput(BOARD_GPIO_LEDCTRL_CONFIG->base, BOARD_GPIO_LEDCTRL_CONFIG->pin, gpioPinClear);
    GPIO_WritePinOutput(BOARD_GPIO_LED1_CONFIG->base, BOARD_GPIO_LED1_CONFIG->pin, on ? gpioPinSet : gpioPinClear);
    on = !on;


    ((I2S_Type *)I2S2_BASE)->TCR3 |= I2S_TCR3_TCE(1);
    for(int i=0; i<32; i++)
        ((I2S_Type *)I2S2_BASE)->TDR[0] = myCounter++;

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