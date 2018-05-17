#include "come_on.h"

void blinky_come_on() {
	bsp_board_leds_init();

	while (true) {
		for (int i = 0; i < LEDS_NUMBER; i++) {
			bsp_board_led_invert(i);
			nrf_delay_ms(500);
			bsp_board_led_invert(i);
			nrf_delay_ms(500);
			bsp_board_led_invert(i);
			nrf_delay_ms(500);
		}
	}
}
