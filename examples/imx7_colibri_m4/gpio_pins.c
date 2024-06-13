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

#include <assert.h>
#include "gpio_pins.h"
#include "board.h"
#include "gpio_imx.h"

gpio_config_t gpio_7_11 = { //pin 99 - speaker_sdn
    "ENC_BTN",                          /* name */
    (volatile uint32_t *)(0x30330258),  /* muxReg */
    5,                                  /* muxConfig */
    (volatile uint32_t *)(0x303304c8),  /* padReg */
    0,                                  /* padConfig */
    (volatile uint32_t *)(0x30260000),  /* base */
    11                                   /* pin */
};

gpio_config_t gpio_6_21 = { //pin 32 - speaker_gain_en
    "ENC_BTN",                          /* name */
    (volatile uint32_t *)(0x30330224),  /* muxReg */
    5,                                  /* muxConfig */
    (volatile uint32_t *)(0x30330494),  /* padReg */
    0,                                  /* padConfig */
    (volatile uint32_t *)(0x30250000),  /* base */
    21                                   /* pin */
};

gpio_config_t gpio_5_11 = { //pin 98 - speaker_gain_c
    "ENC_BTN",                          /* name */
    (volatile uint32_t *)(0x303301b4),  /* muxReg */
    5,                                  /* muxConfig */
    (volatile uint32_t *)(0x30330424),  /* padReg */
    0,                                  /* padConfig */
    (volatile uint32_t *)(0x30240000),  /* base */
    11                                   /* pin */
};

gpio_config_t gpio_1_14 = { //pin 188 - speaker_gain_b
    "ENC_BTN",                          /* name */
    (volatile uint32_t *)(0x3033002c),  /* muxReg */
    0,                                  /* muxConfig */
    (volatile uint32_t *)(0x30330284),  /* padReg */
    0,                                  /* padConfig */
    (volatile uint32_t *)(0x30200000),  /* base */
    14                                   /* pin */
};

gpio_config_t gpio_1_15 = { //pin 178 - speaker_gain_a
    "ENC_BTN",                          /* name */
    (volatile uint32_t *)(0x30330030),  /* muxReg */
    0,                                  /* muxConfig */
    (volatile uint32_t *)(0x30330288),  /* padReg */
    0,                                  /* padConfig */
    (volatile uint32_t *)(0x30200000),  /* base */
    15                                   /* pin */
};

//-------------------------------------

gpio_config_t gpio_rotary_encoder_input_a = {
    "ENC_BTN",                          /* name */
    (volatile uint32_t *)(0x303300b0),  /* muxReg */
    5,                                  /* muxConfig */
    (volatile uint32_t *)(0x30330320),  /* padReg */
    0,                                  /* padConfig */
    (volatile uint32_t *)(0x30210000),  /* base */
    31                                   /* pin */
};

gpio_config_t gpio_rotary_encoder_input_b = {
    "ENC_BTN",                          /* name */
    (volatile uint32_t *)(0x303300ac),  /* muxReg */
    5,                                  /* muxConfig */
    (volatile uint32_t *)(0x3033031c),  /* padReg */
    0,                                  /* padConfig */
    (volatile uint32_t *)(0x30210000),  /* base */
    30                                   /* pin */
};

gpio_config_t gpio_rotary_encoder_read_enable_output = {
    "ENC_BTN",                          /* name */
    (volatile uint32_t *)(0x30330084),  /* muxReg */
    5,                                  /* muxConfig */
    (volatile uint32_t *)(0x303302f4),  /* padReg */
    0,                                  /* padConfig */
    (volatile uint32_t *)(0x30210000),  /* base */
    20                                  /* pin */
};

gpio_config_t gpio_rotary_encoder_button = {
    "ENC_BTN",                          /* name */
    (volatile uint32_t *)(0x3033004c),  /* muxReg */
    5,                                  /* muxConfig */
    (volatile uint32_t *)(0x303302bc),  /* padReg */
    0,                                  /* padConfig */
    (volatile uint32_t *)(0x30210000),  /* base */
    6                                   /* pin */
};

gpio_config_t gpioI2c2En = {
    "EXT_IO0 I2CEN",                    /* name */
    (volatile uint32_t *)(0x3033003c),  /* muxReg */
    5,                                  /* muxConfig */
    (volatile uint32_t *)(0x303302ac),  /* padReg */
    0,                                  /* padConfig */
    (volatile uint32_t *)(0x30210000),  /* base */
    2                                   /* pin */
};

gpio_config_t gpioLedCtrl = {
    "EXT_IO0 LEDCTRL",                  /* name */
    (volatile uint32_t *)(0x30330234),  /* muxReg */
    5,                                  /* muxConfig */
    (volatile uint32_t *)(0x303304a4),  /* padReg */
    0,                                  /* padConfig */
    (volatile uint32_t *)(0x30260000),  /* base */
    2                                   /* pin */
};

gpio_config_t gpioLed_1 = {
    "EXT_IO0 LED1",                      /* name */
    (volatile uint32_t *)(0x30330184),  /* muxReg */
    5,                                  /* muxConfig */
    (volatile uint32_t *)(0x303303f4),  /* padReg */
    0,                                  /* padConfig */
    (volatile uint32_t *)(0x30230000),  /* base */
    23                                   /* pin */
};

gpio_config_t gpioKeyFunc1 = {
    "EXT_IO1",                          /* name */
    (volatile uint32_t *)(0x30330054),  /* muxReg */
    5,                                  /* muxConfig */
    (volatile uint32_t *)(0x303302c4),  /* padReg */
    0x010000000,                        /* padConfig */
    (volatile uint32_t *)(0x30210000),  /* base */
    8                                   /* pin */
};

void configure_gpio_pin(gpio_config_t *config)
{
    assert(config);

    *(config->muxReg) = config->muxConfig;
    *(config->padReg) = config->padConfig;
}

/*******************************************************************************
 * EOF
 ******************************************************************************/
