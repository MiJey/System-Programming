#include "come_on.h"

Player player[2];
uint8_t buttons[2][4] = { {0, 0, 0, 0}, {0, 0, 0, 0} };

static int getWidth(Status status) {
	switch(status) {
	case READY: return 17;
	case WALK: return 17;
	case JUMP:
	case DJUMP: return 29;
	case DEFENSE: return 17;
	case PUNCH: return 23;
	case KICK: return 29;
	} return -1;
}

static int getHeight(Status status) {
	switch(status) {
	case READY: return 39;
	case WALK: return 39;
	case JUMP:
	case DJUMP: return 29;
	case DEFENSE: return 35;
	case PUNCH: return 35;
	case KICK: return 37;
	} return -1;
}

// 이전 위치 클리어
static void clear_previous(uint8_t p) {
	int xOffset = 0;
	int yOffset = 0;
	
	if(player[p].status == PUNCH && player[p].direction == LEFT) {
		xOffset = -6;
	}
		
	if(player[p].status == KICK && player[p].direction == LEFT) {
		xOffset = -12;
		yOffset = 2;
	}
	
	draw_clear_rectangle(
		player[p].x + xOffset,
		player[p].y + yOffset, 
		player[p].x + xOffset + getWidth(player[p].status),
		player[p].y + yOffset - getHeight(player[p].status));
}

static void p1_attack_to_p2(uint8_t p1, uint8_t p2) {
	// p1: 공격, p2: 데미지 입음
	// READY, WALK, JUMP, DJUMP, DEFENSE, PUNCH, KICK
	uint8_t damage[6][3] = {{2, 3, 1}, {3, 4, 2}, {1, 2, 1}, {2, 3, 0}, {1, 1, 2}, {1, 3, 3}};
	uint8_t i = 0, j = 0;
	
	switch(player[p1].status) {
	case PUNCH: j = 0; break;
	case KICK: j = 1; break;
	case JUMP:
	case DJUMP: j = 2; break;
	default: return;	// 공격 x
	}
	
	switch(player[p2].status) {
	case READY: i = 0; break;
	case WALK: i = 1; break;
	case DEFENSE: i = 2; break;
	case PUNCH: i = 3; break;
	case KICK: i = 4; break;
	case JUMP:
	case DJUMP: i = 5; break;
	}
	
	if(player[p2].hp - damage[i][j] < 0) {
		player[p2].hp = 0;
	} else {
		player[p2].hp -= damage[i][j];
	}
	
	draw_hp(p2 + 1, player[p2].hp);
}

// 충돌체크
static void collision_check() {
	uint8_t ax = player[0].x;
	uint8_t ay = player[0].y;
	uint8_t bx = player[0].x + getWidth(player[0].status);
	uint8_t by = player[0].y - getWidth(player[0].status);
	uint8_t cx = player[1].x;
	uint8_t cy = player[1].y;
	uint8_t dx = player[1].x + getWidth(player[1].status);
	uint8_t dy = player[1].y - getWidth(player[1].status);
	
	if((dx < ax || bx < cx) && (dy > ay || by > cy)) {
		bsp_board_led_invert(2);
		// 충돌 x
	} else {
		// 충돌 o
		bsp_board_led_invert(3);
		p1_attack_to_p2(0, 1);
		p1_attack_to_p2(1, 0);
		
		if(player[0].hp == 0 || player[1].hp == 0) {
			game_finish_round();
		}
	}
}

/**************************************************/

