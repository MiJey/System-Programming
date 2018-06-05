#include "come_on.h"

int main(void)
{
	lcd_init();
	ble_start();

	draw_walk_a(0, 0);
	nrf_delay_ms(500);
	draw_walk_b(0, 0);

	// Enter main loop.
	while(1) {
		app_sched_execute();
		if (NRF_LOG_PROCESS() == false) {
			sd_app_evt_wait();
		}
	}
}
