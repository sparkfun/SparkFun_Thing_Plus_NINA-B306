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

#include "SparkFunBLE_BME280_Wrapper.h"

SparkFunBLE_BME280_Wrapper::SparkFunBLE_BME280_Wrapper(BME280* envSensor, envDataType type, int32_t sensorID)
{
    _envSensor = envSensor;
    _envDataType = type;
    _sensorID = sensorID;
}

bool SparkFunBLE_BME280_Wrapper::getEvent(sensors_event_t *event)
{
    /* Clear event */
    memset(event, 0, sizeof(sensors_event_t));

    event->version = sizeof(sensors_event_t);
    event->sensor_id = _sensorID;
    event->timestamp = 0;

    switch(_envDataType) {
        case TEMPERATURE:
            event->type = SENSOR_TYPE_AMBIENT_TEMPERATURE;
            event->temperature = _envSensor->readTempC();
            break;
        case PRESSURE:
            event->type = SENSOR_TYPE_PRESSURE;
            event->pressure = _envSensor->readFloatPressure() * PA_TO_HPA;
            break;
        case REL_H:
            event->type = SENSOR_TYPE_RELATIVE_HUMIDITY;
            event->relative_humidity = _envSensor->readFloatHumidity();
            break;
        default:
            break;
    }

    return true;
}

void SparkFunBLE_BME280_Wrapper::getSensor(sensor_t *sensor)
{
    /* Clear the sensor_t object */
    memset(sensor, 0, sizeof(sensor_t));

    /* Insert the sensor name in the fixed length char array */
    strncpy(sensor->name, "BME280", sizeof(sensor->name) - 1);
    sensor->name[sizeof(sensor->name) - 1] = 0;
    sensor->version = 1;
    sensor->sensor_id = _sensorID;

    switch(_envDataType) {
        case TEMPERATURE:
            sensor->type = SENSOR_TYPE_AMBIENT_TEMPERATURE;
            sensor->max_value = 65;
            sensor->min_value = 0;
            sensor->resolution = 1;
            break;
        case PRESSURE:
            sensor->type = SENSOR_TYPE_PRESSURE;
            sensor->max_value = 1100;
            sensor->min_value = 300;
            sensor->resolution = 18;
            break;
        case REL_H:
            sensor->type = SENSOR_TYPE_RELATIVE_HUMIDITY;
            sensor->max_value = 100.0;
            sensor->min_value = 0.0;
            sensor->resolution = 0.008;
            break;
        default:
            break;
    }
    sensor->min_delay = 12050;

}