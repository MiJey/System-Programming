/**
 * Copyright (c) 2015 - 2017, Nordic Semiconductor ASA
 * 
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 * 
 * 1. Redistributions of source code must retain the above copyright notice, this
 *    list of conditions and the following disclaimer.
 * 
 * 2. Redistributions in binary form, except as embedded into a Nordic
 *    Semiconductor ASA integrated circuit in a product or a software update for
 *    such product, must reproduce the above copyright notice, this list of
 *    conditions and the following disclaimer in the documentation and/or other
 *    materials provided with the distribution.
 * 
 * 3. Neither the name of Nordic Semiconductor ASA nor the names of its
 *    contributors may be used to endorse or promote products derived from this
 *    software without specific prior written permission.
 * 
 * 4. This software, with or without modification, must only be used with a
 *    Nordic Semiconductor ASA integrated circuit.
 * 
 * 5. Any software provided in binary form under this license must not be reverse
 *    engineered, decompiled, modified and/or disassembled.
 * 
 * THIS SOFTWARE IS PROVIDED BY NORDIC SEMICONDUCTOR ASA "AS IS" AND ANY EXPRESS
 * OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY, NONINFRINGEMENT, AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL NORDIC SEMICONDUCTOR ASA OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
 * GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
 * OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * 
 */
#include "nrf_drv_spi.h"
#include "app_util_platform.h"
#include "nrf_gpio.h"
#include "nrf_delay.h"
#include "boards.h"
#include "app_error.h"
#include <string.h>
#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"

#define ST7586_SPI_INSTANCE  0/**< SPI instance index. */
static const nrf_drv_spi_t st7586_spi = NRF_DRV_SPI_INSTANCE(ST7586_SPI_INSTANCE);  /**< SPI instance. */
static volatile bool st7586_spi_xfer_done = false;  /**< Flag used to indicate that SPI instance completed the transfer. */

#define ST_COMMAND	0
#define ST_DATA		1

#define RATIO_SPI0_LCD_SCK	4
#define RATIO_SPI0_LCD_A0		28
#define RATIO_SPI0_LCD_MOSI	29
#define RATIO_SPI0_LCD_BSTB	30
#define RATIO_SPI0_LCD_CS		31

#define LCD_INIT_DELAY(t) nrf_delay_ms(t)

static unsigned char rx_data;

/**
 * @brief SPI user event handler.
 * @param event
 */

void spi_event_handler(nrf_drv_spi_evt_t const *p_event, void *p_context)
{
	st7586_spi_xfer_done = true;
}


void st7586_write(const uint8_t category, const uint8_t data)
{
	int err_code;
	nrf_gpio_pin_write(RATIO_SPI0_LCD_A0, category);

	st7586_spi_xfer_done = false;
	err_code = nrf_drv_spi_transfer(&st7586_spi, &data, 1, &rx_data, 0);
	APP_ERROR_CHECK(err_code);
	while (!st7586_spi_xfer_done) {
		__WFE();
	}
	nrf_delay_us(10);
}

static inline void st7586_pinout_setup()
{
	// spi setup
	int err_code;
	nrf_drv_spi_config_t spi_config = NRF_DRV_SPI_DEFAULT_CONFIG;
	spi_config.ss_pin   = RATIO_SPI0_LCD_CS;
	spi_config.miso_pin = NRF_DRV_SPI_PIN_NOT_USED;
	spi_config.mosi_pin = RATIO_SPI0_LCD_MOSI;
	spi_config.sck_pin  = RATIO_SPI0_LCD_SCK;
	spi_config.frequency = NRF_SPI_FREQ_1M;
	spi_config.mode = NRF_DRV_SPI_MODE_3;

	err_code = nrf_drv_spi_init(&st7586_spi, &spi_config, spi_event_handler, NULL);
	APP_ERROR_CHECK(err_code);

	nrf_gpio_pin_set(RATIO_SPI0_LCD_A0);
	nrf_gpio_cfg_output(RATIO_SPI0_LCD_A0);

	nrf_gpio_pin_clear(RATIO_SPI0_LCD_BSTB);
	nrf_gpio_cfg_output(RATIO_SPI0_LCD_BSTB);
}


