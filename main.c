#include "come_on.h"

int main(void)
{
	ble_start();

	// lcd
	lcd_init();
	draw_rectangle(0x0A, 0x30, 0x28, 0x40);
	draw_rectangle(5, 80, 20, 125);
	draw_rectangle(0, 0, 10, 10);

	// Enter main loop.
	while(1) {
bsp_board_led_invert(0);
nrf_delay_ms(500);
		if (NRF_LOG_PROCESS() == false) {
bsp_board_led_invert(3);
nrf_delay_ms(500);
			power_manage();
bsp_board_led_invert(3);
nrf_delay_ms(500);
		}
	}
}
