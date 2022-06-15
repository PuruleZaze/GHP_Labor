/* Hello World Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include <stdio.h>
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "esp_system.h"
#include "esp_spi_flash.h"
#include "esp_log.h"
#include <time.h>
#include <sys/time.h>

QueueHandle_t queue;
static const char* TAG = "Modul ESP_LOG";
TaskHandle_t task1_handler;
TaskHandle_t task2_handler;
time_t rawtime;
struct tm * timeinfo;
int counter = 0;



void getSystemTime_Task01(void *pvParameters){
    int messageFromQueue;
    for(;;){
        time ( &rawtime );
        timeinfo = localtime ( &rawtime );  
        printf ("Current local time and date: %s", asctime (timeinfo));
        
        vTaskDelay(1000 / portTICK_PERIOD_MS);
        counter++;
        if(counter%CONFIG_TIMER_HELLO_WORLD == 0){
            ESP_LOGW(TAG, "Running on core: %d", xPortGetCoreID());
            ESP_LOGW(TAG, "Es sind %d Sekunden vergangen!", CONFIG_TIMER_HELLO_WORLD);
            xQueueReceive(queue, &messageFromQueue, 0);
            ESP_LOGW(TAG, "Read message from queue: %d", messageFromQueue);
        }
    } 
}

void writeToQueue_Task02(void *pvParameters){
    int messageForQueue = (int) pvParameters;
    for(;;){
        //printf("Writing to a message queue: %d\n", messageForQueue);
        //printf("Running on core: %d\n", xPortGetCoreID());
        xQueueSend(queue, &messageForQueue, portMAX_DELAY);
        messageForQueue++;
    }
}
void app_main(void)
{
    queue = xQueueCreate(1, sizeof(int));
    xTaskCreate(getSystemTime_Task01, "task_1", 4096, NULL, tskIDLE_PRIORITY, &task1_handler);
    xTaskCreate(writeToQueue_Task02, "task_2", 4096, ((void *) 42),  tskIDLE_PRIORITY, &task2_handler);
} 