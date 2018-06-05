#include "come_on.h"

// Scheduler settings
#define SCHED_MAX_EVENT_DATA_SIZE       sizeof(nrf_drv_gpiote_pin_t)
#define SCHED_QUEUE_SIZE                10

#define P1_FN_BUTTON                    BSP_BUTTON_0
#define P1_UP_BUTTON                    BSP_BUTTON_1
#define P1_LE_BUTTON                    BSP_BUTTON_2
#define P1_RI_BUTTON                    BSP_BUTTON_3

// General application timer settings.
#define APP_TIMER_PRESCALER             16    // Value of the RTC1 PRESCALER register.
#define APP_TIMER_OP_QUEUE_SIZE         2     // Size of timer operation queues.

// Application timer ID.
APP_TIMER_DEF(m_btn_a_timer_id);


// Function returns true if called from main context (CPU in thread
// mode), and returns false if called from an interrupt context. This
// is used to show what the scheduler is using, but has little use in
// a real application.
bool main_context ( void )
{
    static const uint8_t ISR_NUMBER_THREAD_MODE = 0;
    uint8_t isr_number =__get_IPSR();
    if ((isr_number ) == ISR_NUMBER_THREAD_MODE)
    {
        return true;
    }
    else
    {
        return false;
    }
}


// Function for controlling LED's based on button presses.
void button_handler(nrf_drv_gpiote_pin_t pin)
{
    uint32_t err_code;

    // Handle button presses.
    switch (pin)
    {
    case P1_FN_BUTTON:
	draw_walk_a(0, 0);
	nrf_delay_ms(300);
	draw_walk_b(0, 0);
	nrf_delay_ms(300);
	//button_event_handler(pin, 1);
        err_code = app_timer_start(m_btn_a_timer_id, APP_TIMER_TICKS(500), NULL);
        APP_ERROR_CHECK(err_code);
        break;
    case P1_UP_BUTTON:
        err_code = app_timer_stop(m_btn_a_timer_id);
        APP_ERROR_CHECK(err_code);
        break;
    default:
        break;
    }

    // Light LED 2 if running in main context and turn it off if running in an interrupt context.
    // This has no practical use in a real application, but it is useful here in the tutorial.
    if (main_context())
    {
//        nrf_drv_gpiote_out_clear(P2_UP_BTNLED);
    }
    else
    {
//        nrf_drv_gpiote_out_set(P2_UP_BTNLED);
    }
}

// Button handler function to be called by the scheduler.
void button_scheduler_event_handler(void *p_event_data, uint16_t event_size)
{
    // In this case, p_event_data is a pointer to a nrf_drv_gpiote_pin_t that represents
    // the pin number of the button pressed. The size is constant, so it is ignored.
    button_handler(*((nrf_drv_gpiote_pin_t*)p_event_data));
}

// Button event handler.
void gpiote_event_handler(nrf_drv_gpiote_pin_t pin, nrf_gpiote_polarity_t action)
{
    // The button_handler function could be implemented here directly, but is
    // extracted to a separate function as it makes it easier to demonstrate
    // the scheduler with less modifications to the code later in the tutorial.
    // button_handler(pin);
    app_sched_event_put(&pin, sizeof(pin), button_scheduler_event_handler);
}

// Function for configuring GPIO.
void gpio_config()
{
    ret_code_t err_code;

    // Initialze driver.
    err_code = nrf_drv_gpiote_init();
    APP_ERROR_CHECK(err_code);
/*
    // Configure 3 output pins for LED's.
    nrf_drv_gpiote_out_config_t out_config = GPIOTE_CONFIG_OUT_SIMPLE(false);
    err_code = nrf_drv_gpiote_out_init(P2_FN_BTNLED, &out_config);
    APP_ERROR_CHECK(err_code);
    err_code = nrf_drv_gpiote_out_init(P2_UP_BTNLED, &out_config);
    APP_ERROR_CHECK(err_code);
    err_code = nrf_drv_gpiote_out_init(P2_LE_BTNLED, &out_config);
    APP_ERROR_CHECK(err_code);
    err_code = nrf_drv_gpiote_out_init(P2_RI_BTNLED, &out_config);
    APP_ERROR_CHECK(err_code);

    // Set output pins (this will turn off the LED's).
    nrf_drv_gpiote_out_set(P2_FN_BTNLED);
    nrf_drv_gpiote_out_set(P2_UP_BTNLED);
    nrf_drv_gpiote_out_set(P2_LE_BTNLED);
    nrf_drv_gpiote_out_set(P2_RI_BTNLED);
*/
    // Make a configuration for input pints. This is suitable for both pins in this example.
    nrf_drv_gpiote_in_config_t in_config = GPIOTE_CONFIG_IN_SENSE_HITOLO(true);
    in_config.pull = NRF_GPIO_PIN_PULLUP;

    // Configure input pins for buttons, with separate event handlers for each button.
    err_code = nrf_drv_gpiote_in_init(P1_FN_BUTTON, &in_config, gpiote_event_handler);
    APP_ERROR_CHECK(err_code);
    err_code = nrf_drv_gpiote_in_init(P1_UP_BUTTON, &in_config, gpiote_event_handler);
    APP_ERROR_CHECK(err_code);
    err_code = nrf_drv_gpiote_in_init(P1_LE_BUTTON, &in_config, gpiote_event_handler);
    APP_ERROR_CHECK(err_code);
    err_code = nrf_drv_gpiote_in_init(P1_RI_BUTTON, &in_config, gpiote_event_handler);
    APP_ERROR_CHECK(err_code);

    // Enable input pins for buttons.
    nrf_drv_gpiote_in_event_enable(P1_FN_BUTTON, true);
    nrf_drv_gpiote_in_event_enable(P1_UP_BUTTON, true);
    nrf_drv_gpiote_in_event_enable(P1_LE_BUTTON, true);
    nrf_drv_gpiote_in_event_enable(P1_RI_BUTTON, true);
}

// Timeout handler for the repeated timer
void timer_handler(void * p_context)
{
    // Toggle LED 1.
//    nrf_drv_gpiote_out_toggle(P2_FN_BTNLED);

    // Light LED 3 if running in main context and turn it off if running in an interrupt context.
    // This has no practical use in a real application, but it is useful here in the tutorial.
    if (main_context())
    {
draw_walk_a(0, 0);
//        nrf_drv_gpiote_out_clear(P2_LE_BTNLED);
    }
    else
    {
draw_walk_b(0, 0);
//        nrf_drv_gpiote_out_set(P2_LE_BTNLED);
    }
}

// Create timers
void create_timers() {
	uint32_t err_code = app_timer_create(&m_btn_a_timer_id, APP_TIMER_MODE_REPEATED, timer_handler);
	APP_ERROR_CHECK(err_code);
}

void scheduler_init() {
	// Configure GPIO's.
	gpio_config();

	// Initialize the Application timer Library.
	app_timer_init();

	// Create application timer instances.
	create_timers();

	APP_SCHED_INIT(SCHED_MAX_EVENT_DATA_SIZE, SCHED_QUEUE_SIZE);
}
