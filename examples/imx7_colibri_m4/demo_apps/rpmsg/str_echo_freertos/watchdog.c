#include "board.h"
#include "debug_console_imx.h"
#include "wdog_imx.h"
#include "watchdog.h"

#if ENABLE_WDT == 1

static uint32_t wdt_counter = 5; /* This case will cause M4 reset after 5.5 seconds */

void wdt_init(void)
{
    wdog_init_config_t config = {
       .wdw   = false,    /*!< true: suspend in low power wait, false: not suspend */
       .wdt   = true,     /*!< true: assert WDOG_B when timeout, false: not assert WDOG_B */
       .wdbg  = true,     /*!< true: suspend in debug mode, false: not suspend */
       .wdzst = false     /*!< true: suspend in doze and stop mode, false: not suspend */
    };

    WDOG_Init(BOARD_WDOG_BASEADDR, &config);

    /* Enable WDOG interrupt 0.5 second before WDOG timeout */
    NVIC_SetPriority(BOARD_WDOG_IRQ_NUM, 3);
    NVIC_EnableIRQ(BOARD_WDOG_IRQ_NUM);
    /* Refresh WDOG to reload counter */
    WDOG_Refresh(BOARD_WDOG_BASEADDR);
    WDOG_EnableInt(BOARD_WDOG_BASEADDR, 1);

    PRINTF("WDOG with timeout 1.5 seconds will now start\n\r");
    /* Enable WDOG with timeout 1.5 second */
    WDOG_Enable(BOARD_WDOG_BASEADDR, 2);

    wdt_counter = 5;
}

void BOARD_WDOG_HANDLER()
{
    WDOG_ClearStatusFlag(BOARD_WDOG_BASEADDR);
    
    if (--wdt_counter){
        PRINTF("wdt refreshed\r\n");
        WDOG_Refresh(BOARD_WDOG_BASEADDR);
    }
}

void wdt_refresher(void *pvParameters)
{
    for (;;){
        wdt_counter = 5;
        vTaskDelay(100);
    }
}

#endif

