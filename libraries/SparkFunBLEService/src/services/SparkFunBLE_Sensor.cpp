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
 */

const uint8_t UUID128_CHR_SFE_MEASUREMENT_PERIOD[16] = {
  0xFE, 0x05, 0xC5, 0xA3, 0x7A, 0x94, 0x7E, 0x9A,
  0x61, 0x47, 0x5E, 0x00, 0x01, 0x00, 0xE0, 0x5F
};


SparkFunBLE_Sensor::SparkFunBLE_Sensor(BLEUuid service_uuid, BLEUuid data_uuid)
  : BLEService(service_uuid), _measurement(data_uuid), _period(UUID128_CHR_SFE_MEASUREMENT_PERIOD) {
  _sensor = NULL;
  _measure_cb = NULL;
  _notify_cb = NULL;
}

err_t SparkFunBLE_Sensor::_begin(int ms) {
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

err_t SparkFunBLE_Sensor::begin(measure_callback_t fp, int ms) {
  _measure_cb = fp;
  return _begin(ms);
}

err_t SparkFunBLE_Sensor::begin(Adafruit_Sensor* sensor, int ms) {
  _sensor = sensor;
  return _begin(ms);
}

void SparkFunBLE_Sensor::setPeriod(int period_ms) {
  _period.write32(period_ms);
  _update_timer(period_ms);
}

void SparkFunBLE_Sensor::setNotifyCallback(notify_callback_t fp) {
  _notify_cb = fp;
}

//--------------------------------------------------------------------+
// Internal API
//--------------------------------------------------------------------+
void SparkFunBLE_Sensor::_notify_handler(uint16_t conn_hdl, uint16_t value) {
  // notify enabled
  if (value & BLE_GATT_HVX_NOTIFICATION) {
    _timer.start();
  } else {
    _timer.stop();
  }

  // invoke callback if any
  if (_notify_cb) _notify_cb(conn_hdl, value);

  // send initial notification if period = 0
  //  if ( 0 == svc._period.read32() )
  //  {
  //    svc._measurement.notify();
  //  }
}

void SparkFunBLE_Sensor::_update_timer(int32_t ms) {
  if ( ms < 0 ) {
    _timer.stop();
  } else if ( ms > 0) {
    _timer.setPeriod(ms);
  } else {
    // Period = 0: keeping the current interval, but report on changes only
  }
}

void SparkFunBLE_Sensor::_measure_handler(void) {
  uint16_t len = _measurement.getMaxLen();
  uint8_t buf[len];

  // Use unified sensor API if available, only with fixed length sensor
  if (_sensor && _measurement.isFixedLen()) {
    sensors_event_t event;
    _sensor->getEvent(&event);

    memcpy(buf, event.data, len);
  }
  // Else use callback
  else if (_measure_cb) {
    len = _measure_cb(buf, sizeof(buf));
    len = min(len, sizeof(buf));
  }
  else {
    return; // nothing to measure
  }

  // no data to notify
  if (!len) return;

  // Period = 0, compare with old data, only update on changes
  if ( 0 == _period.read32() ) {
    uint8_t prev_buf[_measurement.getMaxLen()];
    _measurement.read(prev_buf, sizeof(prev_buf));

    // skip notify if there is no changes
    if ( 0 == memcmp(prev_buf, buf, len) ) return;
  }

  // TODO multiple connections
  _measurement.notify(buf, len);
}

//--------------------------------------------------------------------+
// Static Callbacks
//--------------------------------------------------------------------+

void SparkFunBLE_Sensor::sensor_timer_cb(TimerHandle_t xTimer) {
  SparkFunBLE_Sensor* svc = (SparkFunBLE_Sensor*) pvTimerGetTimerID(xTimer);
  svc->_measure_handler();
}

// Client update period, adjust timer accordingly
void SparkFunBLE_Sensor::sensor_period_write_cb(uint16_t conn_hdl, BLECharacteristic* chr, uint8_t* data, uint16_t len) {
  (void) conn_hdl;
  SparkFunBLE_Sensor* svc = (SparkFunBLE_Sensor*) &chr->parentService();

  int32_t ms = 0;
  memcpy(&ms, data, len);

  svc->_update_timer(ms);
}

void SparkFunBLE_Sensor::sensor_data_cccd_cb(uint16_t conn_hdl, BLECharacteristic* chr, uint16_t value) {
  SparkFunBLE_Sensor* svc = (SparkFunBLE_Sensor*) &chr->parentService();

  svc->_notify_handler(conn_hdl, value);
}