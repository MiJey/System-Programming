#include "come_on.h"

int main() {
	lcd_init();

	draw_splash();
	draw_time(60);
	ble_start();

	// Enter main loop.
	while(1) {
		app_sched_execute();
		if (NRF_LOG_PROCESS() == false) {
			sd_app_evt_wait();
		}	
	}
}
