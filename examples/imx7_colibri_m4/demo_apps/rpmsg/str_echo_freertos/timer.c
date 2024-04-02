#include "board.h"
#include "debug_console_imx.h"
#include "gpt.h"
#include "timer.h"

#if ENABLE_GPT3 == 1
void Hw_Timer_Init(void)
{
    gpt_init_config_t config = {
        .freeRun    = false,
        .waitEnable = true,
        .stopEnable = true,
        .dozeEnable = true,
        .dbgEnable  = false,
        .enableMode = true
    };
    /* Initialize GPT module */
    GPT_Init(BOARD_GPTA_BASEADDR, &config);
    /* Set GPT clock source to 24M OSC */
    GPT_SetClockSource(BOARD_GPTA_BASEADDR, gptClockSourceOsc);
    /* Set GPT interrupt priority 3 */
    NVIC_SetPriority(BOARD_GPTA_IRQ_NUM, 3);
    /* Enable NVIC interrupt */
    NVIC_EnableIRQ(BOARD_GPTA_IRQ_NUM);         

    uint64_t counter = 24000ULL * 1500; /* First get the counter needed by delay time */
    uint32_t high;
    uint32_t div24m, div;
    /* Get the value that exceed maximum register counter */
    high = (uint32_t)(counter >> 32);
    /* high could not exceed 24000, so that predivider is enough */
    div24m = high / 4096; /* We need PRESCALER24M only if high exceed PRESCALER maximum value */
    div = high / (div24m + 1); /* Get PRESCALER value */
    /* Now set prescaler */
    GPT_SetOscPrescaler(BOARD_GPTA_BASEADDR, div24m);
    GPT_SetPrescaler(BOARD_GPTA_BASEADDR, div);
    /* Set GPT compare value */
    GPT_SetOutputCompareValue(BOARD_GPTA_BASEADDR, gptOutputCompareChannel1,
                              (uint32_t)(counter / (div24m + 1) / (div + 1)));

    /* Enable GPT Output Compare1 interrupt */
    GPT_SetIntCmd(BOARD_GPTA_BASEADDR, gptStatusFlagOutputCompare1, true);
    /* GPT start */
    GPT_Enable(BOARD_GPTA_BASEADDR);
}

void BOARD_GPTA_HANDLER(void)
{
    static uint32_t myCounter = 0;

    PRINTF(" time base interrupt %d\r\n", myCounter++);

    PRINTF("rdc_mda1: %.8x\r\n", GPIO_DR_REG(((GPIO_Type *)0x303d0204)));
    PRINTF("rdc_pdap15: %.8x\r\n", GPIO_DR_REG(((GPIO_Type *)0x303d043c)));
    PRINTF("rdc_semaphore1_gate15: %.8x\r\n", GPIO_DR_REG(((GPIO_Type *)0x303b000f)));

    PRINTF("rdc_pdap10: %.8x\r\n", GPIO_DR_REG(((GPIO_Type *)0x303d0428)));
    PRINTF("rdc_semaphore1_gate10: %.8x\r\n", GPIO_DR_REG(((GPIO_Type *)0x303b000a)));
    
    GPT_ClearStatusFlag(BOARD_GPTA_BASEADDR, gptStatusFlagOutputCompare1);
}

#endif