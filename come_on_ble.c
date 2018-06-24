#include "come_on.h"

#define P2_FN_BTNLED                    BSP_BOARD_LED_0
#define P2_UP_BTNLED                    BSP_BOARD_LED_1
#define P2_LE_BTNLED                    BSP_BOARD_LED_2
#define P2_RI_BTNLED                    BSP_BOARD_LED_3

#define P1_FN_BUTTON                    BSP_BUTTON_0
#define P1_UP_BUTTON                    BSP_BUTTON_1
#define P1_LE_BUTTON                    BSP_BUTTON_2
#define P1_RI_BUTTON                    BSP_BUTTON_3

/******************** BLE ********************/

#define DEVICE_NAME                     "Come on!"

#define APP_FEATURE_NOT_SUPPORTED       BLE_GATT_STATUS_ATTERR_APP_BEGIN + 2	// Reply when unsupported features are requested.
#define APP_BLE_OBSERVER_PRIO           3
#define APP_BLE_CONN_CFG_TAG            1
#define APP_ADV_INTERVAL                64	// The advertising interval (in units of 0.625 ms; this value corresponds to 40 ms)
#define APP_ADV_TIMEOUT_IN_SECONDS      BLE_GAP_ADV_TIMEOUT_GENERAL_UNLIMITED	// The advertising time-out (in units of seconds). When set to 0, we will never time out.
#define MIN_CONN_INTERVAL               MSEC_TO_UNITS(100, UNIT_1_25_MS)	// Minimum acceptable connection interval (0.5 seconds).
#define MAX_CONN_INTERVAL               MSEC_TO_UNITS(200, UNIT_1_25_MS)	// Maximum acceptable connection interval (1 second).
#define SLAVE_LATENCY                   0
#define CONN_SUP_TIMEOUT                MSEC_TO_UNITS(4000, UNIT_10_MS)	// Connection supervisory time-out (4 seconds).
#define FIRST_CONN_PARAMS_UPDATE_DELAY  APP_TIMER_TICKS(20000)	// Time from initiating event (connect or start of notification) to first time sd_ble_gap_conn_param_update is called (15 seconds).
#define NEXT_CONN_PARAMS_UPDATE_DELAY   APP_TIMER_TICKS(5000)	// Time between each call to sd_ble_gap_conn_param_update after the first call (5 seconds).
#define MAX_CONN_PARAMS_UPDATE_COUNT    3	// Number of attempts before giving up the connection parameter negotiation.
#define BUTTON_DETECTION_DELAY          APP_TIMER_TICKS(50)
#define DEAD_BEEF                       0xDEADBEEF	// Value used as error code on stack dump, can be used to identify stack location on stack unwind.

// Scheduler settings
#define SCHED_MAX_EVENT_DATA_SIZE       sizeof(nrf_drv_gpiote_pin_t)
#define SCHED_QUEUE_SIZE                10

BLE_LBS_DEF(m_lbs);             // LED Button Service instance.
NRF_BLE_GATT_DEF(m_gatt);       // GATT module instance.

static uint16_t m_conn_handle = BLE_CONN_HANDLE_INVALID;	// Handle of the current connection.

static uint8_t p1_buttons[] = { 0, 0, 0, 0 };
//static uint8_t p2_buttons[] = { 0, 0, 0, 0 };

/******************** 앱 타이머 ********************/

APP_TIMER_DEF(m_game_timer_id); // 게임 시간 타이머
APP_TIMER_DEF(m_p1_timer_id); // P1 이동 타이머
APP_TIMER_DEF(m_p2_timer_id); // P2 이동 타이머

uint8_t game_time = 59;
uint8_t p1_time = 0;
uint8_t p2_time = 0;

// 1초마다 줄어드는 게임 타이머
static void game_timer_scheduler(void *p_event_data, uint16_t event_size) {
	draw_time(game_time);
	if(game_time == 57) { draw_clear_rectangle(27, 119, 99, 100); }
	if(game_time == 0) { 
		// TODO 게임 종료
		game_time = 10;
	} else { game_time -= 1; }
	
	bsp_board_led_invert(0);
}