void game_next_step(uint8_t p) {
	// 이전 위치 클리어
	clear_previous(p);
	player[p].status = player[p].newStatus;
	
	// 새로운 위치를 설정하고 드로우
	switch(player[p].status) {
	case READY:
		stop_player_timer(p);	// 동작 그만
		
		// 범위를 벗어났는지 체크
		if(player[p].x < 0) { player[p].x = 0; }
		if(player[p].x > 108) { player[p].x = 108; }
		player[p].y = 39;
		player[p].tag = false;	// 태그 초기화
		
		draw_ready_p(player[p].x, player[p].y, player[p].direction == LEFT);
		break;
		
	case WALK:
		player[p].y = 39;
		
		if(player[p].direction == LEFT) {
			// 왼쪽으로 걷기, 미러모드 true
			if(player[p].x > 0) { player[p].x -= 6; }
			player[p].tag ? draw_walk_a(player[p].x, player[p].y, true) : draw_walk_b(player[p].x, player[p].y, true);
			player[p].tag = !player[p].tag;
		} else {
			// 오른쪽으로 걷기, 미러모드 false
			if(player[p].x < 105) { player[p].x += 6; }
			player[p].tag ? draw_walk_a(player[p].x, player[p].y, false) : draw_walk_b(player[p].x, player[p].y, false);
			player[p].tag = !player[p].tag;
		}
		break;
		

	case DJUMP:
		// 양 옆으로 이동
		if(player[p].direction == LEFT) { if(player[p].x > 0) { player[p].x -= 6; } }
		else { if(player[p].x < 105) { player[p].x += 6; } }
	case JUMP: {
		if(player[p].x > 105) { player[p].x = 105; }	// 오른쪽 벽에 붙어서 점프하면 밀려나도록
		if(player[p].direction == LEFT) {
			if(player[p].x < 6) { player[p].x = 0; }	// 왼쪽 벽에 붙어서 점프하면 밀려나도록
			else { player[p].x -= 6; }
		}
		
		// 점프 동작 수행
		uint8_t v = 35;	              // 속도
		uint8_t g = 5;	              // 중력 가속도
		uint8_t t = player[p].t;      // 시간
		uint8_t y0 = 30;              // 점프 끝나는 높이
		int y = y0 + v*t - g*t*t;
	
		if(y < y0) {
			player[p].t = 0;
			if(player[p].status == DJUMP) { player[p].newStatus = WALK; }
			else { player[p].newStatus = READY; }
		} else {
			player[p].y = y < 0 ? 0 : y;
			player[p].t += 1;
			draw_jump(player[p].x, player[p].y, player[p].direction == LEFT);
		}
		break; }
		
	case DEFENSE:		
		player[p].y = 35;
		
		draw_defense(player[p].x, player[p].y, player[p].direction == LEFT);
		break;
		
	case PUNCH:
		player[p].y = 35;
		
		if(player[p].x > 102) { player[p].x = 102; }	// 오른쪽 벽을 치면 밀려나도록
		if(player[p].direction == LEFT) {
			if(player[p].x < 6) { player[p].x = 0; }	// 왼쪽 벽을 치면 밀려나도록
			else { player[p].x -= 6; }
		}
		
		draw_punch(player[p].x, player[p].y, player[p].direction == LEFT);
		if(player[p].direction == LEFT) { player[p].x += 6; }
		
		if(player[p].t == 2) { 
			player[p].t = 0;
			player[p].newStatus = DEFENSE;
		}
		else {
			player[p].t += 1;
		}
		
		break;
		
	case KICK:
		player[p].y = 37;
		
		if(player[p].x > 96) { player[p].x = 96; }	// 오른쪽 벽을 차면 밀려나도록
		if(player[p].direction == LEFT) {
			if(player[p].x < 12) { player[p].x = 0; }	// 왼쪽 벽을 차면 밀려나도록
			else { player[p].x -= 12; }
		}
		
		draw_kick(player[p].x, player[p].y, player[p].direction == LEFT);
		if(player[p].direction == LEFT) { player[p].x += 12; }
		
		if(player[p].t == 3) { 
			player[p].t = 0;
			player[p].newStatus = DEFENSE;
		}
		else {
			player[p].t += 1;
		}
		break;
	}
	collision_check();
}

/**************************************************/

// 키 조합에 따라 상태 변경
// READY, WALK, JUMP, DJUMP, DEFENSE, PUNCH, KICK
void press_fn_button(uint8_t p) {
	buttons[p][0] = 1;
	
	if(player[p].status != JUMP && player[p].status != DJUMP) { // 점프 중이 아니고
		if(buttons[p][2] == buttons[p][3]) {                // READY 였으면
			player[p].newStatus = DEFENSE;              // 방어
		} else {                                            // WALK 였으면
			player[p].newStatus = PUNCH;                // 펀치!
		}
	}
}

void press_up_button(uint8_t p) {
	buttons[p][1] = 1;
	
	if(buttons[p][0] == 0) {
		if(buttons[p][2] == buttons[p][3]) {           // READY 였으면
			player[p].newStatus = JUMP;            // 점프
		} else {                                       // WALK 였으면
			player[p].newStatus = DJUMP;           // 대각선 점프
		}
	} else {                                               // DEFENSE 또는 PUNCH(x) 였으면
		player[p].newStatus = KICK;                    // 발차기
	}
}

