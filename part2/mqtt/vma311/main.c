#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "sdkconfig.h"
#include "vma311.h"
#include "mqtt.h"
#include "wifi.h"

void app_main() {
    wifi_init("ssid", "password");
    
    mqtt_init("mqtts://iot.devinci.online/", "sm170975", "B!5C#Cxw");
    
    vma311_data_t vma311_data;
    vma311_init(4); //GPIO35
    while(1) //endless loop
    {
        vma311_data = vma311_get_values();
        if (vma311_data.status == VMA311_OK)//if the transmission went fine
        {
            printf("vma311:temp:%d.%d\n", vma311_data.t_int, vma311_data.t_dec);
            printf("vma311:humidity:%d.%d\n\n", vma311_data.rh_int, vma311_data.rh_dec);
        }
        else
        {
            printf("vma311:error\n\n");//otherwise print error
        }
        
        char Value[10];
        sprintf(Value,"%d", vma311_data.t_int);

        mqtt_publish("sm170975/vma311/temp", Value);
        vTaskDelay(5000 / portTICK_PERIOD_MS);//wait for 5 seconds
    }	 
}
