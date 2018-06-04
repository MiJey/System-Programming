#include "come_on.h"

int main(void)
{
	lcd_init();
	//ble_start();

	// lcd
	draw_rectangle(0x0A, 0x30, 0x28, 0x40);
	draw_rectangle(5, 80, 20, 125);
	draw_walk_a(0, 0);
	nrf_delay_ms(500);
	draw_walk_b(0, 0);
	
	scheduler_test();
	// Enter main loop.
	while(1) {
		app_sched_execute();
//		__WFI();
		sd_app_evt_wait();
//		if (NRF_LOG_PROCESS() == false) {
//			power_manage();
//		}
	}
}
