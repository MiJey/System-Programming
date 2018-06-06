#include "come_on.h"

int main(void)
{
	lcd_init();

	draw_splash();
	nrf_delay_ms(1000);
	ble_start();

	

	// Enter main loop.
	while(1) {
		app_sched_execute();
		if (NRF_LOG_PROCESS() == false) {
			sd_app_evt_wait();
		}
	}
}
