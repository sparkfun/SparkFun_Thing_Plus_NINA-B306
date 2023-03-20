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

const uint8_t SparkFunBLE_BME280::UUID128_SERVICE[16] = {
  0xFE, 0x05, 0xC5, 0xA3, 0x7A, 0x94, 0x7E, 0x9A,
  0x61, 0x47, 0x5E, 0x00, 0x00, 0x01, 0xE0, 0x5F
};

const uint8_t SparkFunBLE_BME280::UUID128_CHR_DATA[16] = {
  0xFE, 0x05, 0xC5, 0xA3, 0x7A, 0x94, 0x7E, 0x9A,
  0x61, 0x47, 0x5E, 0x00, 0x01, 0x01, 0xE0, 0x5F
};

// Constructor
SparkFunBLE_BME280::SparkFunBLE_BME280(void)
  : SparkFunBLE_Sensor(UUID128_SERVICE, UUID128_CHR_DATA) {
  _envSensorPtr = NULL;
  _temp = _press = _humidity = NULL;

  // Setup Measurement Characteristic
  _measurement.setProperties(CHR_PROPS_READ | CHR_PROPS_NOTIFY);
  _measurement.setPermission(SECMODE_OPEN, SECMODE_NO_ACCESS);
  _measurement.setFixedLen(4*3);
}

err_t SparkFunBLE_BME280::begin(BME280* sensor, uint16_t sensorID, int ms) {
  _envSensorPtr = sensor;
  
  static SparkFunBLE_BME280_Wrapper temp = SparkFunBLE_BME280_Wrapper(_envSensorPtr, TEMPERATURE, sensorID);
  static SparkFunBLE_BME280_Wrapper press = SparkFunBLE_BME280_Wrapper(_envSensorPtr, PRESSURE, sensorID + 1);
  static SparkFunBLE_BME280_Wrapper humidity = SparkFunBLE_BME280_Wrapper(_envSensorPtr, REL_H, sensorID + 2);
  
  _temp = &temp;
  _press = &press;
  _humidity = &humidity;

  VERIFY_STATUS( _begin(ms) );

  return ERROR_NONE;
}

err_t SparkFunBLE_BME280::_begin(uint32_t ms) {
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

void SparkFunBLE_BME280::setPeriod(int period_ms) {
  if( period_ms > DEFAULT_PERIOD ){
    _period.write32(period_ms);
    _update_timer(period_ms);
  }
}

void SparkFunBLE_BME280::_update_timer(int32_t ms) {
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

// Notifiy BLE service
void SparkFunBLE_BME280::_measure_handler(void) {
  uint16_t len = _measurement.getMaxLen();
  uint8_t env_data[len];

  if(_envSensorPtr && _measurement.isFixedLen()) {
    sensors_event_t temp_evt, press_evt, humid_evt;

    _temp->getEvent(&temp_evt);
    _press->getEvent(&press_evt);
    _humidity->getEvent(&humid_evt);

    uint16_t offset = 0;
    memcpy(env_data + offset, (const void*) &temp_evt.temperature, sizeof(float));
    offset += sizeof(float);
    memcpy(env_data + offset, (const void*) &press_evt.pressure, sizeof(float));
    offset += sizeof(float);
    memcpy(env_data + offset, (const void*) &humid_evt.relative_humidity, sizeof(float));
  }
  else if (_measure_cb) {
    len = _measure_cb((uint8_t*)env_data, sizeof(env_data));
    len = min(len, sizeof(env_data));
  }
  else {
    return;
  }

  if (!len) {
    return;
  }

  if ( 0 == _period.read32() ) {
    uint8_t prev_buf[_measurement.getMaxLen()];
    _measurement.read(prev_buf, sizeof(prev_buf));

    // skip notify if there is no changes
    if ( 0 == memcmp(prev_buf, env_data, len) ) return;
  }

  _measurement.notify(env_data, sizeof(env_data));
}

void SparkFunBLE_BME280::_notify_handler(uint16_t conn_hdl, uint16_t value) {
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

void SparkFunBLE_BME280::sensor_timer_cb(TimerHandle_t xTimer) {
  SparkFunBLE_BME280* svc = (SparkFunBLE_BME280*) pvTimerGetTimerID(xTimer);
  svc->_measure_handler();
}

void SparkFunBLE_BME280::sensor_period_write_cb(uint16_t conn_hdl, BLECharacteristic* chr, uint8_t* data, uint16_t len) {
  (void) conn_hdl;
  SparkFunBLE_BME280* svc = (SparkFunBLE_BME280*) &chr->parentService();
  int32_t ms = 0;
  memcpy(&ms, data, len);
  
  svc->_update_timer(ms);
}

void SparkFunBLE_BME280::sensor_data_cccd_cb(uint16_t conn_hdl, BLECharacteristic* chr, uint16_t value) {
  SparkFunBLE_BME280* svc = (SparkFunBLE_BME280*) &chr->parentService();
  
  svc->_notify_handler(conn_hdl, value);
}