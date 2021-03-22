#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "sdkconfig.h"
#include "mcp9700.h"
#include "mqtt.h"
#include "wifi.h"

static const adc_channel_t channel = ADC_CHANNEL_7; //ADC1 channel 7 GPIO35

void app_main() {
    wifi_init("ssid", "password");
    
    mqtt_init("mqtts://iot.devinci.online/", "sm170975", "B!5C#Cxw");
    
    mcp9700_init(1,channel);
    uint32_t mcp_value;
    while(1) //endless loop
    {
        mcp_value = mcp9700_get_value(); 
        uint32_t mcp_temp = (mcp_value - 500)/10; 
        char Value[10];
        sprintf(Value,"%d",mcp_temp);

        mqtt_publish("sm170975/mcp9700/temp", Value);  
    }	 
}
