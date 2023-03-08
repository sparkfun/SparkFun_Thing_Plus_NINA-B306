/* 
 * The MIT License (MIT)
 *
 * Copyright (c) 2019 Ha Thach (tinyusb.org) for Adafruit Industries
 * Copyright (c) 2023 SparkFun Electronics
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#include "SparkFunBLEService.h"

//--------------------------------------------------------------------+
// MACRO TYPEDEF CONSTANT ENUM DECLARATION
//--------------------------------------------------------------------+

/* All SparkFun Service/Characteristic UUID128 share the same Base UUID:
 *    5FE0xxxx-005E-4761-9A7E-947AA3C505FE
 *
 * Shared Characteristics
 *  - Measurement Period  0001 | int32_t | Read + Write |
 *    ms between measurements, -1: stop reading, 0: update when changes
 *
 * BME280 Sensor Service  0100
 *  - Sensor Output       0101 | float   | Read + Notify | degC, pres in Pa, %RH
 *  - Measurement Period  0001
 */

const uint8_t SparkFunBLE_BME280::UUID128_SERVICE[16] =
{
  0xFE, 0x05, 0xC5, 0xA3, 0x7A, 0x94, 0x7E, 0x9A,
  0x61, 0x47, 0x5E, 0x00, 0x00, 0x01, 0xE0, 0x5F
};

const uint8_t SparkFunBLE_BME280::UUID128_CHR_DATA[16] = 
{
  0xFE, 0x05, 0xC5, 0xA3, 0x7A, 0x94, 0x7E, 0x9A,
  0x61, 0x47, 0x5E, 0x00, 0x01, 0x01, 0xE0, 0x5F
};

// Constructor
SparkFunBLE_BME280::SparkFunBLE_BME280(void)
  : SparkFunBLE_Sensor(UUID128_SERVICE, UUID128_CHR_DATA)
{
  _envSensor = NULL;
  _temp = _press = _humidity = NULL;

  // Setup Measurement Characteristic
  _measurement.setProperties(CHR_PROPS_READ | CHR_PROPS_NOTIFY);
  _measurement.setPermission(SECMODE_OPEN, SECMODE_NO_ACCESS);
  _measurement.setFixedLen(4*3);
}

err_t SparkFunBLE_BME280::begin(BME280* sensor, uint16_t sensorID, int ms)
{
  SparkFunBLE_BME280_Wrapper temp = SparkFunBLE_BME280_Wrapper(sensor, TEMPERATURE, sensorID);
  SparkFunBLE_BME280_Wrapper press = SparkFunBLE_BME280_Wrapper(sensor, PRESSURE, sensorID + 1);
  SparkFunBLE_BME280_Wrapper humidity = SparkFunBLE_BME280_Wrapper(sensor, REL_H, sensorID + 2);
  
  _envSensor = sensor;
  _temp = &temp;
  _press = &press;
  _humidity = &humidity;

  VERIFY_STATUS( SparkFunBLE_Sensor::_begin(ms) );

  return ERROR_NONE;
}

// Notifiy BLE service
void SparkFunBLE_BME280::_measure_handler(void)
{
  float env_data[3];

  sensors_event_t temp_evt, press_evt, humid_evt;

  _temp->getEvent(&temp_evt);
  _press->getEvent(&press_evt);
  _humidity->getEvent(&humid_evt);

  env_data[0] = temp_evt.temperature;
  env_data[1] = press_evt.pressure;
  env_data[2] = humid_evt.relative_humidity;

  _measurement.notify(env_data, sizeof(env_data));
}