// 플레이어가 움직이는 타이머
static void p1_timer_scheduler(void *p_event_data, uint16_t event_size) { game_next_step(0); }
static void p2_timer_scheduler(void *p_event_data, uint16_t event_size) { game_next_step(1); }

// timer handler --> scheduler
static void game_timer_handler(void *p_context) { app_sched_event_put(&game_time, sizeof(game_time), game_timer_scheduler); }
static void p1_timer_handler(void *p_context) { app_sched_event_put(&p1_time, sizeof(p1_time), p1_timer_scheduler); }
static void p2_timer_handler(void *p_context) { app_sched_event_put(&p2_time, sizeof(p2_time), p2_timer_scheduler); }

// start timer
void start_game_timer() { app_timer_start(m_game_timer_id, APP_TIMER_TICKS(1000), NULL); }
void start_player_timer(uint8_t p, uint8_t tick) {
	app_timer_start(p == 0 ? m_p1_timer_id : m_p2_timer_id, APP_TIMER_TICKS(tick), NULL);
}
void start_p1_timer() { app_timer_start(m_p1_timer_id, APP_TIMER_TICKS(100), NULL); }	// 0.2초마다 이동
static void start_p2_timer() { app_timer_start(m_p2_timer_id, APP_TIMER_TICKS(100), NULL); }

// stop timer
void stop_player_timer(uint8_t p) {
	app_timer_stop(p == 0 ? m_p1_timer_id : m_p2_timer_id);
}

static void create_timers() {
	app_timer_create(&m_game_timer_id, APP_TIMER_MODE_REPEATED, game_timer_handler); // 게임 시간(60초) 타이머
	app_timer_create(&m_p1_timer_id, APP_TIMER_MODE_REPEATED, p1_timer_handler);     // 플레이어 1 이동 타이머
	app_timer_create(&m_p2_timer_id, APP_TIMER_MODE_REPEATED, p2_timer_handler);     // 플레이어 2 이동 타이머
}

/******************** 스케줄러 ********************/

