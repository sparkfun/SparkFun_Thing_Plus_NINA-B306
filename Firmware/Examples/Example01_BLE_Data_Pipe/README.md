# Example01 BLE Data Pipe Demo arduino-cli flash command

From `$REPO/Firmware/Examples/Example01_BLE_Data_Pipe`:

## Locally downloaded Repo and Library

```bash
arduino-cli.exe compile --library /absolute/path/to/library/SparkFun_Thing_Plus_NINA-B306/libraries/SparkFunBLEService -b arduino-git:Adafruit_nRF52_Arduino:sfe_thingplus_ninab306 -e --port $COMPORT --upload Example01_BLE_Data_Pipe.ino
```

## Arduino Library

```bash
arduino-cli.exe compile -b adafruit:nrf52:sfe_thingplus_ninab306 -e --port $COMPORT --upload Example01_BLE_Data_Pipe.ino
```