void press_left_button(uint8_t p) {
	buttons[p][2] = 1;
	
	player[p].direction = LEFT;	// 방향은 왼쪽
	
	// 점프 중에는 좌우 방향키만 먹는다
	if(player[p].status == JUMP || player[p].status == DJUMP) {
		if(buttons[p][3] == 0) { player[p].newStatus = DJUMP; } // JUMP -> DJUMP
		else { player[p].newStatus = JUMP; }                    // DJUMP -> JUMP
		return;
	}
	
	if(buttons[p][0] == 0 && buttons[p][1] == 0) {
		if(buttons[p][3] == 0) { player[p].newStatus = WALK; }  // READY -> WALK
		else { player[p].newStatus = READY; }                   // WALK -> READY
	} else if(buttons[p][0] == 0 && buttons[p][1] == 1) {
		if(buttons[p][3] == 0) { player[p].newStatus = DJUMP; } // JUMP -> DJUMP
		else { player[p].newStatus = JUMP; }                    // DJUMP -> JUMP
	} else if(buttons[p][0] == 1 && buttons[p][1] == 0) {
		if(buttons[p][3] == 0) { player[p].newStatus = PUNCH;} // DEFENSE -> PUNCH
		//else { player[p].newStatus = DEFENSE; }                 // PUNCH -> DEFENSE
	} else {}                                                       // KICK -> KICK
}

void press_right_button(uint8_t p) {
	buttons[p][3] = 1;
	
	player[p].direction = RIGHT;	// 방향은 오른쪽
	
	// 점프 중에는 좌우 방향키만 먹는다
	if(player[p].status == JUMP || player[p].status == DJUMP) {
		if(buttons[p][2] == 0) { player[p].newStatus = DJUMP; } // JUMP -> DJUMP
		else { player[p].newStatus = JUMP; }                    // DJUMP -> JUMP
		return;
	}
	
	if(buttons[p][0] == 0 && buttons[p][1] == 0) {
		if(buttons[p][2] == 0) { player[p].newStatus = WALK; }  // READY -> WALK
		else { player[p].newStatus = READY; }                   // WALK -> READY
	} else if(buttons[p][0] == 0 && buttons[p][1] == 1) {
		if(buttons[p][2] == 0) { player[p].newStatus = DJUMP; } // JUMP -> DJUMP
		else { player[p].newStatus = JUMP; }                    // DJUMP -> JUMP
	} else if(buttons[p][0] == 1 && buttons[p][1] == 0) {
		if(buttons[p][2] == 0) { player[p].newStatus = PUNCH; } // DEFENSE -> PUNCH
		else { player[p].newStatus = DEFENSE; }                 // PUNCH -> DEFENSE
	} else {}                                                       // KICK -> KICK
}

void release_fn_button(uint8_t p) {
	buttons[p][0] = 0;

	if(player[p].status != JUMP && player[p].status != DJUMP) { // 점프 중이 아니고
		if(buttons[p][2] == buttons[p][3]) {                // DEFENSE 였으면
			player[p].newStatus = READY;                // 준비
		} else {                                            // PUNCH 였으면
			player[p].newStatus = WALK;                 // 걷기
		}
	}
}

void release_up_button(uint8_t p) {
	buttons[p][1] = 0;
	
	// 점프는 점프가 끝났을 때 READY나 WALK로 상태를 바꾼다
	if(buttons[p][0] == 0) {
		if(buttons[p][2] == buttons[p][3]) {           // JUMP 였으면
			//player[p].newStatus = READY;           // 준비
		} else {                                       // DJUMP 였으면
			//player[p].newStatus = WALK;            // 걷기
		}
	} else {
		if(buttons[p][2] == buttons[p][3]) {           // KICK 였으면
			player[p].newStatus = DEFENSE;         // 방어
		} else {                                       // KICK 였으면
			player[p].newStatus = PUNCH;            // 펀치
		}
	}
}

