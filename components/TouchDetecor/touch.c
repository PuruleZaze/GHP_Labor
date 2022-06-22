#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "esp_log.h"
#include "esp_system.h"

#include "driver/touch_pad.h"
#include "soc/rtc_periph.h"
#include "soc/sens_periph.h"
#include "driver/gpio.h"
#include "sdkconfig.h"

#include "touch.h"

#define LED_GPIO GPIO_NUM_2
#define TOUCH_NUM 4
#define TOUCH_THRESH_NO_USE (0)
#define TOUCH_THRESH_PERCENT (80)
#define TOUCHPAD_FILTER_TOUCH_PERIOD (10)

static const char *TAG = "Touch pad";

static bool s_pad_activated;
static uint32_t s_pad_init_val;

static void tp_example_set_thresholds(void)
{
    uint16_t touch_value;

    // read filtered value
    touch_pad_read_filtered(TOUCH_NUM, &touch_value);
    s_pad_init_val = touch_value;
    ESP_LOGI(TAG, "test init: touch pad [%d] val is %d", TOUCH_NUM, touch_value);
    // set interrupt threshold.
    ESP_ERROR_CHECK(touch_pad_set_thresh(TOUCH_NUM, touch_value * 2 / 3));
}

void vTaskTouchDetector(void *pvParameters)
{
    static int show_message;
    // remember if led is on or off
    static bool ledOn;
    while (1)
    {
        // interrupt mode, enable touch interrupt
        touch_pad_intr_enable();

        if (s_pad_activated == true)
        {
            ESP_LOGI(TAG, "T%d activated!", TOUCH_NUM);
            // toggle led
            if (ledOn)
                gpio_set_level(LED_GPIO, 0);
            else
                gpio_set_level(LED_GPIO, 1);
            ledOn = !ledOn;

            vTaskDelay(200 / portTICK_PERIOD_MS);
            s_pad_activated = false;
            show_message = 1;
        }

        vTaskDelay(10 / portTICK_PERIOD_MS);

        if (show_message++ % 500 == 0)
        {
            ESP_LOGI(TAG, "Waiting for any pad being touched...");
        }
    }
}

static void tp_example_rtc_intr(void *arg)
{
    uint32_t pad_intr = touch_pad_get_status();
    // clear interrupt
    touch_pad_clear_status();
    // check if sensor is on
    if ((pad_intr >> TOUCH_NUM) & 0x01)
    {
        s_pad_activated = true;
    }
}

static void tp_example_touch_pad_init(void)
{
    for (int i = 0; i < TOUCH_PAD_MAX; i++)
    {
        touch_pad_config(i, TOUCH_THRESH_NO_USE);
    }
}

void registerTouchSensorTask(void)
{
    ESP_LOGI(TAG, "Initializing touch pad");
    ESP_ERROR_CHECK(touch_pad_init());
    touch_pad_set_fsm_mode(TOUCH_FSM_MODE_TIMER);
    touch_pad_set_voltage(TOUCH_HVOLT_2V7, TOUCH_LVOLT_0V5, TOUCH_HVOLT_ATTEN_1V);
    tp_example_touch_pad_init();
    touch_pad_filter_start(TOUCHPAD_FILTER_TOUCH_PERIOD);
    tp_example_set_thresholds();
    touch_pad_isr_register(tp_example_rtc_intr, NULL);
    gpio_pad_select_gpio(LED_GPIO); // Set pin as GPIO
    gpio_set_direction(LED_GPIO, GPIO_MODE_OUTPUT);

    xTaskCreate(&vTaskTouchDetector, "touch_pad_read_task", 2048, NULL, 5, NULL);
}
