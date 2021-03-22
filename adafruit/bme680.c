#include <Wire.h>
#include <SPI.h>
#include <Adafruit_Sensor.h>
#include "Adafruit_BME680.h"

#define BME_SCK 22 // I2C clock SCL pin on ESP32

Adafruit_BME680 bme; //default I2C bus, no pins are assigned

void setup() {
  Serial.begin(115200);
  while (!Serial);
  Serial.println(F("BME680 test"));
  //initialize the sensor
  if (!bme.begin()) {
    Serial.println("Could not find a valid BME680 sensor, check wiring!");
    while (1);
  }
// Set up oversampling and filter initialization
  bme.setTemperatureOversampling(BME680_OS_8X);
  bme.setHumidityOversampling(BME680_OS_2X);
  bme.setPressureOversampling(BME680_OS_4X);
  bme.setIIRFilterSize(BME680_FILTER_SIZE_3);
  bme.setGasHeater(320, 150); // 320*C for 150 ms
}

void loop() {
  if (! bme.performReading()) {
    Serial.println("Failed to perform reading :(");
    return;
  }
  Serial.print("bme680:temp:");
  Serial.println(bme.temperature);
  
  Serial.print("bme680:humidity:");
  Serial.println(bme.humidity);

  Serial.print("bme680:pressure:");
  Serial.println(bme.pressure / 100.0); //Pressure is returned in the SI units of Pascals.
  
  Serial.print("bme680:gas:");
  Serial.println(bme.gas_resistance / 1000.0);

  Serial.println();
  delay(10000);
}
