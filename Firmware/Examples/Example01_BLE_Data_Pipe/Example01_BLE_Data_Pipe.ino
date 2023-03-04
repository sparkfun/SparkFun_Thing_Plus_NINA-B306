/*
  BLE Data Pipe
  
  This example creates a BLE peripheral with a sensor service using the 
  SparkFun NINA-B306 Thing Plus. The sensor service notifies (push data to) the
  central.
  
  The sensor service contains the following characteristics:
  Acceleration X,Y,Z
  Gyro X,Y,Z
  Temperature (C), Humidity, Pressure

  MIT License

  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files (the "Software"), to deal
  in the Software without restriction, including without limitation the rights
  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
  copies of the Software, and to permit persons to whom the Software is
  furnished to do so, subject to the following conditions:

  The above copyright notice and this permission notice shall be included in all
  copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
  SOFTWARE.
  
*/

#include <Wire.h>
#include <Adafruit_TinyUSB.h>
#include <SparkFunBME280.h>
#include <SparkFun_ISM330DHCX.h>
#include <SparkFun_MAX1704x_Fuel_Gauge_Arduino_Library.h>

#include "bluefruit.h"

/* Battery Monitor */
SFE_MAX1704X lipo(MAX1704X_MAX17048);

/* BME280 Environmental Sensor */
BME280 envSensor;
BME280_SensorMeasurements envData;

/* ISM330DHCX 6-DoF IMU Sensor */
SparkFun_ISM330DHCX imuSensor;

/* BLE Data Pipe Service Definition
 * 
 * 
 */
BLEService        imus = BLEService();
BLECharacteristic acxc = BLECharacteristic();
BLECharacteristic acyc = BLECharacteristic();
BLECharacteristic aczc = BLECharacteristic();
BLECharacteristic gyxc = BLECharacteristic();
BLECharacteristic gyyc = BLECharacteristic();
BLECharacteristic gyzc = BLECharacteristic();

BLEService        envs = BLEService();
BLECharacteristic tmpc = BLECharacteristic();
BLECharacteristic prsc = BLECharacteristic();
BLECharacteristic humc = BLECharacteristic();

/* BLE Device Information
 * Name - Thing Plus NINA-B306 Data Pipe
 * Manufacturer - SparkFun Electronics
 * Model - SparkFun Thing Plus NINA-B306
 */
BLEDis bledis;

/* BLE Battery Service helper class */
BLEBas blebas;

sfe_ism_data_t accelData;
sfe_ism_data_t gyroData;

long previousMillis = 0;

void setup() {
  Serial.begin(115200);
  while(!Serial) delay(10);

  Serial.println(F("SparkFun BLE Data Pipe Example"));
  Serial.println(F("------------------------------\n"));

  Wire.begin();

  // Initialize Sensors
  initFuelGauge();

  initBME280();

  initISM330DHCX();

  //Initialize BLE things
  Bluefruit.begin();
  Bluefruit.setName("Data Pipe Example");

  // Configure and start Device Information Service
  Serial.println(F("Configuring the Device Information Service..."));
  bledis.setManufacturer("SparkFun Electronics");
  bledis.setModel("Thing Plus NINA-B306");
  bledis.setHardwareRev("v10");
  bledis.setSoftwareRev("0.1.0");
  bledis.begin();

  // Configure and start BLE Battery Service and initialize to 100%
  Serial.println(F("Configuring the Battery Service..."));
  blebas.begin();
  blebas.write(100);

  // Configure and start BLE Environmental Sensor Service
  Serial.println(F("Configuring the Environmental Sensor Service..."));


  // Configure and start the IMU Sensor Service
  Serial.println(F("Configuring the IMU Data Service..."));
  

  Serial.println(F("Setup complete."));
  
  /*  Start advertising BLE. It will start continuously transmitting BLE
      advertising packets and will be visible to remote BLE central devices
      until it receives a connection.  
  */
  startAdv();

  Serial.println(F("BLE device active, waiting for connections..."));

}

void loop() {
  
  // // String(accelData.xData)+","+String(accelData.yData)+","+String(accelData.zData)
  // // Wait for BLE central
  // BLEDevice central = BLE.central();

  // if (central) {
  //   Serial.print("Connected to central: ");

  //   Serial.println(central.address());
  //   digitalWrite(LED_BUILTIN, HIGH);

  //   while (central.connected()) {
  //     long currentMillis = millis();

  //     if (currentMillis - previousMillis >=200) {
  //       previousMillis = currentMillis;
  //       readSensors();
  //     }
  //   }

  //   digitalWrite(LED_BUILTIN, LOW);
  //   Serial.print("Disconnected from central: ");
  //   Serial.println(central.address());    
  // }
}

