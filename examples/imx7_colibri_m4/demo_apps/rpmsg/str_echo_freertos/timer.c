#include "board.h"
#include "debug_console_imx.h"
#include "gpt.h"
#include "timer.h"

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

    freqA = 24000000 / 2;
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
    // PRINTF("%d\r\n", myCounter++);
    // PRINTF("rdc_mda1: %.8x\r\n", GPIO_DR_REG(((GPIO_Type *)0x303d0204)));
    // PRINTF("rdc_pdap15: %.8x\r\n", GPIO_DR_REG(((GPIO_Type *)0x303d043c)));
    // PRINTF("rdc_semaphore1_gate15: %.8x\r\n", GPIO_DR_REG(((GPIO_Type *)0x303b000f)));
    // PRINTF("rdc_pdap10: %.8x\r\n", GPIO_DR_REG(((GPIO_Type *)0x303d0428)));
    // PRINTF("rdc_semaphore1_gate10: %.8x\r\n", GPIO_DR_REG(((GPIO_Type *)0x303b000a)));
    GPT_ClearStatusFlag(BOARD_GPTA_BASEADDR, gptStatusFlagOutputCompare1);
}

#endif