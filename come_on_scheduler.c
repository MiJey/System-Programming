#include "come_on.h"

// Scheduler settings
#define SCHED_MAX_EVENT_DATA_SIZE       sizeof(nrf_drv_gpiote_pin_t)
#define SCHED_QUEUE_SIZE                10

// Pins for LED's and buttons.
// The diodes on the DK are connected with the cathodes to the GPIO pin, so
// clearing a pin will light the LED and setting the pin will turn of the LED.
#define LED_1_PIN                       BSP_LED_0     // LED 1 on the nRF51-DK or nRF52-DK
#define LED_2_PIN                       BSP_LED_1     // LED 3 on the nRF51-DK or nRF52-DK
#define LED_3_PIN                       BSP_LED_2     // LED 3 on the nRF51-DK or nRF52-DK
#define BUTTON_1_PIN                    BSP_BUTTON_0  // Button 1 on the nRF51-DK or nRF52-DK
#define BUTTON_2_PIN                    BSP_BUTTON_1  // Button 2 on the nRF51-DK or nRF52-DK

// General application timer settings.
#define APP_TIMER_PRESCALER             16    // Value of the RTC1 PRESCALER register.
#define APP_TIMER_OP_QUEUE_SIZE         2     // Size of timer operation queues.

// Application timer ID.
APP_TIMER_DEF(m_led_a_timer_id);


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
    case BUTTON_1_PIN:
	draw_walk_a(0, 0);
	nrf_delay_ms(500);
	draw_walk_b(0, 0);
        err_code = app_timer_start(m_led_a_timer_id, APP_TIMER_TICKS(200), NULL);
        APP_ERROR_CHECK(err_code);
        break;
    case BUTTON_2_PIN:
        err_code = app_timer_stop(m_led_a_timer_id);
        APP_ERROR_CHECK(err_code);
        break;
    default:
        break;
    }

    // Light LED 2 if running in main context and turn it off if running in an interrupt context.
    // This has no practical use in a real application, but it is useful here in the tutorial.
    if (main_context())
    {
        nrf_drv_gpiote_out_clear(LED_2_PIN);
    }
    else
    {
        nrf_drv_gpiote_out_set(LED_2_PIN);
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
static void gpio_config()
{
    ret_code_t err_code;

    // Initialze driver.
    err_code = nrf_drv_gpiote_init();
    APP_ERROR_CHECK(err_code);

    // Configure 3 output pins for LED's.
    nrf_drv_gpiote_out_config_t out_config = GPIOTE_CONFIG_OUT_SIMPLE(false);
    err_code = nrf_drv_gpiote_out_init(LED_1_PIN, &out_config);
    APP_ERROR_CHECK(err_code);
    err_code = nrf_drv_gpiote_out_init(LED_2_PIN, &out_config);
    APP_ERROR_CHECK(err_code);
    err_code = nrf_drv_gpiote_out_init(LED_3_PIN, &out_config);
    APP_ERROR_CHECK(err_code);

    // Set output pins (this will turn off the LED's).
    nrf_drv_gpiote_out_set(LED_1_PIN);
    nrf_drv_gpiote_out_set(LED_2_PIN);
    nrf_drv_gpiote_out_set(LED_3_PIN);

    // Make a configuration for input pints. This is suitable for both pins in this example.
    nrf_drv_gpiote_in_config_t in_config = GPIOTE_CONFIG_IN_SENSE_HITOLO(true);
    in_config.pull = NRF_GPIO_PIN_PULLUP;

    // Configure input pins for buttons, with separate event handlers for each button.
    err_code = nrf_drv_gpiote_in_init(BUTTON_1_PIN, &in_config, gpiote_event_handler);
    APP_ERROR_CHECK(err_code);
    err_code = nrf_drv_gpiote_in_init(BUTTON_2_PIN, &in_config, gpiote_event_handler);
    APP_ERROR_CHECK(err_code);

    // Enable input pins for buttons.
    nrf_drv_gpiote_in_event_enable(BUTTON_1_PIN, true);
    nrf_drv_gpiote_in_event_enable(BUTTON_2_PIN, true);
}


// Timeout handler for the repeated timer
static void timer_handler(void * p_context)
{
    // Toggle LED 1.
    nrf_drv_gpiote_out_toggle(LED_1_PIN);

    // Light LED 3 if running in main context and turn it off if running in an interrupt context.
    // This has no practical use in a real application, but it is useful here in the tutorial.
    if (main_context())
    {
        nrf_drv_gpiote_out_clear(LED_3_PIN);
    }
    else
    {
        nrf_drv_gpiote_out_set(LED_3_PIN);
    }
}


// Create timers
static void create_timers()
{
    uint32_t err_code;

    // Create timers
    err_code = app_timer_create(&m_led_a_timer_id,
                                APP_TIMER_MODE_REPEATED,
                                timer_handler);
    APP_ERROR_CHECK(err_code);
}


// Function starting the internal LFCLK oscillator.
// This is needed by RTC1 which is used by the Application Timer
// (When SoftDevice is enabled the LFCLK is always running and this is not needed).
static void lfclk_request(void)
{
    uint32_t err_code = nrf_drv_clock_init();
    APP_ERROR_CHECK(err_code);
    nrf_drv_clock_lfclk_request(NULL);
}


// Main function.
void scheduler_test()
{
    // Request LF clock.
    lfclk_request();

    // Configure GPIO's.
    gpio_config();

    // Initialize the Application timer Library.
    app_timer_init();

    // Create application timer instances.
    create_timers();

	APP_SCHED_INIT(SCHED_MAX_EVENT_DATA_SIZE, SCHED_QUEUE_SIZE);
    // Main loop.
/*
    while (true)
    {
    	app_sched_execute();
        // Wait for interrupt.
        __WFI();
    }
*/
}