void startAdv() {
  Bluefruit.Advertising.addFlags(BLE_GAP_ADV_FLAGS_LE_ONLY_GENERAL_DISC_MODE);
  Bluefruit.Advertising.addTxPower();

  // Add services
  //Bluefruit.Advertising.addService();

  Bluefruit.ScanResponse.addName();

  /* Start Advertising
   * - Enable auto advertising if disconnected
   * - Interval: fast mode = 20 ms, slow mode = 152.5 ms
   * - Timeout for fast mode is 30 seconds
   * - Start(timeout) with timeout = 0 will advertise forever (until connected)
   * 
   * For recommended advertising interval
   * https://developer.apple.com/library/content/qa/qa1931/_index.html
   */
  Bluefruit.Advertising.restartOnDisconnect(true);
  Bluefruit.Advertising.setInterval(32, 244);       // units of 0.625 ms
  Bluefruit.Advertising.setFastTimeout(30);         // number of seconds in fast mode
  Bluefruit.Advertising.start(0);                   // 0 = Don't stop advertising unless connected

}

void initFuelGauge() {
  Serial.println(F("Initializing MAX17048 Battery Fuel Gauge..."));

  if (!lipo.begin()) {
    Serial.println(F("MAX17048 not detected. Please check wiring. Freezing."));
    while(1);
  }

  // Read and print the battery threshold
  Serial.print(F("Battery empty threshold is currently: "));
  Serial.print(lipo.getThreshold());
  Serial.println(F("%"));

	// We can set an interrupt to alert when the battery SoC gets too low.
	// We can alert at anywhere between 1% and 32%:
	lipo.setThreshold(20); // Set alert threshold to 20%.

  // Read and print the battery empty threshold
  Serial.print(F("Battery empty threshold is now: "));
  Serial.print(lipo.getThreshold());
  Serial.println(F("%"));
  
  // Clear any alert that might already be generated.
  lipo.clearAlert();

}

void initBME280() {
  Serial.println("Initializing BME280 Environmental Sensor...");
  envSensor.setI2CAddress(0x76); // Default address on NINA-B306

  if (!envSensor.beginI2C()) {
    Serial.println("The BME280 did not respond. Please check address and wiring.");
    while(1);
  }
  Serial.println("BME280 initialization successful.");
}

void initISM330DHCX() {
  Serial.println("Initializing ISM330DHCX...");

  if (!imuSensor.begin()) {
    Serial.println("The ISM330DHCX did not respond. Please check address and wiring.");
    while(1);
  }

  Serial.println("ISM330DHCX Initialization successful. Resetting device settings...");

  imuSensor.deviceReset();

  while(!imuSensor.getDeviceReset()){
    delay(1);
  }
    
  Serial.println("Device reset complete. Configuring ISM330DHCX...");
  delay(100);

  imuSensor.setDeviceConfig();
  imuSensor.setBlockDataUpdate();

  // Set accelerometer output data rate and data precision
  imuSensor.setAccelDataRate(ISM_XL_ODR_104Hz);
  imuSensor.setAccelFullScale(ISM_4g);

  // Set gyro output data rate and data precision
  imuSensor.setGyroDataRate(ISM_GY_ODR_104Hz);
  imuSensor.setGyroFullScale(ISM_500dps);

  // Turn on accelerometer's filter and apply settings.
  imuSensor.setAccelFilterLP2();
  imuSensor.setAccelSlopeFilter(ISM_LP_ODR_DIV_100);

  // Turn on gyro's filter and apply settings.
  imuSensor.setGyroFilterLP1();
  imuSensor.setGyroLP1Bandwidth(ISM_MEDIUM);

  Serial.println("ISM330DHCX successfully configured.");
}


// void readSensors() {
//   // if (imuSensor.checkStatus()) {
//   //   imuSensor.getAccel(&accelData);
//   //   imuSensor.getGyro(&gyroData);
//   //   accelXChar.writeValue(accelData.xData);
//   //   accelYChar.writeValue(accelData.yData);
//   //   accelZChar.writeValue(accelData.zData);
//   //   gyroXChar.writeValue(0.0175*gyroData.xData);
//   //   gyroYChar.writeValue(0.0175*gyroData.yData);
//   //   gyroZChar.writeValue(0.0175*gyroData.zData);
//   // }
//   // while (envSensor.isMeasuring()){};
//   // envSensor.readAllMeasurements(&envData);
//   // tempChar.writeValue(envData.temperature);
//   // humidChar.writeValue(envData.humidity);
//   // pressChar.writeValue(envData.pressure);

// }