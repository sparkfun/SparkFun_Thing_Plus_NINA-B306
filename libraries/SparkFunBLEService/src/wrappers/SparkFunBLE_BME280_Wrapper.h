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

#ifndef _SPARKFUNBLE_BME280_WRAPPER_H_
#define _SPARKFUNBLE_BME280_WRAPPER_H_

#include <SparkFunBME280.h>
#include <Adafruit_Sensor.h>

enum envDataType{
    TEMPERATURE,
    PRESSURE,
    REL_H
};

/* Wraps the BME280 Sensor class in an Adafruit_Sensor class */
class SparkFunBLE_BME280_Wrapper : public Adafruit_Sensor
{
    public:
        SparkFunBLE_BME280_Wrapper(BME280 *envSensor, envDataType type, int32_t sensorID = -1);

        bool getEvent(sensors_event_t *);
        void getSensor(sensor_t *);

    protected:
        BME280 *_envSensor;
        
        envDataType _envDataType;
        BME280_SensorMeasurements *envData;
    
    private:
        int32_t _sensorID;

        const float PA_TO_HPA = 0.01;
}

#endif // _SPARKFUNBLE_BME280_WRAPPER_H_