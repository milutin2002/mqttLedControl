#include <stdio.h>
#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"
#include "FreeRTOS.h"
#include  "task.h"
#include "queue.h"
#include "event_groups.h"

#define LED_PIN 13

#ifndef WIFI_SSID
#define WIFI_SSID "Change me"
#endif

#ifndef WIFI_PASS
#define WIFI_PASS "Change me"
#endif

typedef enum { LED_OFF_MSG,LED_ON_MSG}msg_t;

static QueueHandle_t queueHandle;







void wifiTask(void * param){
    if(cyw43_arch_init()){
        printf("Wifi task problem\n");
        while(true){
            vTaskDelay(pdMS_TO_TICKS(1000));
        }
    }
    cyw43_arch_enable_sta_mode();
    cyw43_wifi_pm(&cyw43_state, CYW43_PM2_POWERSAVE_MODE);
    int rc;
    while((rc=cyw43_arch_wifi_connect_timeout_ms(WIFI_SSID,WIFI_PASS,CYW43_AUTH_WPA2_MIXED_PSK,20000))!=0){
        printf("Retrying wifi passwords with ");
        printf(WIFI_SSID);
        printf(" ");
        printf(WIFI_PASS);
        printf("\n");
        printf("Some other error happened with code %d\n",rc);
        int st = cyw43_wifi_link_status(&cyw43_state, CYW43_ITF_STA);
        printf("Wifi connected with rc=%d",rc);

        vTaskDelay(pdMS_TO_TICKS(3000));  
    }
     uint8_t *ip_address = (uint8_t*)&(cyw43_state.netif[0].ip_addr.addr);
        while(true){
            printf("IP address %d.%d.%d.%d\n", ip_address[0], ip_address[1], ip_address[2], ip_address[3]);
        }
}

void blinkTask(void *param){
    bool ledState=false;
    while(true){
        ledState=!ledState;
        gpio_put(LED_PIN,ledState);
        //msg_t msg= ledState?LED_OFF_MSG:LED_ON_MSG;
        //xQueueSend(queueHandle,&msg,0);
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
    //wifiTask(NULL);
    xTaskCreate(wifiTask,"Wifi task",1024,NULL,tskIDLE_PRIORITY+1,NULL);
    xTaskCreate(blinkTask,"Blink task",256,NULL,tskIDLE_PRIORITY+1,NULL);
    //xTaskCreate(loggerTask,"Logger task",256,NULL,tskIDLE_PRIORITY+1,NULL);
    vTaskStartScheduler();
    while(true){}
}
