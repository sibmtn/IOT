#include "vma311.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static vma311_t vma311;

/* static function prototypes */
static void vma311_send_start_signal();
static int vma311_wait(uint16_t, int);
static int vma311_check_response();
static inline vma311_status_t vma311_read_byte(uint8_t *);
static vma311_status_t vma311_check_crc(uint8_t *);

// Initialize VMA311.
void vma311_init(gpio_num_t num)
{
    vma311.num = num;
    vma311.last_read_time = -2000000;
    esp_err_t gpio_reset_pin(gpio_num_t num); // Reset the GPIO pins
    vTaskDelay(pdMS_TO_TICKS(1000));// Delay 1 second until the sensor becomes stable
}

//Send start signal to VMA311.
void vma311_send_start_signal()
{
    gpio_set_direction(vma311.num, GPIO_MODE_OUTPUT);
    gpio_set_level(vma311.num, 0); //LOW for at least 18ms to let DHT11 the time to receive
    ets_delay_us(20 * 1000);
    gpio_set_level(vma311.num, 1); // HIGH and wait for the DHT11 response for 40 microseconds
    ets_delay_us(40);
}
// Get the values send by the VMA311 sensor.
vma311_data_t vma311_get_values()
{
    vma311_data_t error_data = {VMA311_TIMEOUT_ERROR, -1, -1, -1, -1};
    uint8_t data[5] = {0, 0, 0, 0, 0}; //initialize an array to store the values of each of the 5 bytes sent by the sensor:
    //la partie entière de l'humidité relative, la partie décimale, la partie entière de la température et enfin sa partie décimale.
    //plus une somme de contrôle

    if (esp_timer_get_time() - 2000000 < vma311.last_read_time) //if the duration since the last reading of the sensor is under 2000000 ticks, the program returns the previous measures that the sensor has done
    {
        return vma311.data;
    }
    vma311.last_read_time = esp_timer_get_time(); //otherwise we keep going and we set the time of the last reading to the beginning of this reading
    vma311_send_start_signal(); 
    if (vma311_check_response() == VMA311_TIMEOUT_ERROR)
    {
        return error_data;
    }
    for (int i = 0; i < 5; ++i)
    {
        if (vma311_read_byte(&data[i])) ////this function converts the bits sent by the sensor to a digital value
        {
            return error_data;
        }
    }
    if(vma311_check_crc(data) != VMA311_CRC_ERROR) // if there's no transmission error, then the data sent by the sensor is correct and we assign them to the sensor object
    {
        vma311.data.rh_int = data[0];
        vma311.data.rh_dec = data[1];
        vma311.data.t_int = data[2];
        vma311.data.t_dec = data[3];
        vma311.data.status = VMA311_OK;

    }
    return vma311.data;
}
//Wait for a number of microseconds at a given level.
int vma311_wait(uint16_t us, int level)
{
    int us_ticks = 0;

    while(gpio_get_level(vma311.num) == level)
    { 
        if(us_ticks++ > us) 
        {
            return VMA311_TIMEOUT_ERROR;
        }
        ets_delay_us(1);
    }
    return us_ticks;
}

//Check the response of the VMA311 sensor.
int vma311_check_response()
{
    gpio_set_direction(vma311.num, GPIO_MODE_INPUT);
    if(vma311_wait(81, 0) == VMA311_TIMEOUT_ERROR) //duration =81 microseconds, level = LOW
    {
        return VMA311_TIMEOUT_ERROR;
    }
    if(vma311_wait(81, 1) == VMA311_TIMEOUT_ERROR) //duration =81 microseconds, level = HIGH
    {
        return VMA311_TIMEOUT_ERROR;
    }
    return VMA311_OK;
}

vma311_status_t vma311_read_byte(uint8_t *byte)
{
    for (int i = 0; i < 8; ++i)
    {
        if(vma311_wait(50, 0) == VMA311_TIMEOUT_ERROR) //before transmitting a bit, the sensor normally begins with a 50 us low level on the pin
        {
            return VMA311_TIMEOUT_ERROR;
        }
        if(vma311_wait(70, 1) > 28)  //if the high level on the pin is longer than 28us it means that the sensor is sending a bit of one
        {
            *byte |= (1 << (7 - i));
        }
    }
    return VMA311_OK;
}
// Check the checksum to verify that data are consistent.
vma311_status_t vma311_check_crc(uint8_t data[])
{
  int sum = 0;
    for (int i = 0; i < 4; ++i) //sum of each value sent by the sensor
    {
        sum += data[i];
    }
    if (sum != data[4])//if the sum is not equal to the checksum (stored in the last spot of the array), it means there's a communication error
    {
        return VMA311_CRC_ERROR;
    }
    else//otherwise the data is fine
    {
        return VMA311_OK;
    }
}

void app_main()
{
    vma311_data_t vma311_data;
    vma311_init(4); //GPIO35
    while(1) //endless loop
    {
        vma311_data = vma311_get_values();//this  function returns the array containing the relative humidity' integer and decimal parts and the temperature's decimal and integral parts and the status of the sensor
        if (vma311_data.status == VMA311_OK)//if the transmission went fine
        {
            printf("vma311:temp:%d.%d\n", vma311_data.t_int, vma311_data.t_dec);
            printf("vma311:humidity:%d.%d\n\n", vma311_data.rh_int, vma311_data.rh_dec);
        }
        else
        {
            printf("vma311:error\n\n");//otherwise print error
        }
        vTaskDelay(5000 / portTICK_PERIOD_MS);//wait for 5 seconds
    }
}
