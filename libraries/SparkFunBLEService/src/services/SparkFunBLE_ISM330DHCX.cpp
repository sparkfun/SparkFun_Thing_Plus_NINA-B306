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
#include <Arduino.h>

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

const uint8_t SparkFunBLE_ISM330DHCX::UUID128_SERVICE[16] = {
  0xFE, 0x05, 0xC5, 0xA3, 0x7A, 0x94, 0x7E, 0x9A,
  0x61, 0x47, 0x5E, 0x00, 0x00, 0x02, 0xE0, 0x5F
};

const uint8_t SparkFunBLE_ISM330DHCX::UUID128_CHR_DATA[16] = {
  0xFE, 0x05, 0xC5, 0xA3, 0x7A, 0x94, 0x7E, 0x9A,
  0x61, 0x47, 0x5E, 0x00, 0x01, 0x02, 0xE0, 0x5F
};

// Constructor
SparkFunBLE_ISM330DHCX::SparkFunBLE_ISM330DHCX(void)
    : SparkFunBLE_Sensor(UUID128_SERVICE, UUID128_CHR_DATA) { 
  _imuSensorPtr = NULL;
  _accel = _gyro = NULL;
  
  // Setup Measurement Characteristic
  _measurement.setProperties(CHR_PROPS_READ | CHR_PROPS_NOTIFY);
  _measurement.setPermission(SECMODE_OPEN, SECMODE_NO_ACCESS);
  _measurement.setFixedLen(4 * 6); // 4-byte floats, 6 values
}

err_t SparkFunBLE_ISM330DHCX::begin(SparkFun_ISM330DHCX* sensor, uint16_t sensorID) {
  _imuSensorPtr = sensor;

  static SparkFunBLE_ISM330DHCX_Wrapper accel  = SparkFunBLE_ISM330DHCX_Wrapper(_imuSensorPtr, ACCELEROMETER, sensorID);
  static SparkFunBLE_ISM330DHCX_Wrapper gyro   = SparkFunBLE_ISM330DHCX_Wrapper(_imuSensorPtr, GYROSCOPE, sensorID + 1);

  _accel = &accel;
  _gyro = &gyro;

  const uint32_t period_ms = 500;

  // Invoke base class begin(), this will add Service, Measurement, and Period Characteristics
  VERIFY_STATUS( _begin(period_ms) );

  return ERROR_NONE;
}

err_t SparkFunBLE_ISM330DHCX::_begin(uint32_t ms) {
  // Invoke base class begin()
  VERIFY_STATUS( BLEService::begin() );

  _measurement.setCccdWriteCallback(sensor_data_cccd_cb, true);
  VERIFY_STATUS( _measurement.begin() );
  _measurement.write32(0); // zero 4 bytes could help to init some services

  _period.setProperties(CHR_PROPS_READ | CHR_PROPS_WRITE);
  _period.setPermission(SECMODE_OPEN, SECMODE_OPEN);
  _period.setFixedLen(4);
  VERIFY_STATUS( _period.begin() );
  _period.write32(ms);

  _period.setWriteCallback(sensor_period_write_cb, true);

  // setup timer
  _timer.begin(ms, sensor_timer_cb, this, true);

  return ERROR_NONE;
}

void SparkFunBLE_ISM330DHCX::setPeriod(int period_ms) {
  if( period_ms > 100 ){
    _period.write32(period_ms);
    _update_timer(period_ms);
  }
}

void SparkFunBLE_ISM330DHCX::_update_timer(int32_t ms) {
  if ( ms < 0 ) {
    _timer.stop();
  }
  else if ( ms > 0 ) {
    _timer.setPeriod(ms);
  }
  else {
    // Period = 0: keep the current interval, but report on changes only
  }
}

void SparkFunBLE_ISM330DHCX::_measure_handler(void) {
  uint16_t len = _measurement.getMaxLen();
  uint8_t imu_data[len];
  
  if(_imuSensorPtr && _measurement.isFixedLen()) {
    sensors_event_t accel_evt, gyro_evt;

    _accel->getEvent(&accel_evt);
    _gyro->getEvent(&gyro_evt);

    uint16_t offset = 0;
    memcpy(imu_data + offset, (const void*) &accel_evt.acceleration.x, sizeof(float));
    offset += sizeof(float);
    memcpy(imu_data + offset, (const void*) &accel_evt.acceleration.y, sizeof(float));
    offset += sizeof(float);
    memcpy(imu_data + offset, (const void*) &accel_evt.acceleration.z, sizeof(float));
    offset += sizeof(float);
    memcpy(imu_data + offset, (const void*) &gyro_evt.gyro.x, sizeof(float));
    offset += sizeof(float);
    memcpy(imu_data + offset, (const void*) &gyro_evt.gyro.y, sizeof(float));
    offset += sizeof(float);
    memcpy(imu_data + offset, (const void*) &gyro_evt.gyro.z, sizeof(float));
  }
  else if (_measure_cb) {
    len = _measure_cb((uint8_t*)imu_data, sizeof(imu_data));
    len = min(len, sizeof(imu_data));
  }
  else {
    return;
  }

  if (!len){
    return;
  }

  if ( 0 == _period.read32() ) {
    uint8_t prev_buf[_measurement.getMaxLen()];
    _measurement.read(prev_buf, sizeof(prev_buf));

    // skip notify if there is no changes
    if ( 0 == memcmp(prev_buf, imu_data, len) ) return;
  }

  _measurement.notify(imu_data, sizeof(imu_data));
}

void SparkFunBLE_ISM330DHCX::_notify_handler(uint16_t conn_hdl, uint16_t value) {
  if (value & BLE_GATT_HVX_NOTIFICATION) {
    _timer.start();
  }
  else {
    _timer.stop();
  }

  if (_notify_cb) _notify_cb(conn_hdl, value);

  // send initial notification if period = 0
  if ( 0 == _period.read32() ) {
    _measure_handler();
    // _measurement.notify();
  }
}

void SparkFunBLE_ISM330DHCX::sensor_timer_cb(TimerHandle_t xTimer) {
  SparkFunBLE_ISM330DHCX* svc = (SparkFunBLE_ISM330DHCX*) pvTimerGetTimerID(xTimer);
  svc->_measure_handler();
}

void SparkFunBLE_ISM330DHCX::sensor_period_write_cb(uint16_t conn_hdl, BLECharacteristic* chr, uint8_t* data, uint16_t len) {
  (void) conn_hdl;
  SparkFunBLE_ISM330DHCX* svc = (SparkFunBLE_ISM330DHCX*) &chr->parentService();
  int32_t ms = 0;
  memcpy(&ms, data, len);
  
  svc->_update_timer(ms);
}

void SparkFunBLE_ISM330DHCX::sensor_data_cccd_cb(uint16_t conn_hdl, BLECharacteristic* chr, uint16_t value) {
  SparkFunBLE_ISM330DHCX* svc = (SparkFunBLE_ISM330DHCX*) &chr->parentService();
  
  svc->_notify_handler(conn_hdl, value);
}