static void button_pressed_scheduler_event_handler(void *p_event_data, uint16_t event_size) {
	uint8_t pin = *((uint8_t*)p_event_data);
	switch(pin) {
	// 키 조합에 따라 상태 변경
	// READY, WALK, JUMP, DJUMP, DEFENSE, PUNCH, KICK
	case P1_FN_BUTTON:
		ble_lbs_on_button_change(m_conn_handle, &m_lbs, 27);	// 26, 27
		p1_buttons[0] = 1;
		
		if(player[0].status != JUMP && player[0].status != DJUMP) { // 점프 중이 아니고
			if(p1_buttons[2] == p1_buttons[3]) {                // READY 였으면
				player[0].newStatus = DEFENSE;              // 방어
			} else {                                            // WALK 였으면
				player[0].newStatus = PUNCH;                // 펀치!
			}
		}
		break;
		
	case P1_UP_BUTTON:
		ble_lbs_on_button_change(m_conn_handle, &m_lbs, 29);	// 28, 29
		p1_buttons[1] = 1;
		
		if(p1_buttons[0] == 0) {
			if(p1_buttons[2] == p1_buttons[3]) {           // READY 였으면
				player[0].newStatus = JUMP;            // 점프
			} else {                                       // WALK 였으면
				player[0].newStatus = DJUMP;           // 대각선 점프
			}
		} else {                                               // DEFENSE 또는 PUNCH(x) 였으면
			player[0].newStatus = KICK;                    // 발차기
		}
		break;
		
	case P1_LE_BUTTON:
		ble_lbs_on_button_change(m_conn_handle, &m_lbs, 31);	// 30, 31
		p1_buttons[2] = 1;
		
		player[0].direction = LEFT;	// 방향은 왼쪽
		
		// 점프 중에는 좌우 방향키만 먹는다
		if(player[0].status == JUMP || player[0].status == DJUMP) {
			if(p1_buttons[3] == 0) { player[0].newStatus = DJUMP; } // JUMP -> DJUMP
			else { player[0].newStatus = JUMP; }                    // DJUMP -> JUMP
			break;
		}
		
		if(p1_buttons[0] == 0 && p1_buttons[1] == 0) {
			if(p1_buttons[3] == 0) { player[0].newStatus = WALK; }  // READY -> WALK
			else { player[0].newStatus = READY; }                   // WALK -> READY
		} else if(p1_buttons[0] == 0 && p1_buttons[1] == 1) {
			if(p1_buttons[3] == 0) { player[0].newStatus = DJUMP; } // JUMP -> DJUMP
			else { player[0].newStatus = JUMP; }                    // DJUMP -> JUMP
		} else if(p1_buttons[0] == 1 && p1_buttons[1] == 0) {
			if(p1_buttons[3] == 0) { player[0].newStatus = PUNCH;} // DEFENSE -> PUNCH
			//else { player[0].newStatus = DEFENSE; }                 // PUNCH -> DEFENSE
		} else {}                                                       // KICK -> KICK
		break;
		
	case P1_RI_BUTTON:
		ble_lbs_on_button_change(m_conn_handle, &m_lbs, 33);	// 32, 33
		p1_buttons[3] = 1;
		
		player[0].direction = RIGHT;	// 방향은 오른쪽
		
		// 점프 중에는 좌우 방향키만 먹는다
		if(player[0].status == JUMP || player[0].status == DJUMP) {
			if(p1_buttons[2] == 0) { player[0].newStatus = DJUMP; } // JUMP -> DJUMP
			else { player[0].newStatus = JUMP; }                    // DJUMP -> JUMP
			break;
		}
		
		if(p1_buttons[0] == 0 && p1_buttons[1] == 0) {
			if(p1_buttons[2] == 0) { player[0].newStatus = WALK; }  // READY -> WALK
			else { player[0].newStatus = READY; }                   // WALK -> READY
		} else if(p1_buttons[0] == 0 && p1_buttons[1] == 1) {
			if(p1_buttons[2] == 0) { player[0].newStatus = DJUMP; } // JUMP -> DJUMP
			else { player[0].newStatus = JUMP; }                    // DJUMP -> JUMP
		} else if(p1_buttons[0] == 1 && p1_buttons[1] == 0) {
			if(p1_buttons[2] == 0) { player[0].newStatus = PUNCH; } // DEFENSE -> PUNCH
			else { player[0].newStatus = DEFENSE; }                 // PUNCH -> DEFENSE
		} else {}                                                       // KICK -> KICK
		break;
	}
	start_p1_timer();
}

