#include <stdio.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <esp_system.h>
#include <bmp280.h>
#include <string.h>
#include "sensor.h"

#ifndef APP_CPU_NUM
#define APP_CPU_NUM PRO_CPU_NUM
#endif

bmp280_params_t params;
bmp280_t dev;
bool init = false;

static void initSensor()
{
   printf("init");
   bmp280_init_default_params(&params);
   memset(&dev, 0, sizeof(bmp280_t));

   ESP_ERROR_CHECK(bmp280_init_desc(&dev, BMP280_I2C_ADDRESS_0, 0, CONFIG_EXAMPLE_I2C_MASTER_SDA, CONFIG_EXAMPLE_I2C_MASTER_SCL));
   ESP_ERROR_CHECK(bmp280_init(&dev, &params));
   init = true;
}

float readTemp()
{

   if (!init)
      initSensor();

   bool bme280p = dev.id == BME280_CHIP_ID;
   printf("BMP280: found %s\n", bme280p ? "BME280" : "BMP280");

   float pressure, temperature, humidity;

   if (bmp280_read_float(&dev, &temperature, &pressure, &humidity) != ESP_OK)
   {
      printf("Temperature/pressure reading failed\n");
      return;
   }
   printf("Pressure: %.2f Pa, Temperature: %.2f C \n", pressure, temperature);
   return temperature;
}
