/*
 * The MIT License (MIT)
 *
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
 * ISM330DHCX Service     0200
 *  - Sensor Output       0201 | float   | Read + Notify | ax, ay, az, gx, gy, gz
 *  - Measurement Period  0001
 */

const uint8_t SparkFunBLE_ISM330DHCX::UUID128_SERVICE[16] =
{
  0xFE, 0x05, 0xC5, 0xA3, 0x7A, 0x94, 0x7E, 0x9A,
  0x61, 0x47, 0x5E, 0x00, 0x00, 0x02, 0xE0, 0x5F
};

const uint8_t SparkFunBLE_ISM330DHCX::UUID128_CHR_DATA[16] = 
{
  0xFE, 0x05, 0xC5, 0xA3, 0x7A, 0x94, 0x7E, 0x9A,
  0x61, 0x47, 0x5E, 0x00, 0x01, 0x02, 0xE0, 0x5F
};

// Constructor
SparkFunBLE_ISM330DHCX::SparkFunBLE_ISM330DHCX(void)
    : SparkFunBLE_Sensor(UUID128_SERVICE, UUID128_CHR_DATA)
{
  _imuSensor = NULL;
  _accel = _gyro = NULL;
  
  // Setup Measurement Characteristic
  _measurement.setProperties(CHR_PROPS_READ | CHR_PROPS_NOTIFY);
  _measurement.setPermission(SECMODE_OPEN, SECMODE_NO_ACCESS);
  _measurement.setFixedLen(4 * 6); // 4-byte floats, 6 values
}

err_t SparkFunBLE_ISM330DHCX::begin(SparkFun_ISM330DHCX* sensor, uint16_t sensorID)
{
  SparkFunBLE_ISM330DHCX_Wrapper accel  = SparkFunBLE_ISM330DHCX_Wrapper(_imuSensor, ACCELEROMETER, sensorID);
  SparkFunBLE_ISM330DHCX_Wrapper gyro   = SparkFunBLE_ISM330DHCX_Wrapper(_imuSensor, GYROSCOPE, sensorID + 1);
  
  _imuSensor = sensor;
  _accel = &accel;
  _gyro = &gyro;

  int32_t const period_ms = 100;

  // Invoke base class begin(), this will add Service, Measurement, and Period Characteristics
  VERIFY_STATUS( SparkFunBLE_Sensor::_begin(period_ms) );

  return ERROR_NONE;
}

void SparkFunBLE_ISM330DHCX::_measure_handler(void)
{
  float imu_data[6];

  sensors_event_t accel_evt, gyro_evt;

  _accel->getEvent(&accel_evt);
  _gyro->getEvent(&gyro_evt);

  imu_data[0] = accel_evt.acceleration.x;
  imu_data[1] = accel_evt.acceleration.y;
  imu_data[2] = accel_evt.acceleration.z;
  imu_data[3] = gyro_evt.gyro.x;
  imu_data[4] = gyro_evt.gyro.y;
  imu_data[5] = gyro_evt.gyro.z;

  _measurement.notify(imu_data, sizeof(imu_data));
}
