#include "come_on.h"

Player player[2];

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
		// 범위를 벗어났는지 체크
		if(player[p].x < 0) { player[p].x = 0; }
		if(player[p].x > 108) { player[p].x = 108; }
		player[p].y = 39;
		
		stop_player_timer(p);	// 동작 그만
		draw_ready_p(player[p].x, player[p].y, player[p].direction == LEFT);
		break;
		
	case WALK:
		player[p].y = 39;
		
		if(player[p].direction == LEFT) {
			// 왼쪽으로 걷기, 미러모드 true
			if(player[p].x > 0) { player[p].x -= 3; }
			player[p].tag ? draw_walk_a(player[p].x, player[p].y, true) : draw_walk_b(player[p].x, player[p].y, true);
			player[p].tag = !player[p].tag;
		} else {
			// 오른쪽으로 걷기, 미러모드 false
			if(player[p].x < 108) { player[p].x += 3; }
			player[p].tag ? draw_walk_a(player[p].x, player[p].y, false) : draw_walk_b(player[p].x, player[p].y, false);
			player[p].tag = !player[p].tag;
		}
		break;
		

	case DJUMP:
		// 양 옆으로 이동
		if(player[p].direction == LEFT) { if(player[p].x > 0) { player[p].x -= 3; } }
		else { if(player[p].x < 108) { player[p].x += 3; } }
	case JUMP: {
		if(player[p].x > 96) { player[p].x = 96; }	// 오른쪽 벽에 붙어서 점프하면 밀려나도록
		player[p].y = 37;
		
		// 점프 동작 수행
		uint8_t v = 20;	              // 속도
		uint8_t g = 3;	              // 중력 가속도
		uint8_t t = player[p].t;      // 시간
		uint8_t y0 = 39;              // 준비자세일 때 높이(초기값)
		uint8_t y = y0 + v*t - g*t*t;
	
		if(y < y0) {
			player[p].t = 0;
			if(player[p].status == DJUMP) { player[p].newStatus = WALK; }
			else { player[p].newStatus = READY; }
		} else {
			player[p].y = y;
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
		
		player[p].newStatus = DEFENSE;
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
		
		player[p].newStatus = DEFENSE;
		break;
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
