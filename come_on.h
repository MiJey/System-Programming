#include <stdbool.h>
#include <stdint.h>
#include <string.h>

// 공통
#include "nrf_delay.h"	// nrf_delay로 시작하는 함수들
#include "boards.h"	// bsp_board로 시작하는 함수들
#include "app_error.h"

// spi
#include "nrf_drv_spi.h"
#include "app_util_platform.h"
#include "nrf_gpio.h"
#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"

// ble
#include "nordic_common.h"
#include "nrf.h"
#include "ble.h"
#include "ble_err.h"
#include "ble_hci.h"
#include "ble_srv_common.h"
#include "ble_advdata.h"
#include "ble_conn_params.h"
#include "nrf_sdh.h"
#include "nrf_sdh_ble.h"
#include "app_timer.h"
#include "app_button.h"
#include "ble_lbs.h"
#include "nrf_ble_gatt.h"

#include "nrf_soc.h"
#include "nrf_nvic.h"

/**************************************************/

void blinky_come_on();

// spi
void lcd_init();
void draw_rectangle(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2);
