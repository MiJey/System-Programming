#include "come_on.h"

Player player[2];
uint8_t buttons[2][4] = { {0, 0, 0, 0}, {0, 0, 0, 0} };

static void clear_previous(uint8_t p) {
	int xOffset = 0;
	int yOffset = 0;
	int width = 0;
	int height = 0;
	
	switch(player[p].status) {
	case READY: width = 17; height = 39; break;
	case WALK: width = 17; height = 39; break;
	case JUMP:
	case DJUMP: width = 29; height = 29; break;
	case DEFENSE: width = 17; height = 35; break;
	case PUNCH: width = 23; height = 35; 
		if(player[p].direction == LEFT) {
			xOffset = -6;
		}
		break;
	case KICK: width = 29; height = 37;
		if(player[p].direction == LEFT) {
			xOffset = -12;
			yOffset = 2;
		}
		break;
	}

	draw_clear_rectangle(
		player[p].x + xOffset,
		player[p].y + yOffset, 
		player[p].x + xOffset + width,
		player[p].y + yOffset - height);
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
}

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
	
	draw_circle(0, 1);
	draw_circle(1, 0);
	draw_circle(2, 0);
	draw_circle(3, 1);
	
	draw_ready_p(player[0].x, player[0].y, false);	// mirror false
	draw_ready_p(player[1].x, player[1].y, true);	// mirror true
	
	draw_ready();
	nrf_delay_ms(3000);
	draw_go();

	start_game_timer();
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