static void button_released_scheduler_event_handler(void *p_event_data, uint16_t event_size) {
	uint8_t pin = *((uint8_t*)p_event_data);
	switch(pin) {
	// 키 조합에 따라 상태 변경
	// READY, WALK, JUMP, DJUMP, DEFENSE, PUNCH, KICK
	case P1_FN_BUTTON:
		ble_lbs_on_button_change(m_conn_handle, &m_lbs, 26);	// 26, 27
		p1_buttons[0] = 0;
		
		if(player[0].status != JUMP && player[0].status != DJUMP) { // 점프 중이 아니고
			if(p1_buttons[2] == p1_buttons[3]) {                // DEFENSE 였으면
				player[0].newStatus = READY;                // 준비
			} else {                                            // PUNCH 였으면
				player[0].newStatus = WALK;                 // 걷기
			}
		}
		break;
	
	case P1_UP_BUTTON:
		ble_lbs_on_button_change(m_conn_handle, &m_lbs, 28);	// 28, 29
		p1_buttons[1] = 0;
		
		// 점프는 점프가 끝났을 때 READY나 WALK로 상태를 바꾼다
		if(p1_buttons[0] == 0) {
			if(p1_buttons[2] == p1_buttons[3]) {           // JUMP 였으면
				//player[0].newStatus = READY;           // 준비
			} else {                                       // DJUMP 였으면
				//player[0].newStatus = WALK;            // 걷기
			}
		} else {
			if(p1_buttons[2] == p1_buttons[3]) {           // KICK 였으면
				player[0].newStatus = DEFENSE;         // 방어
			} else {                                       // KICK 였으면
				player[0].newStatus = PUNCH;            // 펀치
			}
		}
		break;
		
	case P1_LE_BUTTON:
		ble_lbs_on_button_change(m_conn_handle, &m_lbs, 30);	// 30, 31
		p1_buttons[2] = 0;
		
		// 점프 중에는 좌우 방향키를 떼도 계속 점프를 한다
		if(player[0].status == JUMP || player[0].status == DJUMP) {
			if(p1_buttons[3] == 0) { player[0].newStatus = JUMP; }    // DJUMP -> JUMP
			else { player[0].newStatus = DJUMP; }                     // JUMP -> DJUMP
			break;
		}
		
		if(p1_buttons[0] == 0 && p1_buttons[1] == 0) {
			if(p1_buttons[3] == 0) { player[0].newStatus = READY; }   // WALK -> READY
			else { player[0].newStatus = WALK; }                      // READY -> WALK
		} else if(p1_buttons[0] == 0 && p1_buttons[1] == 1) {
			if(p1_buttons[3] == 0) { player[0].newStatus = JUMP; }    // DJUMP -> JUMP
			else { player[0].newStatus = DJUMP; }                     // JUMP -> DJUMP
		} else if(p1_buttons[0] == 1 && p1_buttons[1] == 0) {
			if(p1_buttons[3] == 0) { player[0].newStatus = DEFENSE; } // PUNCH -> DEFENSE
			else { player[0].newStatus = PUNCH; }                     // DEFENSE -> PUNCH
		} else {}                                                         // KICK -> KICK
		break;
		
	case P1_RI_BUTTON:
		ble_lbs_on_button_change(m_conn_handle, &m_lbs, 32);	// 32, 33
		p1_buttons[3] = 0;

		// 점프 중에는 좌우 방향키를 떼도 계속 점프를 한다
		if(player[0].status == JUMP || player[0].status == DJUMP) {
			if(p1_buttons[2] == 0) { player[0].newStatus = JUMP; }    // DJUMP -> JUMP
			else { player[0].newStatus = DJUMP; }                     // JUMP -> DJUMP
			break;
		}
		
		if(p1_buttons[0] == 0 && p1_buttons[1] == 0) {
			if(p1_buttons[2] == 0) { player[0].newStatus = READY; }   // WALK -> READY
			else { player[0].newStatus = WALK; }                      // READY -> WALK
		} else if(p1_buttons[0] == 0 && p1_buttons[1] == 1) {
			if(p1_buttons[2] == 0) { player[0].newStatus = JUMP; }    // DJUMP -> JUMP
			else { player[0].newStatus = DJUMP; }                     // JUMP -> DJUMP
		} else if(p1_buttons[0] == 1 && p1_buttons[1] == 0) {
			if(p1_buttons[2] == 0) { player[0].newStatus = DEFENSE; } // PUNCH -> DEFENSE
			else { player[0].newStatus = PUNCH; }                     // DEFENSE -> PUNCH
		} else {}                                                         // KICK -> KICK
		break;
	}
}

static void button_event_handler(uint8_t pin, uint8_t button_action) {
	if(button_action) { app_sched_event_put(&pin, sizeof(pin), button_pressed_scheduler_event_handler); }
	else { app_sched_event_put(&pin, sizeof(pin), button_released_scheduler_event_handler); }
}

static void buttons_init(void) {
	//The array must be static because a pointer to it will be saved in the button handler module.
	static app_button_cfg_t buttons[] = {
		{P1_FN_BUTTON, false, BUTTON_PULL, button_event_handler},
		{P1_UP_BUTTON, false, BUTTON_PULL, button_event_handler},
		{P1_LE_BUTTON, false, BUTTON_PULL, button_event_handler},
		{P1_RI_BUTTON, false, BUTTON_PULL, button_event_handler}};
	ret_code_t err_code = app_button_init(buttons, sizeof(buttons) / sizeof(buttons[0]), BUTTON_DETECTION_DELAY);
	APP_ERROR_CHECK(err_code);
}

/**************************************************/

void log_init(void) {
	ret_code_t err_code = NRF_LOG_INIT(NULL);
	APP_ERROR_CHECK(err_code);
	NRF_LOG_DEFAULT_BACKENDS_INIT();
}

