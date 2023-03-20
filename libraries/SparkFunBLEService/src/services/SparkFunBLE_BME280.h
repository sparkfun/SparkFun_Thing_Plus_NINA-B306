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

#ifndef _SPARKFUNBLE_BME280_H_
#define _SPARKFUNBLE_BME280_H_

#include "SparkFunBLEService.h"
#include "wrappers/SparkFunBLE_BME280_Wrapper.h"

// Forward declarations
class BME280;

class SparkFunBLE_BME280 : public SparkFunBLE_Sensor
{
  public:
    static const uint8_t UUID128_SERVICE[16];
    static const uint8_t UUID128_CHR_DATA[16];

    SparkFunBLE_BME280(void);
    err_t begin (BME280* sensor, uint16_t sensorID = -3, int ms = DEFAULT_PERIOD);

    void setPeriod(int period_ms);

  protected:
    err_t _begin(uint32_t ms);

    BME280* _envSensorPtr;

    SparkFunBLE_BME280_Wrapper* _temp;
    SparkFunBLE_BME280_Wrapper* _press;
    SparkFunBLE_BME280_Wrapper* _humidity;

    measure_callback_t _measure_cb;
    notify_callback_t _notify_cb;

    SoftwareTimer _timer;

    void _update_timer(int32_t ms);
    void _measure_handler(void);
    void _notify_handler(uint16_t conn_hdl, uint16_t value);

    static void sensor_timer_cb(TimerHandle_t xTimer);
    static void sensor_period_write_cb(uint16_t conn_hdl, BLECharacteristic* chr, uint8_t* data, uint16_t len);
    static void sensor_data_cccd_cb(uint16_t conn_hdl, BLECharacteristic* chr, uint16_t value);
};

#endif // _SPARKFUNBLE_BME280_H_