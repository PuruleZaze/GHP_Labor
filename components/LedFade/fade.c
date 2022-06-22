#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <stdio.h>
#include "driver/ledc.h"
#include "esp_err.h"
#include "freertos/semphr.h"

#include "fade.h"

#if CONFIG_IDF_TARGET_ESP32
#define LEDC_HS_TIMER LEDC_TIMER_0
#define LEDC_HS_MODE LEDC_HIGH_SPEED_MODE
#define LEDC_HS_CH0_GPIO GPIO_NUM_5
#define LEDC_HS_CH0_CHANNEL LEDC_CHANNEL_0
#endif
#define LEDC_TEST_CH_NUM (1)
#define LEDC_TEST_DUTY (4000)
#define LEDC_TEST_FADE_TIME (3000)

// notification that fade is finished via semaphore
static bool cb_ledc_fade_end_event(const ledc_cb_param_t *param, void *user_arg)
{
    portBASE_TYPE taskAwoken = pdFALSE;

    if (param->event == LEDC_FADE_END_EVT)
    {
        SemaphoreHandle_t counting_sem = (SemaphoreHandle_t)user_arg;
        xSemaphoreGiveFromISR(counting_sem, &taskAwoken);
    }

    return (taskAwoken == pdTRUE);
}

void vTaskFadeLed(void *pvParameters)
{
    // setup timer for pwm generation
    ledc_timer_config_t ledc_timer = {
        .duty_resolution = LEDC_TIMER_13_BIT,
        .freq_hz = 5000,
        .speed_mode = LEDC_HS_MODE,
        .timer_num = LEDC_HS_TIMER,
        .clk_cfg = LEDC_AUTO_CLK,
    };
    ledc_timer_config(&ledc_timer);

    // setup channel for ledc
    ledc_channel_config_t ledc_channel = {
        .channel = LEDC_HS_CH0_CHANNEL,
        .duty = 0,
        .gpio_num = LEDC_HS_CH0_GPIO,
        .speed_mode = LEDC_HS_MODE,
        .hpoint = 0,
        .timer_sel = LEDC_HS_TIMER,
        .flags.output_invert = 0,
    };

    ledc_channel_config(&ledc_channel);

    ledc_fade_func_install(0);

    ledc_cbs_t callbacks = {
        .fade_cb = cb_ledc_fade_end_event,
    };

    SemaphoreHandle_t counting_sem = xSemaphoreCreateCounting(LEDC_TEST_CH_NUM, 0);

    ledc_cb_register(ledc_channel.speed_mode, ledc_channel.channel, &callbacks, (void *)counting_sem);

    while (1)
    {
        printf("1. LEDC fade up to duty = %d\n", LEDC_TEST_DUTY);
        ledc_set_fade_with_time(ledc_channel.speed_mode,
                                ledc_channel.channel, LEDC_TEST_DUTY, LEDC_TEST_FADE_TIME);
        ledc_fade_start(ledc_channel.speed_mode,
                        ledc_channel.channel, LEDC_FADE_NO_WAIT);

        // wait till fade is over and semaphore is available
        xSemaphoreTake(counting_sem, portMAX_DELAY);

        printf("2. LEDC fade down to duty = 0\n");
        ledc_set_fade_with_time(ledc_channel.speed_mode,
                                ledc_channel.channel, 0, LEDC_TEST_FADE_TIME);
        ledc_fade_start(ledc_channel.speed_mode,
                        ledc_channel.channel, LEDC_FADE_NO_WAIT);

        // wait till fade is over and semaphore is available
        xSemaphoreTake(counting_sem, portMAX_DELAY);
    }
}