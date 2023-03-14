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

#ifndef _SPARKFUNBLE_ISM330DHCX_H_
#define _SPARKFUNBLE_ISM330DHCX_H_

#include "SparkFunBLEService.h"
#include "wrappers/SparkFunBLE_ISM330DHCX_Wrapper.h"

class SparkFun_ISM330DHCX;

class SparkFunBLE_ISM330DHCX : public SparkFunBLE_Sensor
{
  public:
    static const uint8_t UUID128_SERVICE[16];
    static const uint8_t UUID128_CHR_DATA[16];

    SparkFunBLE_ISM330DHCX(void);
    err_t begin(Adafruit_USBD_CDC* Serial, SparkFun_ISM330DHCX* sensor, uint16_t sensorID = -2);
    
  protected:
    void _measure_handler(void);

  private:
    SparkFun_ISM330DHCX* _imuSensor;

    SparkFunBLE_ISM330DHCX_Wrapper* _accel;
    SparkFunBLE_ISM330DHCX_Wrapper* _gyro;
    Adafruit_USBD_CDC* _Serial;
};

#endif // _SPARKFUNBLE_ISM330DHCX_H_