static void advertising_start(void) {
	ret_code_t           err_code;
	ble_gap_adv_params_t adv_params;

	// Start advertising
	memset(&adv_params, 0, sizeof(adv_params));

	adv_params.type        = BLE_GAP_ADV_TYPE_ADV_IND;
	adv_params.p_peer_addr = NULL;
	adv_params.fp          = BLE_GAP_ADV_FP_ANY;
	adv_params.interval    = APP_ADV_INTERVAL;
	adv_params.timeout     = APP_ADV_TIMEOUT_IN_SECONDS;

	err_code = sd_ble_gap_adv_start(&adv_params, APP_BLE_CONN_CFG_TAG);
	APP_ERROR_CHECK(err_code);
//	bsp_board_led_on(ADVERTISING_LED);
}

static void game_ready_scheduler(void *p_event_data, uint16_t event_size) {
	game_ready();
}

void ble_evt_handler(ble_evt_t const *p_ble_evt, void *p_context) {
	ret_code_t err_code;

	switch (p_ble_evt->header.evt_id) {
	case BLE_GAP_EVT_CONNECTED:
		NRF_LOG_INFO("Connected");
//		bsp_board_led_on(CONNECTED_LED);
//		bsp_board_led_off(ADVERTISING_LED);
		m_conn_handle = p_ble_evt->evt.gap_evt.conn_handle;

		err_code = app_button_enable();
		APP_ERROR_CHECK(err_code);
		app_sched_event_put(NULL, 0, game_ready_scheduler);	// 블루투스 연결되면 게임 레디
		break;

	case BLE_GAP_EVT_DISCONNECTED:
		NRF_LOG_INFO("Disconnected");
//		bsp_board_led_off(CONNECTED_LED);
		m_conn_handle = BLE_CONN_HANDLE_INVALID;
		err_code = app_button_disable();
		APP_ERROR_CHECK(err_code);
		advertising_start();
		break;

	case BLE_GAP_EVT_SEC_PARAMS_REQUEST:
		// Pairing not supported
		err_code = sd_ble_gap_sec_params_reply(m_conn_handle, BLE_GAP_SEC_STATUS_PAIRING_NOT_SUPP, NULL, NULL);
		APP_ERROR_CHECK(err_code);
		break;

#ifndef S140
	case BLE_GAP_EVT_PHY_UPDATE_REQUEST: {
		NRF_LOG_DEBUG("PHY update request.");
		ble_gap_phys_t const phys = { .rx_phys = BLE_GAP_PHY_AUTO, .tx_phys = BLE_GAP_PHY_AUTO, };
		err_code = sd_ble_gap_phy_update(p_ble_evt->evt.gap_evt.conn_handle, &phys);
		APP_ERROR_CHECK(err_code);
	}	break;
#endif

	case BLE_GATTS_EVT_SYS_ATTR_MISSING:
		// No system attributes have been stored.
		err_code = sd_ble_gatts_sys_attr_set(m_conn_handle, NULL, 0, 0);
		APP_ERROR_CHECK(err_code);
		break;

	case BLE_GATTC_EVT_TIMEOUT:
		// Disconnect on GATT Client timeout event.
		NRF_LOG_DEBUG("GATT Client Timeout.");
		err_code = sd_ble_gap_disconnect(p_ble_evt->evt.gattc_evt.conn_handle, BLE_HCI_REMOTE_USER_TERMINATED_CONNECTION);
		APP_ERROR_CHECK(err_code);
		break;

	case BLE_GATTS_EVT_TIMEOUT:
		// Disconnect on GATT Server timeout event.
		NRF_LOG_DEBUG("GATT Server Timeout.");
		err_code = sd_ble_gap_disconnect(p_ble_evt->evt.gatts_evt.conn_handle, BLE_HCI_REMOTE_USER_TERMINATED_CONNECTION);
		APP_ERROR_CHECK(err_code);
		break;

	case BLE_EVT_USER_MEM_REQUEST:
		err_code = sd_ble_user_mem_reply(p_ble_evt->evt.gattc_evt.conn_handle, NULL);
		APP_ERROR_CHECK(err_code);
		break;

	case BLE_GATTS_EVT_RW_AUTHORIZE_REQUEST: {
		ble_gatts_evt_rw_authorize_request_t  req;
		ble_gatts_rw_authorize_reply_params_t auth_reply;

		req = p_ble_evt->evt.gatts_evt.params.authorize_request;

		if (req.type != BLE_GATTS_AUTHORIZE_TYPE_INVALID) {
			if ((req.request.write.op == BLE_GATTS_OP_PREP_WRITE_REQ)
			|| (req.request.write.op == BLE_GATTS_OP_EXEC_WRITE_REQ_NOW)
			|| (req.request.write.op == BLE_GATTS_OP_EXEC_WRITE_REQ_CANCEL)) {
				if (req.type == BLE_GATTS_AUTHORIZE_TYPE_WRITE) {
					auth_reply.type = BLE_GATTS_AUTHORIZE_TYPE_WRITE;
				} else {
					auth_reply.type = BLE_GATTS_AUTHORIZE_TYPE_READ;
				}
				auth_reply.params.write.gatt_status = APP_FEATURE_NOT_SUPPORTED;
				err_code = sd_ble_gatts_rw_authorize_reply(p_ble_evt->evt.gatts_evt.conn_handle, &auth_reply);
				APP_ERROR_CHECK(err_code);
			}
		}
	} break;

	default: break;
	}
}

