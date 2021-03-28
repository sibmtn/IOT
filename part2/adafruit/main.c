#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "sdkconfig.h"
#include "wifi.h"
#include "aio.h"
#include "mcp9700.h"

static const adc_channel_t channel = ADC_CHANNEL_7; //ADC1 channel 7 GPIO35

void app_main() {
    wifi_init("wifiname", "password");

    aio_init("sibmartin", "key");

    mcp9700_init(1,channel);
    uint32_t mcp_value;
    while(1) //endless loop
    {
        mcp_value = mcp9700_get_value(); 
        uint32_t mcp_temp = (mcp_value - 500)/10; //this function converts the voltage value in degree, it is found in the datasheet of the mcp
        char ValueIO[10];
        sprintf(ValueIO,"%d",mcp_temp); // I have to convert the value in a char*

        aio_create_data(ValueIO, "envmon.mcp9700-temp");
    }	
}
