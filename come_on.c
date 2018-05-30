#include "come_on.h"

// TODO button_action이 바뀔 때 걷기 멈추기 callback
void come_on_button_right(uint8_t button_action) {
	draw_walk_a(0, 0);
	nrf_delay_ms(500);
	draw_walk_b(0, 0);
	nrf_delay_ms(500);
	draw_walk_a(0, 0);
	nrf_delay_ms(500);
	draw_walk_b(0, 0);
	nrf_delay_ms(500);
	draw_walk_a(0, 0);
	nrf_delay_ms(500);
	draw_walk_b(0, 0);
	nrf_delay_ms(500);
}
