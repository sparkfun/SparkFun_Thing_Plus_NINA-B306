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

#include "SparkFunBLE_ISM330DHCX_Wrapper.h"

SparkFunBLE_ISM330DHCX_Wrapper::SparkFunBLE_ISM330DHCX_Wrapper(Adafruit_USBD_CDC* Serial, SparkFun_ISM330DHCX *imuSensor, imuDataType type, int32_t sensorID)
{
    _Serial = Serial;
    _imu = imuSensor;
    _imuDataType = type;
    _sensorID = sensorID;
    if (_imuDataType == ACCELEROMETER)
    {
        _Serial->println(F("ISM330DHCX Accelerometer Wrapper Started."));
    }
    else if (_imuDataType == GYROSCOPE)
    {
        _Serial->println(F("ISM330DHCX Gyroscope Wrapper Started."));
    }
    else
    {
        _Serial->println(F("Unknown ISM330DHCX Wrapper Started."));
    }
}

bool SparkFunBLE_ISM330DHCX_Wrapper::getEvent(sensors_event_t *event)
{
    _Serial->println(F("ISM330DHCX getEvent entry."));
    /* Clear event */
    memset(event, 0, sizeof(sensors_event_t));

    _Serial->println(F("ISM330DHCX getEvent memory clear."));

    event->version = sizeof(sensors_event_t);
    event->sensor_id = _sensorID;
    event->timestamp = 0;

    while(!_imu->checkStatus()){};
    if (_imuDataType == ACCELEROMETER)
    {
        _Serial->println(F("ISM330DHCX getEvent accelerometer data entry."));
        event->type = SENSOR_TYPE_ACCELEROMETER;
        _imu->getAccel(&_sensorData);
        event->acceleration.x = _sensorData.xData * G_TO_MS2;
        event->acceleration.y = _sensorData.yData * G_TO_MS2;
        event->acceleration.z = _sensorData.zData * G_TO_MS2;
        _Serial->println(F("ISM330DHCX getEvent accelerometer data exit."));
    }
    else
    {
        _Serial->println(F("ISM330DHCX getEvent gyro data entry."));
        event->type = SENSOR_TYPE_GYROSCOPE;
        _imu->getGyro(&_sensorData);
        event->gyro.x = _sensorData.xData * DEG_TO_RAD;
        event->gyro.y = _sensorData.yData * DEG_TO_RAD;
        event->gyro.z = _sensorData.zData * DEG_TO_RAD;
        _Serial->println(F("ISM330DHCX getEvent accelerometer data exit."));
    }
    
    _Serial->println(F("ISM330DHCX getEvent exit."));

    return true;
}

void SparkFunBLE_ISM330DHCX_Wrapper::getSensor(sensor_t *sensor)
{
    _Serial->println(F("ISM330DHCX getSensor entry."));
    /* Clear the sensor_t object */
    memset(sensor, 0, sizeof(sensor_t));
    _Serial->println(F("ISM330DHCX getSensor memory clear."));

    /* Insert the sensor name in the fixed length char array */
    strncpy(sensor->name, "ISM330DHCX", sizeof(sensor->name) - 1);
    sensor->name[sizeof(sensor->name) - 1] = 0;
    sensor->version = 1;
    sensor->sensor_id = _sensorID;

    if (_imuDataType == ACCELEROMETER)
    {
        _Serial->println(F("ISM330DHCX getSensor accelerometer info entry."));
        sensor->type = SENSOR_TYPE_ACCELEROMETER;
        sensor->max_value = 156.9064F;   /* 16g = 156.9064 m/s/s */
        sensor->min_value = -156.9064F;  /* -16g = 156.9064 m/s/s */
        sensor->resolution = 0.0041384F; /* 0.422mg = 0.004138 m/s/s */
        _Serial->println(F("ISM330DHCX getSensor accelerometer info exit."));
    }
    else
    {
        _Serial->println(F("ISM330DHCX getSensor gyro info entry."));
        sensor->type = SENSOR_TYPE_GYROSCOPE;
        sensor->max_value = 69.812F;     /* 4000 deg/s = 69.812 rad/s */
        sensor->min_value = -69.812F;    /* -4000 deg/s = 69.812 rad/s */
        sensor->resolution = 0.0024434F; /* 140 mdeg/s = 2.4434 mrad/s */
        _Serial->println(F("ISM330DHCX getSensor gyro info exit."));
    }
    sensor->min_delay = 9616;
    _Serial->println(F("ISM330DHCX getSensor exit."));
}
