#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#include "nrf_delay.h"	// nrf_delay로 시작하는 함수들
#include "boards.h"	// bsp_board로 시작하는 함수들

/* spi */
#include "nrf_drv_spi.h"
#include "app_util_platform.h"
#include "nrf_gpio.h"
#include "app_error.h"
#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"

/**************************************************/

void blinky_come_on();

/* spi */
void st7586_init();
void draw_rectangle(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2);
