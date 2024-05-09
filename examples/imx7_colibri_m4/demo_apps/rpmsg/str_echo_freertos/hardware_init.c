#include "board.h"
#include "pin_mux.h"
#include "gpt.h"
#include "clock_freq.h"
#include "uart_imx.h"
#include "gpio_imx.h"
#include "gpio_pins.h"

void hardware_init(void)
{
    BOARD_RdcInit();
    BOARD_ClockInit(); 

    RDC_SetPdapAccess(RDC, BOARD_DEBUG_UART_RDC_PDAP, 0xFF, false, false);
    CCM_UpdateRoot(CCM, BOARD_DEBUG_UART_CCM_ROOT, ccmRootmuxUartOsc24m, 0, 0);
    CCM_EnableRoot(CCM, BOARD_DEBUG_UART_CCM_ROOT);
    CCM_ControlGate(CCM, BOARD_DEBUG_UART_CCM_CCGR, ccmClockNeededAll);
    configure_uart_pins(BOARD_DEBUG_UART_BASEADDR);
    DbgConsole_Init(BOARD_DEBUG_UART_BASEADDR, get_uart_clock_freq(BOARD_DEBUG_UART_BASEADDR), 115200, uartModemModeDte);

    // PRINTF("uart6_urxd: %.8x\r\n", GPIO_DR_REG(((GPIO_Type *)0x30a80000)));
    // PRINTF("uart6_utxd: %.8x\r\n", GPIO_DR_REG(((GPIO_Type *)0x30a80040)));
    // PRINTF("uart6_ucr1: %.8x\r\n", GPIO_DR_REG(((GPIO_Type *)0x30a80080)));
    // PRINTF("uart6_ucr2: %.8x\r\n", GPIO_DR_REG(((GPIO_Type *)0x30a80084)));
    // PRINTF("uart6_ucr3: %.8x\r\n", GPIO_DR_REG(((GPIO_Type *)0x30a80088)));
    // PRINTF("uart6_ucr4: %.8x\r\n", GPIO_DR_REG(((GPIO_Type *)0x30a8008c)));
    // PRINTF("uart6_ufcr: %.8x\r\n", GPIO_DR_REG(((GPIO_Type *)0x30a80090)));
    // PRINTF("uart6_usr1: %.8x\r\n", GPIO_DR_REG(((GPIO_Type *)0x30a80094)));
    // PRINTF("uart6_usr2: %.8x\r\n", GPIO_DR_REG(((GPIO_Type *)0x30a80098)));
    // PRINTF("uart6_uesc: %.8x\r\n", GPIO_DR_REG(((GPIO_Type *)0x30a8009c)));
    // PRINTF("uart6_utim: %.8x\r\n", GPIO_DR_REG(((GPIO_Type *)0x30a800a0)));
    // PRINTF("uart6_ubir: %.8x\r\n", GPIO_DR_REG(((GPIO_Type *)0x30a800a4)));
    // PRINTF("uart6_ubmr: %.8x\r\n", GPIO_DR_REG(((GPIO_Type *)0x30a800a8)));
    // PRINTF("uart6_ubrc: %.8x\r\n", GPIO_DR_REG(((GPIO_Type *)0x30a800ac)));
    // PRINTF("uart6_unems: %.8x\r\n", GPIO_DR_R./b EG(((GPIO_Type *)0x30a800b0)));
    // PRINTF("uart6_uts: %.8x\r\n", GPIO_DR_REG(((GPIO_Type *)0x30a800b4)));
    // PRINTF("uart6_umrc: %.8x\r\n", GPIO_DR_REG(((GPIO_Type *)0x30a800b8)));

    RDC_SetPdapAccess(RDC, BOARD_WDOG_RDC_PDAP, 3 << (BOARD_DOMAIN_ID * 2), false, false);
    RDC_SetPdapAccess(RDC, BOARD_MU_RDC_PDAP, 3 << (BOARD_DOMAIN_ID * 2), false, false);
    CCM_ControlGate(CCM, BOARD_MU_CCM_CCGR, ccmClockNeededRun);

    RDC_SetPdapAccess(RDC, BOARD_I2C_RDC_PDAP, 3 << (BOARD_DOMAIN_ID * 2), false, false);
    CCM_UpdateRoot(CCM, BOARD_I2C_CCM_ROOT, ccmRootmuxI2cOsc24m, 0, 0);
    CCM_EnableRoot(CCM, BOARD_I2C_CCM_ROOT);
    CCM_ControlGate(CCM, BOARD_I2C_CCM_CCGR, ccmClockNeededRunWait);
    configure_i2c_pins(BOARD_I2C_BASEADDR);

    gpio_init_config_t ledCtrlInitConfig = {
        .pin = BOARD_GPIO_I2C2_EN->pin,
        .direction = gpioDigitalOutput,
        .interruptMode = gpioNoIntmode
    };
    GPIO_Init(BOARD_GPIO_I2C2_EN->base, &ledCtrlInitConfig); 
}

/*******************************************************************************
 * EOF
 ******************************************************************************/
