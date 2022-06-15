#include <stdio.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <esp_system.h>
#include <esp_log.h>
#include <bmp280.h>
#include <time.h>
#include <string.h>
#include "sensor.h"

#ifndef APP_CPU_NUM
#define APP_CPU_NUM PRO_CPU_NUM
#endif


static const char *TAG = "BMP280";

static float temp_int = 0.0;

void bmp280_test(void *pvParameters)
{
   bmp280_params_t params;
   bmp280_init_default_params(&params);
   bmp280_t dev;
   memset(&dev, 0, sizeof(bmp280_t));

   ESP_ERROR_CHECK(bmp280_init_desc(&dev, BMP280_I2C_ADDRESS_0, 0, CONFIG_EXAMPLE_I2C_MASTER_SDA, CONFIG_EXAMPLE_I2C_MASTER_SCL));
   ESP_ERROR_CHECK(bmp280_init(&dev, &params));

   bool bme280p = dev.id == BME280_CHIP_ID;
   ESP_LOGI(TAG, "BMP280: found %s\n", bme280p ? "BME280" : "BMP280");

   float pressure, temperature, humidity;

   while (1)
   {
      vTaskDelay(10000 / portTICK_RATE_MS);
      if (bmp280_read_float(&dev, &temperature, &pressure, &humidity) != ESP_OK)
      {
         printf("Temperature/pressure reading failed\n");
         continue;
      }

      /* float is used in printf(). you need non-default configuration in
       * sdkconfig for ESP8266, which is enabled by default for this
       * example. see sdkconfig.defaults.esp8266
       */
      temp_int = temperature;
      time_t now;
      struct tm timeinfo;
      time(&now);
      localtime_r(&now, &timeinfo);
      char strftime_buf[64];
      strftime(strftime_buf, sizeof(strftime_buf), "%c", &timeinfo);
      ESP_LOGI(TAG, "%s : Pressure: %.2f Pa, Temperature: %.2f C\n", strftime_buf, pressure, temperature);
   }
}

float readTemp()
{
   return temp_int;
}
