#include "come_on.h"

int main() {
	//blinky_come_on();

	/* Button 관련 코드 */

	/* LCD 관련 코드 */
	st7586_init();
	draw_rectangle(0x0A, 0x30, 0x28, 0x40);
	draw_rectangle(5, 80, 20, 125);
	/* BLE 관련 코드 */
}
