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
#include "i2c.h"

#define APP_TASK_STACK_SIZE 256
#define APP_MU_IRQ_PRIORITY 3

void BOARD_MU_HANDLER(void);

int main(void){    
    hardware_init();

#if ENABLE_GPT3 == 1    
    Hw_Timer_Init();
#endif

    skb_comm_semaphore_init();
    
#if ENABLE_WDT == 1
    wdt_init();
#endif    
    
    init_gpio();
    MU_Init(BOARD_MU_BASE_ADDR);
    NVIC_SetPriority(BOARD_MU_IRQ_NUM, APP_MU_IRQ_PRIORITY);
    NVIC_EnableIRQ(BOARD_MU_IRQ_NUM);    

    // PRINTF("test____________:%d\r\n", test_function(5,6));

    init_tca6424();

#if ENABLE_WDT == 1
    xTaskCreate(wdt_refresher, "RESET_WTG", APP_TASK_STACK_SIZE, NULL, tskIDLE_PRIORITY+1, NULL);
#endif
    xTaskCreate(task_ledToggle, "LED_TOGGLE", APP_TASK_STACK_SIZE, NULL, tskIDLE_PRIORITY+1, NULL);
    xTaskCreate(task_button_card_process, "BTN_CARD", APP_TASK_STACK_SIZE, NULL, tskIDLE_PRIORITY+2, NULL);
    xTaskCreate(task_fill_skb_rx_buf, "GET_UART_DATA", APP_TASK_STACK_SIZE, NULL, tskIDLE_PRIORITY+2, NULL);
    xTaskCreate(task_parse_skb_rx_buf, "GET_SKB_PACKAGES", APP_TASK_STACK_SIZE, NULL, tskIDLE_PRIORITY+1, NULL);
    xTaskCreate(task_send_data_to_skb, "SEND_SKB_PACKAGES", APP_TASK_STACK_SIZE, NULL, tskIDLE_PRIORITY+1, NULL);

    PRINTF("Boot.. \n\r");
    vTaskStartScheduler();
    while (true);
}

void BOARD_MU_HANDLER(void){
    rpmsg_handler();
}

