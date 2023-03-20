# SparkFun BLE Service for the Thing Plus NINA-B306

## Libraries required

| Name                               | Version  | Available in Arduino? | Link                                                                                            |
| ---------------------------------- | -------- | --------------------- | ----------------------------------------------------------------------------------------------- |
| SparkFun 6DoF ISM330DHCX           | > 1.0.6  | ✓                     | [Link](https://github.com/sparkfun/SparkFun_6DoF_ISM330DHCX_Arduino_Library)                    |
| SparkFun BME280                    | > 2.0.9  | ✓                     | [Link](https://github.com/sparkfun/SparkFun_BME280_Arduino_Library)                             |
| SparkFun MAX1704x Fuel Gauge       | > 1.0.4  | ✓                     | [Link](https://github.com/sparkfun/SparkFun_MAX1704x_Fuel_Gauge_Arduino_Library)                |
| Adafruit Bluefruit nRF52 Libraries | > 0.21.0 | ✗ (in board repo)     | [Link](https://github.com/adafruit/Adafruit_nRF52_Arduino/tree/master/libraries/Bluefruit52Lib) |
| Adafruit TinyUSB Library           | > 1.7.0  | ✓                     | [Link](https://github.com/adafruit/Adafruit_TinyUSB_Arduino)                                    |
| Adafruit Unified Sensor            | > 1.1.9  | ✗                     | [Link](https://github.com/adafruit/Adafruit_Sensor)                                             |

## Description

Arduino Library to support BLE sensor services on the Thing Plus NINA-B306. Based on Adafruit Bluefruit nRF52 Libraries, Adafruit BLE Services, and Adafruit Sensor.

## Services

All SparkFun Service/Characteristic UUID128 share the same Base UUID:

`5FE0xxxx-005E-4761-9A7E-947AA3C505FE`

| UUID                                 | Value                                   | ID   | Unit       | Permissions   | Description                                                                 |
| ------------------------------------ | --------------------------------------- | ---- | ---------- | ------------- | --------------------------------------------------------------------------- |
| 5FE00001-005E-4761-9A7E-947AA3C505FE | Measurement Period                      | 0001 | `int32_t`  | Read + Write  | ms between measurements, -1: stop reading, 0: update when changes           |
| 5FE00100-005E-4761-9A7E-947AA3C505FE | BME280 Sensor Service                   | 0100 |            | Read          | Sensor output from the BME280.                                              |
| 5FE00101-005E-4761-9A7E-947AA3C505FE | BME280 Sensor Output Characteristic     | 0101 | `float[3]` | Read + Notify | Temperature in `degC`, Pressure in `hPa`, Humidity in `% Relative Humidity` |
| 5FE00200-005E-4761-9A7E-947AA3C505FE | ISM330DHCX Sensor Service               | 0200 |            | Read          | Sensor output from the ISM330DHCX.                                          |
| 5FE00201-005E-4761-9A7E-947AA3C505FE | ISM330DHCX Sensor Output Characteristic | 0201 | `float[6]` | Read + Notify | 3-axis Acceleration in `m/s<sup>2</sup>`, 3-axis Rotational Rate in `rad/s` |
