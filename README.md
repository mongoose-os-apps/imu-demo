# Mongoose IMU Demo

This is a set of apps that demonstrate the [Mongoose IMU](https://github.com/mongoose-os-libs/imu)
library. This IMU library supports lots of gyroscope, accelerometer and
magnetometer sensors commonly found on the market today.

## Demo Apps

### Mongoose OS

The primary component is a Mongoose OS application (currently for ESP32) that
interfaces the IMU library and emits binary packets over serial console (which
is why `mos console` won't do much but print out garbage). The sensors are
initialized and a Madgwick AHRS (attitude and heading reference system) is
started with an update frequency of 100Hz. The AHRS serves to fuse the data
from individual sensors:

*   Accelerometer: measures the acceleration of the device in three directions,
typically `X`, `Y`, and `Z` axes that are perpendicular to one another. One of
these, typically `Z`, is being pulled towards earth and therefore measures 
grativy. Most accelerometers will have a range of between 2G and 16G.
*   Gyroscope: measures the angular rate, also in three axes. Most gyro's will
have a range of between 250deg/sec and 2000deg/sec.
*   Magnetometers: measures the earth's magnetic field through the device,
also in three directions.

These sensors yield 9 values, which can be combined by an AHRS into a stable
representation of the device's position in 3D space.

### Console application

There's a simple console application based on `ncurses` which reads the raw
data in addition to the `Quaternion` that is computed by the microcontroller,
and derives the `Roll`, `Pitch` and `Yaw` just like an airplane would.

![Console Demo](img/console-client.png)

By default, the console application assumes the microcontroller is on /dev/ttyUSB1. To read
from a different serial interface, provide this on the command line, for example:

```
client /dev/ttyUSB0
```

### Chrome application

This more sophisticated Chrome Extension will draw the device in your browser
including graphs of the sensor data and other goodies. It's used by the
author of the IMU library mostly to validate correctness of new drivers.

[![Chrome Demo](img/chrome-client.png)](https://youtu.be/G2P0DbbmTMo)

To install the Chrome Extension:


1. In the browser, navigate to chrome://extensions
2. Turn on Developer Mode
3. Click 'Load unpacked'. 
4. Navigate to (project home)/chrome and click open. The Mongoose OS :: IMU Visualization extension now appears under Chrome Apps in the list of extensions.

To launch the extension:
1. In the browser, navigate to chrome://apps
2. Click on the Mongoose OS :: IMU Visualization app and it should launch.


If you are using Microsoft Edge Chromium, the URLs are edge://extensions and edge://apps


### Debugging

First, check that your getting output from the mongoose app. To do this, restart the device and read the console 
```
mos console 
```
If the output is not changing, it's likely that you're IMU connection is not configured correctly.


In mos.yml remove these lines:
```
  - ["debug.stdout_uart", 0]
  - ["debug.stderr_uart", 0]
```

You should see the following in the output in the console after restarting:
```
[Feb  6 17:51:13.892] main.c:41               Found device at I2C address 0x0c
[Feb  6 17:51:14.004] main.c:41               Found device at I2C address 0x3c
[Feb  6 17:51:14.100] main.c:41               Found device at I2C address 0x68
[Feb  6 17:51:14.132] mgos_imu_mpu925x.c:31   Detected MPU9250 at I2C 0x68
[Feb  6 17:51:14.228] mgos_imu_mpu925x.c:31   Detected MPU9250 at I2C 0x68
```

If you get this in the output then your configuration of the IMU device is incorrect.
```
Feb  6 17:36:32.629] main.c:38               Scanning I2C bus for devices
[Feb  6 17:36:32.892] mgos_imu_accelerome:227 Could not detect accelerometer type 1 (MPU9250) at I2C 0x68
[Feb  6 17:36:32.897] main.c:154              Cannot create accelerometer on IMU
[Feb  6 17:36:32.906] mgos_imu_gyroscope.:226 Could not detect gyroscope type 1 (MPU9250) at I2C 0x68
[Feb  6 17:36:32.911] main.c:161              Cannot create gyroscope on IMU
[Feb  6 17:36:32.920] mgos_imu_magnetomet:208 Could not detect magnetometer type 1 (AK8963) at I2C 0x0c
[Feb  6 17:36:32.926] main.c:171              Cannot create magnetometer on IMU
```

Things to check:

Check that you have the correct board type defined in src/main.c. The boards are defined in include/boards.h

Check SDA and SCL. If you are using a board with the IMU chipset built in, refer to the schematic. If necessary, update these lines in config.schema

```
- ["i2c.sda_gpio", 19]
- ["i2c.scl_gpio", 18]
```


# Disclaimer

This project is not an official Google project. It is not supported by Google
and Google specifically disclaims all warranties as to its quality,
merchantability, or fitness for a particular purpose.

