#include <stdio.h>
#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"
#include "FreeRTOS.h"
#include  "task.h"
#include "queue.h"
#include "event_groups.h"

#include "lwip/api.h"
#include "lwip/err.h"
#include "lwip/apps/mqtt.h"

#define NET_READY_BIT (1U << 0)

static EventGroupHandle_t netEvents;

#define MQTT_HOST "broker.emqx.io"
#define MQTT_PORT 1883
#define TOPIC_SET "pico/led/set"
#define TOPIC_STATUS "pico/led/status"

#define LED_PIN 13

#ifndef WIFI_SSID
#define WIFI_SSID "Change me"
#endif

#ifndef WIFI_PASS
#define WIFI_PASS "Change me"
#endif

typedef enum { LED_OFF_MSG,LED_ON_MSG}msg_t;

static QueueHandle_t queueHandle;

static mqtt_client_t *mq=NULL;

static void onTopic(void *arg,const char *topic,u32_t len){
    printf("MQTT incoming topic %s\n",topic);
}
static void onData(void *arg,const u8_t *data,u16_t len,u8_t flags){
    char buf[8];
    u16_t n= len< (sizeof(buf)-1) ? len : (sizeof(buf)-1);
    memcpy(buf,data,n);
    buf[n]=0;
    msg_t cmd;
    if(strncmp(buf,"ON",2)==0){
        cmd=LED_ON_MSG;
        printf("MSG on\n");
    }
    else if(strncmp(buf,"OFF",3)==0){
        cmd=LED_OFF_MSG;
        printf("MSG off\n");
    }
    else{
        printf("Got something else %s\n",buf);
    }
}
static void onConnect(mqtt_client_t *client,void *arg,mqtt_connection_status_t st){
    if(st==MQTT_CONNECT_ACCEPTED){
        printf("MQTT connected\n");
        mqtt_set_inpub_callback(mq,onTopic,onData,NULL);
        mqtt_subscribe(mq,TOPIC_SET,0,NULL,NULL);
    }
    else{
        printf("Mqtt connection failed\n");
    }
}

void mqttTask( void * _){
    xEventGroupWaitBits(netEvents,NET_READY_BIT,pdFALSE,pdTRUE,portMAX_DELAY);
    ip_addr_t broker_ip;
    err_t de=netconn_gethostbyname(MQTT_HOST,&broker_ip);
    if(de!=ERR_OK){
        while(true){
            printf("DNS failed (%d)\n",(int)de);
        }
        vTaskDelete(NULL);
        return;
    }
    printf("Broker %s -> %s\n",MQTT_HOST,ipaddr_ntoa(&broker_ip));
    mq=mqtt_client_new();
    struct mqtt_connect_client_info_t ci;
    memset(&ci,0,sizeof(ci));
    ci.client_id="pico-led-1";
    ci.keep_alive=30;
    ci.will_topic=TOPIC_STATUS;
    ci.will_msg="OFF";
    ci.will_qos=0;
    ci.will_retain=1;
    while(true){
        if(!mqtt_client_is_connected(mq)){
            printf("MQTT connecting\n");
           // cyw43_arch_lwip_begin();
            mqtt_client_connect(mq,&broker_ip,MQTT_PORT,onConnect,NULL,&ci);
            //cyw43_arch_lwip_end();
        }
        vTaskDelay(pdMS_TO_TICKS(5000));
    }
}

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
    printf("IP address %d.%d.%d.%d\n", ip_address[0], ip_address[1], ip_address[2], ip_address[3]);
    xEventGroupSetBits(netEvents,NET_READY_BIT);
    vTaskDelete(NULL);
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
    netEvents=xEventGroupCreate();
    //wifiTask(NULL);
    xTaskCreate(wifiTask,"Wifi task",1024,NULL,tskIDLE_PRIORITY+3,NULL);
    xTaskCreate(mqttTask,"Mqtt task",1024,NULL,tskIDLE_PRIORITY+2,NULL);
    xTaskCreate(blinkTask,"Blink task",256,NULL,tskIDLE_PRIORITY+1,NULL);
    //xTaskCreate(loggerTask,"Logger task",256,NULL,tskIDLE_PRIORITY+1,NULL);
    vTaskStartScheduler();
    while(true){}
}
