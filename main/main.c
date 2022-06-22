#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "fade.h"
#include "touch.h"

void app_main(void)
{
    xTaskCreate(&vTaskFadeLed, "fade_led", configMINIMAL_STACK_SIZE * 5, NULL, 5, NULL);
    registerTouchSensorTask();
}
