#include <stdio.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <esp_system.h>
#include <bmp280.h>
#include <string.h>

#include "sensor.h"

void app_main()
{
    while (1)
    {
        ESP_ERROR_CHECK(i2cdev_init());
        readTemp();
        vTaskDelay(1000 / portTICK_RATE_MS);
    }
}