static void ble_stack_init(void) {
	ret_code_t err_code = nrf_sdh_enable_request();
	APP_ERROR_CHECK(err_code);

	// Configure the BLE stack using the default settings.
	// Fetch the start address of the application RAM.
	uint32_t ram_start = 0;
	err_code = nrf_sdh_ble_default_cfg_set(APP_BLE_CONN_CFG_TAG, &ram_start);
	APP_ERROR_CHECK(err_code);

	// Enable BLE stack.
	err_code = nrf_sdh_ble_enable(&ram_start);
	APP_ERROR_CHECK(err_code);

	// Register a handler for BLE events.
	NRF_SDH_BLE_OBSERVER(m_ble_observer, APP_BLE_OBSERVER_PRIO, ble_evt_handler, NULL);
}

static void gap_params_init(void) {
	ret_code_t              err_code;
	ble_gap_conn_params_t   gap_conn_params;
	ble_gap_conn_sec_mode_t sec_mode;

	BLE_GAP_CONN_SEC_MODE_SET_OPEN(&sec_mode);
	err_code = sd_ble_gap_device_name_set(&sec_mode, (const uint8_t *)DEVICE_NAME, strlen(DEVICE_NAME));
	APP_ERROR_CHECK(err_code);

	memset(&gap_conn_params, 0, sizeof(gap_conn_params));
	gap_conn_params.min_conn_interval = MIN_CONN_INTERVAL;
	gap_conn_params.max_conn_interval = MAX_CONN_INTERVAL;
	gap_conn_params.slave_latency     = SLAVE_LATENCY;
	gap_conn_params.conn_sup_timeout  = CONN_SUP_TIMEOUT;
	err_code = sd_ble_gap_ppcp_set(&gap_conn_params);
	APP_ERROR_CHECK(err_code);
}

static void gatt_init(void) {
	ret_code_t err_code = nrf_ble_gatt_init(&m_gatt, NULL);
	APP_ERROR_CHECK(err_code);
}

static void p2_btn_scheduler(void *p_event_data, uint16_t event_size) {
	uint8_t p2_state = *((uint8_t*)p_event_data);
	
	switch (p2_state) {
	case 0: bsp_board_led_off(P2_FN_BTNLED); break;
	case 1: bsp_board_led_on(P2_FN_BTNLED); break;
	case 2: bsp_board_led_off(P2_UP_BTNLED); break;
	case 3: bsp_board_led_on(P2_UP_BTNLED); break;
	
	case 4: // p2 left 버튼 뗐을 때
		if(player[1].status != JUMP) { player[1].newStatus = READY; }
		stop_player_timer(1);
		break;
		
	case 5: // p2 left 버튼 눌렀을 때
		player[1].newStatus = WALK;
		player[1].direction = LEFT;
		start_p2_timer();
		break;
		
	case 6: // p2 right 버튼 뗐을 때
		if(player[1].status != JUMP) { player[1].newStatus = READY; }
		stop_player_timer(1);
		break;
		
	case 7: // p2 right 버튼 눌렀을 때
		player[1].newStatus = WALK;
		player[1].direction = RIGHT;
		start_p2_timer();
		break;
	default: break;
	}
}