void release_left_button(uint8_t p) {
	buttons[p][2] = 0;
	
	// 점프 중에는 좌우 방향키를 떼도 계속 점프를 한다
	if(player[p].status == JUMP || player[p].status == DJUMP) {
		if(buttons[p][3] == 0) { player[p].newStatus = JUMP; }    // DJUMP -> JUMP
		else { player[p].newStatus = DJUMP; }                     // JUMP -> DJUMP
		return;
	}
	
	if(buttons[p][0] == 0 && buttons[p][1] == 0) {
		if(buttons[p][3] == 0) { player[p].newStatus = READY; }   // WALK -> READY
		else { player[p].newStatus = WALK; }                      // READY -> WALK
	} else if(buttons[p][0] == 0 && buttons[p][1] == 1) {
		if(buttons[p][3] == 0) { player[p].newStatus = JUMP; }    // DJUMP -> JUMP
		else { player[p].newStatus = DJUMP; }                     // JUMP -> DJUMP
	} else if(buttons[p][0] == 1 && buttons[p][1] == 0) {
		if(buttons[p][3] == 0) { player[p].newStatus = DEFENSE; } // PUNCH -> DEFENSE
		else { player[p].newStatus = PUNCH; }                     // DEFENSE -> PUNCH
	} else {}                                                         // KICK -> KICK
}

void release_right_button(uint8_t p) {
	buttons[p][3] = 0;

	// 점프 중에는 좌우 방향키를 떼도 계속 점프를 한다
	if(player[p].status == JUMP || player[p].status == DJUMP) {
		if(buttons[p][2] == 0) { player[p].newStatus = JUMP; }    // DJUMP -> JUMP
		else { player[p].newStatus = DJUMP; }                     // JUMP -> DJUMP
		return;
	}
	
	if(buttons[p][0] == 0 && buttons[p][1] == 0) {
		if(buttons[p][2] == 0) { player[p].newStatus = READY; }   // WALK -> READY
		else { player[p].newStatus = WALK; }                      // READY -> WALK
	} else if(buttons[p][0] == 0 && buttons[p][1] == 1) {
		if(buttons[p][2] == 0) { player[p].newStatus = JUMP; }    // DJUMP -> JUMP
		else { player[p].newStatus = DJUMP; }                     // JUMP -> DJUMP
	} else if(buttons[p][0] == 1 && buttons[p][1] == 0) {
		if(buttons[p][2] == 0) { player[p].newStatus = DEFENSE; } // PUNCH -> DEFENSE
		else { player[p].newStatus = PUNCH; }                     // DEFENSE -> PUNCH
	} else {}                                                         // KICK -> KICK
}

/**************************************************/

void game_ready() {
	player[0].x = 9;
	player[0].y = 39;
	player[0].t = 0;
	player[0].hp = 48;
	player[0].tag = true;
	player[0].status = READY;
	player[0].direction = RIGHT;
	
	player[1].x = 102;
	player[1].y = 39;
	player[1].t = 0;
	player[1].hp = 48;
	player[1].tag = true;
	player[1].status = READY;
	player[1].direction = LEFT;
	
	draw_clear();	// 화면 전체 클리어
	draw_time(60);
	
	draw_hp(1, 48);
	draw_hp(2, 48);
	
	draw_circle(0, player[0].wins > 0 ? 1 : 0);
	draw_circle(1, player[0].wins > 1 ? 1 : 0);
	draw_circle(2, player[1].wins > 1 ? 1 : 0);
	draw_circle(3, player[1].wins > 0 ? 1 : 0);
	
	draw_ready_p(player[0].x, player[0].y, false);	// mirror false
	draw_ready_p(player[1].x, player[1].y, true);	// mirror true
	
	draw_ready();
	nrf_delay_ms(3000);
	draw_go();

	start_game_timer();
}

void game_init() {
	player[0].wins = 0;
	player[1].wins = 0;
	
	game_ready();
}

void game_finish_round() {
	stop_game_timer();
	stop_player_timer(0);
	stop_player_timer(1);
	
	// 승리한 플레이어 표시
	if(player[0].hp == player[1].hp) {
		// 비김(둘 다 0일때 포함)
		draw_draw();
	} else {
		if(player[0].hp == 0 || player[1].hp == 0) {
			// ko 승
			draw_ko();
			nrf_delay_ms(2000);
		}
		
		if(player[0].hp > player[1].hp) {
			player[0].wins += 1;
			draw_p1_win();
		} else {
			player[1].wins += 1;
			draw_p2_win();
		}
	}
	
	nrf_delay_ms(3000);	
	
	// 3판 2선승제
	if(player[0].wins >= 2 || player[1].wins >= 2) {
		game_end();
	} else {
		game_ready();
	}
}

void game_end() {
	draw_circle(0, player[0].wins > 0 ? 1 : 0);
	draw_circle(1, player[0].wins > 1 ? 1 : 0);
	draw_circle(2, player[1].wins > 1 ? 1 : 0);
	draw_circle(3, player[1].wins > 0 ? 1 : 0);
	
	draw_end();
	while(1);
}
