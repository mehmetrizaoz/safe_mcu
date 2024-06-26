/*
 * Copyright (c) 2015, Freescale Semiconductor, Inc.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * o Redistributions of source code must retain the above copyright notice, this list
 *   of conditions and the following disclaimer.
 *
 * o Redistributions in binary form must reproduce the above copyright notice, this
 *   list of conditions and the following disclaimer in the documentation and/or
 *   other materials provided with the distribution.
 *
 * o Neither the name of Freescale Semiconductor, Inc. nor the names of its
 *   contributors may be used to endorse or promote products derived from this
 *   software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
 * ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "clock_freq.h"
#include "ccm_imx7d.h"
#include "ccm_analog_imx7d.h"

/*FUNCTION**********************************************************************
 *
 * Function Name : get_gpt_clock_freq
 * Description   : Get clock frequency applys to the GPT module
 *
 *END**************************************************************************/
uint32_t get_gpt_clock_freq(GPT_Type* base)
{
    uint32_t root;
    uint32_t hz;
    uint32_t pre, post;

    switch((uint32_t)base)
    {
        case GPT3_BASE:
            root = CCM_GetRootMux(CCM, ccmRootGpt3);
            CCM_GetRootDivider(CCM, ccmRootGpt3, &pre, &post);
            break;
        case GPT4_BASE:
            root = CCM_GetRootMux(CCM, ccmRootGpt4);
            CCM_GetRootDivider(CCM, ccmRootGpt4, &pre, &post);
            break;
        default:
            return 0;
    }

    switch(root)
    {
        case ccmRootmuxGptOsc24m:
            hz = 24000000;
            break;
        case ccmRootmuxGptSysPllPfd0:
            hz = CCM_ANALOG_GetPfdFreq(CCM_ANALOG, ccmAnalogPfd0Frac);
            break;
        default:
            return 0;
    }

    return hz / (pre + 1) / (post + 1);
}

/*FUNCTION**********************************************************************
 *
 * Function Name : get_ecspi_clock_freq
 * Description   : Get clock frequency applys to the ECSPI module
 *
 *END**************************************************************************/
uint32_t get_ecspi_clock_freq(ECSPI_Type* base)
{
    uint32_t root;
    uint32_t hz;
    uint32_t pre, post;

    switch((uint32_t)base)
    {
        case ECSPI1_BASE:
            root = CCM_GetRootMux(CCM, ccmRootEcspi1);
            CCM_GetRootDivider(CCM, ccmRootEcspi1, &pre, &post);
            break;
        case ECSPI2_BASE:
            root = CCM_GetRootMux(CCM, ccmRootEcspi2);
            CCM_GetRootDivider(CCM, ccmRootEcspi2, &pre, &post);
            break;
        case ECSPI3_BASE:
            root = CCM_GetRootMux(CCM, ccmRootEcspi3);
            CCM_GetRootDivider(CCM, ccmRootEcspi3, &pre, &post);
            break;
        default:
            return 0;
    }

    switch(root)
    {
        case ccmRootmuxEcspiOsc24m:
            hz = 24000000;
            break;
        case ccmRootmuxEcspiSysPllPfd4:
            hz = CCM_ANALOG_GetPfdFreq(CCM_ANALOG, ccmAnalogPfd4Frac);
            break;
        default:
            return 0;
    }

    return hz / (pre + 1) / (post + 1);
}

/*FUNCTION**********************************************************************
 *
 * Function Name : get_flexcan_clock_freq
 * Description   : Get clock frequency applys to the FLEXCAN module
 *
 *END**************************************************************************/
