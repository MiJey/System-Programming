#include "come_on.h"

int x = 0;
void p1_move_right() {
	x += 5;
	draw_clear();
	draw_walk_a(x, 39);
	nrf_delay_ms(100);
	x += 5;
	draw_clear();
	draw_jump(x, 0);
	nrf_delay_ms(100);
}

void game_ready() {
	draw_clear();

	draw_hp(1, 48);
	draw_hp(2, 48);
	
	draw_circle(0, 1);
	draw_circle(1, 0);
	draw_circle(2, 0);
	draw_circle(3, 1);

	draw_ready();
	draw_time(60);
}
