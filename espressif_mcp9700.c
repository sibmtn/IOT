#include <stdlib.h>
#include "mcp9700.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static mcp9700_t mcp9700;

#if CONFIG_IDF_TARGET_ESP32
// Set fields of the static mcp9700 structure to be able to access them in other function of this library.
static const adc_bits_width_t width = ADC_WIDTH_BIT_12;
static const adc_atten_t atten = ADC_ATTEN_DB_0; //0 dB attenuation => input voltage is from 0 to 1.1V
static const adc_channel_t channel = ADC_CHANNEL_7; //ADC1 channel 7 GPIO35
static esp_adc_cal_characteristics_t *adc_chars; 

#endif

// Initialize the ADC to use it with the MCP9700 sensor.
void mcp9700_init(adc_unit_t unit, adc_channel_t channel)   
{
    
    if (mcp9700.unit == ADC_UNIT_1)
    {
        // Configure the ADC: its width and its attenuation.
        adc1_config_width(width);
        adc1_config_channel_atten(channel, atten);
    }
    else /* mcp9700.unit == ADC_UNIT_2 */
    {
        adc2_config_channel_atten((adc2_channel_t)channel, atten);
    }
    // Get the characteristics of the ADC stored in the eFUSE.
    adc_chars = calloc(1, sizeof(esp_adc_cal_characteristics_t)); //the calloc function allocates a place in memory
    // with all bits equal to 0, here we indicate to the pointer adc_chars, the adress of this place in memory
    esp_adc_cal_characterize(unit, ADC_ATTEN, ADC_WIDTH, DEFAULT_VREF, adc_chars); //this function stores the ADC
    //voltage curve based on the characterisitics (parameters of the function) of the ADC, at the adress pointed by adc_chars
    mcp9700.unit = unit;//we assign the parameter values to the mcp9700 structure values
    mcp9700.channel = channel;
    mcp9700.adc_chars = *adc_chars;
}
// Get the temperature value measured by the MCP9700 by applying multi-sampling.
int32_t mcp9700_get_value()
{
    uint32_t adc_reading = 0;
    uint32_t voltage;
	    
    for (int i = 0; i < NO_OF_SAMPLES; i++) //64 samples
    {
        //Sum the values converted by the ADC.
        if (mcp9700.unit == ADC_UNIT_1)
        {
            adc_reading += adc1_get_raw(mcp9700.channel);
        }
        else /* mcp9700.unit == ADC_UNIT_2 */
        {
            int raw;
            adc2_get_raw(mcp9700.channel, width, &raw);
            adc_reading += raw;
        }
    }
    adc_reading /= NO_OF_SAMPLES; // Average these value to get a only one sample.
    voltage = esp_adc_cal_raw_to_voltage(adc_reading, adc_chars); //Convert the voltage using the ESP dedicated function.
        printf("Raw: %d\tVoltage: %dmV\n", adc_reading, voltage); //Compute and return the temperature value.
    vTaskDelay(pdMS_TO_TICKS(1000));   //wait 1 sec
    return (voltage);
}

void app_main(void){
    mcp9700_init(1,channel);
    uint32_t mcp_value;
    while(1) //endless loop
    {
        mcp_value = mcp9700_get_value(); 
        float mcp_temp = (mcp_value - 500)/10; //this function converts the voltage value in degree, it is found in the datasheet of the mcp
        printf("mcp9700:temp:%f\n", mcp_temp);
    }	
}