uint32_t get_flexcan_clock_freq(CAN_Type* base)
{
    uint32_t root;
    uint32_t hz;
    uint32_t pre, post;

    switch((uint32_t)base)
    {
        case CAN1_BASE:
            root = CCM_GetRootMux(CCM, ccmRootCan1);
            CCM_GetRootDivider(CCM, ccmRootCan1, &pre, &post);
            break;
        case CAN2_BASE:
            root = CCM_GetRootMux(CCM, ccmRootCan2);
            CCM_GetRootDivider(CCM, ccmRootCan2, &pre, &post);
            break;
        default:
            return 0;
    }

    switch(root)
    {
        case ccmRootmuxCanOsc24m:
            hz = 24000000;
            break;
        case ccmRootmuxCanSysPllDiv4:
            hz = CCM_ANALOG_GetSysPllFreq(CCM_ANALOG) >> 2;
            break;
        case ccmRootmuxCanSysPllDiv1:
            hz = CCM_ANALOG_GetSysPllFreq(CCM_ANALOG);
            break;
        default:
            return 0;
    }

    return hz / (pre + 1) / (post + 1);
}

/*FUNCTION**********************************************************************
 *
 * Function Name : get_I2C_clock_freq
 * Description   : Get clock frequency applys to the I2C module
 *
 *END**************************************************************************/
uint32_t get_i2c_clock_freq(I2C_Type* base)
{
    uint32_t root;
    uint32_t hz;
    uint32_t pre, post;

    switch((uint32_t)base)
    {
        case I2C1_BASE:
            root = CCM_GetRootMux(CCM, ccmRootI2c1);
            CCM_GetRootDivider(CCM, ccmRootI2c1, &pre, &post);
            break;
        case I2C2_BASE:
            root = CCM_GetRootMux(CCM, ccmRootI2c2);
            CCM_GetRootDivider(CCM, ccmRootI2c2, &pre, &post);
            break;
        case I2C3_BASE:
            root = CCM_GetRootMux(CCM, ccmRootI2c3);
            CCM_GetRootDivider(CCM, ccmRootI2c3, &pre, &post);
            break;
        case I2C4_BASE:
            root = CCM_GetRootMux(CCM, ccmRootI2c4);
            CCM_GetRootDivider(CCM, ccmRootI2c4, &pre, &post);
            break;
        default:
            return 0;
    }

    switch(root)
    {
        case ccmRootmuxI2cOsc24m:
            hz = 24000000;
            break;
        case ccmRootmuxI2cSysPllDiv4:
            hz = CCM_ANALOG_GetSysPllFreq(CCM_ANALOG) >> 2;
            break;
        default:
            return 0;
    }

    return hz / (pre + 1) / (post + 1);
}

/*FUNCTION**********************************************************************
 *
 * Function Name : get_uart_clock_freq
 * Description   : Get clock frequency feeding into the UART module
 *
 *END**************************************************************************/
uint32_t get_uart_clock_freq(UART_Type* base)
{
    uint32_t root;
    uint32_t hz;
    uint32_t pre, post;
    uint32_t ccmRoot;
    
    switch((uint32_t)base)
    {
        case UART1_BASE: ccmRoot = ccmRootUart1; break;
        case UART2_BASE: ccmRoot = ccmRootUart2; break;
        case UART3_BASE: ccmRoot = ccmRootUart3; break;
        case UART4_BASE: ccmRoot = ccmRootUart4; break;
        case UART5_BASE: ccmRoot = ccmRootUart5; break;
        case UART6_BASE: ccmRoot = ccmRootUart6; break;
        case UART7_BASE: ccmRoot = ccmRootUart7; break;
        default:         return 0;
    }

    root = CCM_GetRootMux(CCM, ccmRoot);
    CCM_GetRootDivider(CCM, ccmRoot, &pre, &post);
    
    switch(root)
    {
        case ccmRootmuxUartOsc24m:
            hz = 24000000;
            break;
        case ccmRootmuxUartSysPllDiv2:
            hz = CCM_ANALOG_GetSysPllFreq(CCM_ANALOG) >> 1;
            break;
        case ccmRootmuxUartSysPllDiv1:
            hz = CCM_ANALOG_GetSysPllFreq(CCM_ANALOG);
            break;
        default:
            return 0;
    }

    return hz / (pre + 1) / (post + 1);
}

/*******************************************************************************
 * EOF
 ******************************************************************************/