static void led_write_handler(uint16_t conn_handle, ble_lbs_t * p_lbs, uint8_t led_state) {
	switch (led_state) {
	case 0: bsp_board_led_off(P2_FN_BTNLED); break;
	case 1: bsp_board_led_on(P2_FN_BTNLED); break;
	case 2: bsp_board_led_off(P2_UP_BTNLED); break;
	case 3: bsp_board_led_on(P2_UP_BTNLED); break;
	case 4: bsp_board_led_off(P2_LE_BTNLED); break;
	case 5: bsp_board_led_on(P2_LE_BTNLED); break;
	case 6: bsp_board_led_off(P2_RI_BTNLED); break;
	case 7: bsp_board_led_on(P2_RI_BTNLED); break; }
	
	// p2 움직이는 것도 스케줄러에 넣음
	app_sched_event_put(&led_state, sizeof(led_state), p2_btn_scheduler);
}

static void services_init(void) {
	ret_code_t     err_code;
	ble_lbs_init_t init;

	init.led_write_handler = led_write_handler;

	err_code = ble_lbs_init(&m_lbs, &init);
	APP_ERROR_CHECK(err_code);
}

static void advertising_init(void) {
	ret_code_t    err_code;
	ble_advdata_t advdata;
	ble_advdata_t srdata;
	ble_uuid_t    adv_uuids[] = {{LBS_UUID_SERVICE, m_lbs.uuid_type}};

	// Build and set advertising data
	memset(&advdata, 0, sizeof(advdata));

	advdata.name_type          = BLE_ADVDATA_FULL_NAME;
	advdata.include_appearance = true;
	advdata.flags              = BLE_GAP_ADV_FLAGS_LE_ONLY_GENERAL_DISC_MODE;

	memset(&srdata, 0, sizeof(srdata));
	srdata.uuids_complete.uuid_cnt = sizeof(adv_uuids) / sizeof(adv_uuids[0]);
	srdata.uuids_complete.p_uuids  = adv_uuids;

	err_code = ble_advdata_set(&advdata, &srdata);
	APP_ERROR_CHECK(err_code);
}

// conn_params
static void on_conn_params_evt(ble_conn_params_evt_t * p_evt) {
	ret_code_t err_code;

	if (p_evt->evt_type == BLE_CONN_PARAMS_EVT_FAILED) {
		err_code = sd_ble_gap_disconnect(m_conn_handle, BLE_HCI_CONN_INTERVAL_UNACCEPTABLE);
		APP_ERROR_CHECK(err_code);
	}
}
static void conn_params_error_handler(uint32_t nrf_error) { APP_ERROR_HANDLER(nrf_error); }
static void conn_params_init(void) {
	ret_code_t             err_code;
	ble_conn_params_init_t cp_init;

	memset(&cp_init, 0, sizeof(cp_init));

	cp_init.p_conn_params                  = NULL;
	cp_init.first_conn_params_update_delay = FIRST_CONN_PARAMS_UPDATE_DELAY;
	cp_init.next_conn_params_update_delay  = NEXT_CONN_PARAMS_UPDATE_DELAY;
	cp_init.max_conn_params_update_count   = MAX_CONN_PARAMS_UPDATE_COUNT;
	cp_init.start_on_notify_cccd_handle    = BLE_GATT_HANDLE_INVALID;
	cp_init.disconnect_on_fail             = false;
	cp_init.evt_handler                    = on_conn_params_evt;
	cp_init.error_handler                  = conn_params_error_handler;

	err_code = ble_conn_params_init(&cp_init);
	APP_ERROR_CHECK(err_code);
}

/**************************************************/

void ble_start() {
	bsp_board_leds_init();	// leds_init
	log_init();
	
	// scheduler
	app_timer_init();
	create_timers();	// 앱 타이머
	buttons_init();
	APP_SCHED_INIT(SCHED_MAX_EVENT_DATA_SIZE, SCHED_QUEUE_SIZE);
	
	// ble
	ble_stack_init();
	gap_params_init();
	gatt_init();
	services_init();
	advertising_init();
	conn_params_init();

	advertising_start();
}
