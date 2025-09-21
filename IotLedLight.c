#include <stdio.h>
#include "pico/stdlib.h"
#include "FreeRTOS.h"
#include  "task.h"
#include "queue.h"

#define LED_PIN 13

typedef enum { LED_OFF_MSG,LED_ON_MSG}msg_t;

static QueueHandle_t queueHandle;

void blinkTask(void *param){
    bool ledState=false;
    while(true){
        ledState=!ledState;
        gpio_put(LED_PIN,ledState);
        msg_t msg= ledState?LED_OFF_MSG:LED_ON_MSG;
        xQueueSend(queueHandle,&msg,0);
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}
void loggerTask(void * param){
    msg_t msg;
    while(true){
        if(xQueueReceive(queueHandle,&msg,portMAX_DELAY)){
            if(msg==LED_OFF_MSG){
                printf("LED off\n");
            }
            else{
                printf("LED on\n");
            }
        }
    }
}
int main()
{
    stdio_init_all();
    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN,GPIO_OUT);
    queueHandle=xQueueCreate(4,sizeof(msg_t));
    xTaskCreate(blinkTask,"Blink task",256,NULL,tskIDLE_PRIORITY+1,NULL);
    xTaskCreate(loggerTask,"Logger task",256,NULL,tskIDLE_PRIORITY+1,NULL);
    vTaskStartScheduler();
    while(true){}
}
