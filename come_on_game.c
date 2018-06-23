#include "come_on.h"

int x = 0;
bool walk = true;

void p1_move_right() {
	draw_clear(x, 39, x + 18, 0);
	x += 5;
	if(x > 128) { x = 0; }
	
	if(walk) {
		draw_walk_a(x, 39);
	} else {
		draw_walk_b(x, 39);
	}
	
	walk = !walk;
	
}

void game_ready() {
	draw_clear(0, 159, 127, 0);

	draw_hp(1, 48);
	draw_hp(2, 48);
	
	draw_circle(0, 1);
	draw_circle(1, 0);
	draw_circle(2, 0);
	draw_circle(3, 1);

	draw_ready();
	draw_time(60);
}
