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
		if (NRF_LOG_PROCESS() == false) {
			power_manage();
		}
	}
}
