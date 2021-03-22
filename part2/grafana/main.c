#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "sdkconfig.h"
#include "vma311.h"
#include "mqtt.h"
#include "wifi.h"
#include "mcp9700.h"
#include "bme680.h"
#include "bme680_platform.h"
#include "bme680_types.h"

static const adc_channel_t channel = ADC_CHANNEL_7; //ADC1 channel 7 GPIO35
static bme680_sensor_t* sensor = 0;


// SPI interface definitions for ESP32
#define SPI_BUS       HSPI_HOST
#define SPI_SCK_GPIO  18
#define SPI_MOSI_GPIO 23
#define SPI_MISO_GPIO 19
#define SPI_CS_GPIO   15

static void bme680_init()
{
    spi_bus_init(SPI_BUS, SPI_SCK_GPIO, SPI_MISO_GPIO, SPI_MOSI_GPIO);

    // init the sensor connected to SPI_BUS with SPI_CS_GPIO as chip select.
    sensor = bme680_init_sensor (SPI_BUS, 0, SPI_CS_GPIO);

    if (sensor)
    {
         
    }
    else
        printf("Could not initialize BME680 sensor\n");
    
}

void app_main() {
    wifi_init("ssid", "password");
    
    mqtt_init("mqtts://iot.devinci.online/", "sm170975", "B!5C#Cxw");
    
    vma311_data_t vma311_data;
    vma311_init(4);

    mcp9700_init(1,channel);
    uint32_t mcp_value;

    bme680_values_float_t values;
    bme680_init();
    uint32_t duration = bme680_get_measurement_duration(sensor);
 
    while(1) //endless loop
    {
        vma311_data = vma311_get_values();
        
        mcp_value = mcp9700_get_value(); 
        float mcp_temp = (mcp_value - 500)/10; 

        if (bme680_force_measurement (sensor))
        {
            // passive waiting until measurement results are available
            vTaskDelay (duration);
        }
        
        char Value[10];

        sprintf(Value,"%d.%d", vma311_data.t_int,vma311_data.t_dec);
        mqtt_publish("sm170975/vma311/temp", Value);

        sprintf(Value, "%d.%d", vma311_data.rh_int, vma311_data.rh_dec);
        mqtt_publish("sm170975/vma311/humidity", Value);

        sprintf(Value, "%f", mcp_temp);
        mqtt_publish("sm170975/mcp9700/temp", Value);

        sprintf(Value, "%f", values.temperature);
        mqtt_publish("sm170975/bme680/temp", Value);

        sprintf(Value, "%f", values.humidity);
        mqtt_publish("sm170975/bme680/humidity", Value);

        sprintf(Value, "%f", values.pressure);
        mqtt_publish("sm170975/bme680/pression", Value);

        sprintf(Value, "%f", values.gas_resistance);
        mqtt_publish("sm170975/bme680/gas", Value);

        vTaskDelay(5000 / portTICK_PERIOD_MS);//wait for 5 seconds
    }	 
}
