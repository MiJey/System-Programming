#include "come_on.h"

Player player[2];

static void move_left(uint8_t p) {}
static void move_right(uint8_t p) {
	draw_clear_previous(player[p].x, player[p].y, player[p].x + 3, player[p].y - 39);	// 18x40
	
	if(player[p].x <= 105) { player[p].x += 3; }
	
	if(player[p].tag) { draw_walk_a(player[p].x, player[p].y);
	} else { draw_walk_b(player[p].x, player[p].y); }
	player[p].tag = !player[p].tag;
}

void move_player(uint8_t num) {
	switch(player[num].status) {
	case READY: break;
	case WALK:
		if(player[num].direction == LEFT) { move_left(num); }
		else { move_right(num); }
		break;
	case DEFENSE: break;
	case PUNCH: break;
	case KICK: break;
	case JUMP: break;
	}
}

void stop_player(uint8_t num) {
	draw_ready_p(player[num].x, player[num].y);
}

void game_ready() {
	player[0].x = 9;
	player[0].y = 39;
	player[0].hp = 48;
	player[0].tag = true;
	player[0].status = READY;
	player[0].direction = RIGHT;
	
	player[1].x = 101;
	player[1].y = 39;
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
	
	draw_ready_p(player[0].x, player[0].y);
	draw_ready_p(player[1].x, player[1].y);	
	player[0].direction = NEUTRAL;
	player[1].direction = NEUTRAL;
	
	draw_ready();
}
