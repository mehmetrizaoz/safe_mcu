#include "rpmsg/rpmsg_rtos.h"
#include "FreeRTOS.h"
#include "task.h"
#include "board.h"
#include "mu_imx.h"
#include "uart_imx.h"
#include "debug_console_imx.h"
#include "watchdog.h"
#include "timer.h"
#include "MCIMX7D/include/MCIMX7D_M4.h"
#include "gpio.h"
#include "comm_skb.h"
#include "semphr.h"

#define APP_TASK_STACK_SIZE 256
#define APP_MU_IRQ_PRIORITY 3

void BOARD_MU_HANDLER(void);

int main(void){    
    hardware_init();
    skb_comm_semaphore_init();
    
#if ENABLE_WDT == 1
    wdt_init();
#endif
    GPIO_Ctrl_InitLedPin();
    MU_Init(BOARD_MU_BASE_ADDR);
    NVIC_SetPriority(BOARD_MU_IRQ_NUM, APP_MU_IRQ_PRIORITY);
    NVIC_EnableIRQ(BOARD_MU_IRQ_NUM);

    PRINTF("Bootiwwwwng... \n\r");
    // PRINTF("test:%d\r\n", test_function(5,6));
    // xTaskCreate(get_a7_data, "GET_RPMSG_DATA", APP_TASK_STACK_SIZE, NULL, tskIDLE_PRIORITY+1, NULL);
    xTaskCreate(ledToggle, "LED_TOGGLE", APP_TASK_STACK_SIZE, NULL, tskIDLE_PRIORITY+1, NULL);
    
#if ENABLE_WDT == 1
    xTaskCreate(wdt_refresher, "RESET_WTG", APP_TASK_STACK_SIZE, NULL, tskIDLE_PRIORITY+1, NULL);
#endif

    xTaskCreate(fill_skb_rx_buf, "GET_UART_DATA", APP_TASK_STACK_SIZE, NULL, tskIDLE_PRIORITY+2, NULL);
    xTaskCreate(parse_skb_rx_buf, "GET_SKB_PACKAGES", APP_TASK_STACK_SIZE, NULL, tskIDLE_PRIORITY+1, NULL);
    
    xTaskCreate(send_data_to_skb, "SEND_SKB_PACKAGES", APP_TASK_STACK_SIZE, NULL, tskIDLE_PRIORITY+1, NULL);

    vTaskStartScheduler();
    while (true);
}

void BOARD_MU_HANDLER(void){
    // PRINTF("MU HANDLER\r\n");
    rpmsg_handler();
}
