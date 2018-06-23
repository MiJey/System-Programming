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

// scheduler
#include "nrf_drv_gpiote.h"
#include "nrf_drv_clock.h"
#include "app_scheduler.h"

/******************** 구조체 ********************/

typedef enum {
	READY = 1840,    // 0(준비)   18x40px
	WALK = 11840,    // 1(걷기)   18x40px
	DEFENSE = 21836, // 2(방어)   18x36px
	PUNCH = 32140,   // 3(주먹)   21x40px
	KICK = 43040,    // 4(발차기) 30x40px
	JUMP = 53030     // 5(점프)   30x30px
} Status;
// typedef enum { READY, WALK, DEFENSE, PUNCH, KICK, JUMP } Status;
typedef enum { LEFT, NEUTRAL, RIGHT } Direction;
typedef struct {
	uint8_t x;
	uint8_t y;
	uint8_t hp;
	bool tag;
	Status status;
	Direction direction;
} Player;

extern Player player[2];

/**************************************************/

// spi
void lcd_init();
void st7586_write(const uint8_t category, const uint8_t data);

// draw
void draw_rectangle(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2);
void draw_ready_p(uint8_t x, uint8_t y);
void draw_walk_a(uint8_t x, uint8_t y);
void draw_walk_b(uint8_t x, uint8_t y);
void draw_jump(uint8_t x, uint8_t y);
void draw_hp(uint8_t pl, uint8_t hp);
void draw_circle(uint8_t x1, uint8_t wl);
void draw_ready();
void draw_go();
void draw_splash();
void draw_clear();
void draw_clear_previous(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2);
void draw_time(uint8_t time);

// ble
void ble_start();

// game
void move_player(uint8_t num);
void stop_player(uint8_t num);
void game_ready();