void Initialization_ST7586S(){
	nrf_gpio_pin_write(RATIO_SPI0_LCD_BSTB, 0);
	LCD_INIT_DELAY(50);
	nrf_gpio_pin_write(RATIO_SPI0_LCD_BSTB, 1);

	LCD_INIT_DELAY(10);
	LCD_INIT_DELAY(120);
	st7586_write(ST_COMMAND, 0xD7);
	st7586_write(ST_DATA, 0x9F);
	st7586_write(ST_COMMAND, 0xE0);
	st7586_write(ST_DATA, 0x00);
	LCD_INIT_DELAY(10);
	st7586_write(ST_COMMAND, 0xE3);
	LCD_INIT_DELAY(20);
	st7586_write(ST_COMMAND, 0xE1);
	st7586_write(ST_COMMAND, 0x11);
	st7586_write(ST_COMMAND, 0x28);
	LCD_INIT_DELAY(50);

	// 부팅 수정한 부분
	st7586_write(ST_COMMAND,  0xC0);
	st7586_write(ST_DATA, 0x53);
	st7586_write(ST_DATA, 0x01);
	st7586_write(ST_COMMAND,  0xC3);
	st7586_write(ST_DATA, 0x02);
	st7586_write(ST_COMMAND,  0xC4);
	st7586_write(ST_DATA, 0x06);

	st7586_write(ST_COMMAND, 0xD0);
	st7586_write(ST_DATA, 0x1D);
	st7586_write(ST_COMMAND, 0xB5);
	st7586_write(ST_DATA, 0x00);
	st7586_write(ST_COMMAND, 0x39);
	st7586_write(ST_COMMAND, 0x3A);
	st7586_write(ST_DATA, 0x02);
	st7586_write(ST_COMMAND, 0x36);
	st7586_write(ST_DATA, 0x00);
	st7586_write(ST_COMMAND, 0xB0);
	st7586_write(ST_DATA, 0x9F);
	st7586_write(ST_COMMAND, 0xB4);
	st7586_write(ST_DATA, 0xA0);
	st7586_write(ST_COMMAND, 0x30);
	st7586_write(ST_DATA, 0x00);
	st7586_write(ST_DATA, 0x00);
	st7586_write(ST_DATA, 0x00);
	st7586_write(ST_DATA, 0x77);
	st7586_write(ST_COMMAND, 0x20);
	st7586_write(ST_COMMAND, 0x2A);
	st7586_write(ST_DATA, 0x00);
	st7586_write(ST_DATA, 0x00);
	st7586_write(ST_DATA, 0x00);
	st7586_write(ST_DATA, 0x7F);
	st7586_write(ST_COMMAND, 0x2B);
	st7586_write(ST_DATA, 0x00);
	st7586_write(ST_DATA, 0x00);
	st7586_write(ST_DATA, 0x00);
	st7586_write(ST_DATA, 0x9F);

	st7586_write(ST_COMMAND, 0x29);	// display on
}

int main(void)
{
	bsp_board_leds_init();

	APP_ERROR_CHECK(NRF_LOG_INIT(NULL));
	NRF_LOG_DEFAULT_BACKENDS_INIT();
    
	st7586_pinout_setup();
	Initialization_ST7586S();
	//NRF_LOG_INFO("SPI example.");

    // st7586_write(ST_COMMAND, data) -> When A0 is a command, it calls a function that performs SPI communication
    // st7586_write(ST_DATA, data) -> When A0 is a Data, it calls a function that performs SPI communication

    /* Toggle LEDs. */
    while (true)
    {
        for (int i = 0; i < LEDS_NUMBER; i++)
        {
            bsp_board_led_invert(0);
            nrf_delay_ms(500);
        }
    }

/*
    while (1)
    {
        // Reset rx buffer and transfer done flag


        while (1) {
        	__WFE();
        }

        NRF_LOG_FLUSH();

        bsp_board_led_invert(BSP_BOARD_LED_0);
        nrf_delay_ms(200);
    }
*/
}
