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

typedef enum { READY, WALK, JUMP, DJUMP, DEFENSE, PUNCH, KICK } Status;
typedef enum { LEFT, RIGHT } Direction;
typedef struct {
	uint8_t x;
	uint8_t y;
	uint8_t t;
	uint8_t hp;
	bool tag;
	Status status;
	Status newStatus;
	Direction direction;
} Player;

extern Player player[2];

/**************************************************/

// spi
void lcd_init();
void st7586_write(const uint8_t category, const uint8_t data);

// draw
void draw_ready_p(uint8_t x, uint8_t y, bool mirror);
void draw_walk_a(uint8_t x, uint8_t y, bool mirror);
void draw_walk_b(uint8_t x, uint8_t y, bool mirror);
void draw_jump(uint8_t x, uint8_t y, bool mirror);
void draw_defense(uint8_t x, uint8_t y, bool mirror);
void draw_punch(uint8_t x, uint8_t y, bool mirror);
void draw_kick(uint8_t x, uint8_t y, bool mirror);

void draw_hp(uint8_t pl, uint8_t hp);
void draw_circle(uint8_t x1, uint8_t wl);
void draw_ready();
void draw_go();
void draw_splash();
void draw_clear();
void draw_clear_rectangle(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2);
void draw_time(uint8_t time);

// ble
void ble_start();
void start_game_timer();
void start_player_timer(uint8_t p, uint8_t tick);
void stop_player_timer(uint8_t p);

// game
void game_next_step(uint8_t num);
void game_ready();

