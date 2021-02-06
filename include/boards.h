#include "mgos.h"
#include "mgos_imu.h"

#if BOARD == 1
// LSM303D_L3GD20_COMBO -- sold as GY-89
  #define ACC_I2CADDR     0x1d
  #define ACC_TYPE        ACC_LSM303D
  #define GYRO_I2CADDR    0x6b
  #define GYRO_TYPE       GYRO_L3GD20
  #define GYRO_ORIENT     { 0, -1, 0, 1, 0, 0, 0, 0, 1 }
  #define MAG_I2CADDR     0x1d
  #define MAG_TYPE        MAG_LSM303D
  #define MAG_ORIENT      { 1, 0, 0, 0, 1, 0, 0, 0, 1 }
#elif BOARD == 2
// ITG3205_ADXL345_HMC5883L_COMBO -- sold as HW-579
  #define ACC_I2CADDR     0x53
  #define ACC_TYPE        ACC_ADXL345
  #define GYRO_I2CADDR    0x68
  #define GYRO_TYPE       GYRO_ITG3205
  #define MAG_I2CADDR     0x1d
  #define MAG_TYPE        MAG_HMC5883L
#elif BOARD == 3
// Sold as M5 Stack (ESP32 + MPU9250)
  #define ACC_I2CADDR     0x68
  #define ACC_TYPE        ACC_MPU9250
  #define GYRO_I2CADDR    0x68
  #define GYRO_TYPE       GYRO_MPU9250
  #define GYRO_ORIENT     { 1, 0, 0, 0, 1, 0, 0, 0, 1 }
  #define MAG_I2CADDR     0x0c
  #define MAG_TYPE        MAG_AK8963
  #define MAG_ORIENT      { 0, 1, 0, 1, 0, 0, 0, 0, -1 }
#elif BOARD == 4
  #define ACC_I2CADDR     0x68
  #define ACC_TYPE        ACC_LSM9DS1
  #define GYRO_I2CADDR    0x68
  #define GYRO_TYPE       GYRO_LSM9DS1
  #define GYRO_ORIENT     { 1, 0, 0, 0, 1, 0, 0, 0, 1 }
  #define MAG_I2CADDR     0x0c
  #define MAG_TYPE        MAG_LSM9DS1
  #define MAG_ORIENT      { 1, 0, 0, 0, 1, 0, 0, 0, 1 }
#endif
