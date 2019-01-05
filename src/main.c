#include "mgos.h"
#include "mgos_i2c.h"
#include "mgos_imu.h"
#include "madgwick.h"
#include "imupacket.h"
#include "serial.h"

#define BOARD  3
#include "boards.h"

#ifndef BOARD
  #define ACC_I2CADDR     0x6b
  #define ACC_TYPE        ACC_LSM9DS1
  #define GYRO_I2CADDR    0x6b
  #define GYRO_TYPE       GYRO_LSM9DS1
  #define GYRO_ORIENT     { 1, 0, 0, 0, -1, 0, 0, 0, 1 }
  #define MAG_I2CADDR     0x1e
  #define MAG_TYPE        MAG_LSM9DS1
  #define MAG_ORIENT      { -1, 0, 0, 0, -1, 0, 0, 0, 1 }
#endif

bool s_calibrating = false;
struct mgos_imu_madgwick *s_filter = NULL;
static uint16_t s_imu_period        = 10;     // msec
static uint16_t s_imu_info_period   = 5000;
static uint16_t s_imu_quat_period   = 50;
static uint16_t s_imu_angles_period = 100;

static void i2c_scanner(struct mgos_i2c *i2c);
static void i2c_scanner(struct mgos_i2c *i2c) {
  int i;

  if (!i2c) {
    LOG(LL_ERROR, ("No global I2C bus configured"));
    return;
  }

  LOG(LL_INFO, ("Scanning I2C bus for devices"));
  for (i = 0; i < 0x78; i++) {
    if (mgos_i2c_write(i2c, i, NULL, 0, true)) {
      LOG(LL_INFO, ("Found device at I2C address 0x%02x", i));
    }
  }
  return;
}


static void imu_calibrate(struct mgos_imu *imu, struct imu_packet_data *p) {
  static struct imu_packet_offset o   = { 0, 0, 0, 0, 0, 0, 0 };
  static uint32_t calibration_samples = 300;

  if (!imu || !p) {
    return;
  }

  o.ax += p->ax;
  o.ay += p->ay;
  o.az += p->az;
  o.gx += p->gx;
  o.gy += p->gy;
  o.gz += p->gz;
  o.samples++;

  if (o.samples >= calibration_samples) {
    float ax, ay, az, gx, gy, gz;
    // One of the accel axes will be pulling G
    o.ax /= (float)o.samples;
    o.ay /= (float)o.samples;
    o.az /= (float)o.samples;
    if (fabs(o.ax) > 9.0f) {
      o.ax = 0;
    }
    if (fabs(o.ay) > 9.0f) {
      o.ay = 0;
    }
    if (fabs(o.az) > 9.0f) {
      o.az = 0;
    }

    o.gx /= (float)o.samples;
    o.gy /= (float)o.samples;
    o.gz /= (float)o.samples;

    imupacket_emit_offset(&o);
    mgos_imu_gyroscope_get_offset(imu, &gx, &gy, &gz);
    mgos_imu_accelerometer_get_offset(imu, &ax, &ay, &az);

    gx -= o.gx;
    gy -= o.gy;
    gz -= o.gz;
    ax -= o.ax;
    ay -= o.ay;
    az -= o.az;
    mgos_imu_gyroscope_set_offset(imu, gx, gy, gz);
    mgos_imu_accelerometer_set_offset(imu, ax, ay, az);

    // Clear up counter for subsequent calls to imu_calibrate()
    memset(&o, 0, sizeof(struct imu_packet_offset));
    calibration_samples = 300;
    s_calibrating       = false;
  }
}

static void imu_cb(void *user_data) {
  struct mgos_imu *      imu = (struct mgos_imu *)user_data;
  struct imu_packet_data p;

  if (!imu) {
    return;
  }

  mgos_imu_accelerometer_get(imu, &p.ax, &p.ay, &p.az);
  mgos_imu_gyroscope_get(imu, &p.gx, &p.gy, &p.gz);
  mgos_imu_magnetometer_get(imu, &p.mx, &p.my, &p.mz);

  if (s_calibrating) {
    imu_calibrate(imu, &p);
  }

  mgos_imu_madgwick_update(s_filter,
                           p.gx, p.gy, p.gz,
                           p.ax, p.ay, p.az,
                           0, 0, 0);

  mgos_imu_madgwick_get_counter(s_filter, &p.filter_counter);
  imupacket_emit_data(&p);
}

enum mgos_app_init_result mgos_app_init(void) {
  struct mgos_i2c *i2c = mgos_i2c_get_global();
  struct mgos_imu *imu = mgos_imu_create();

  if (!i2c) {
    LOG(LL_ERROR, ("I2C bus missing, set i2c.enable=true in mos.yml"));
    return false;
  }

  i2c_scanner(i2c);

  if (!imu) {
    LOG(LL_ERROR, ("Cannot create IMU"));
    return false;
  }

  if (ACC_I2CADDR>0 && !mgos_imu_accelerometer_create_i2c(imu, i2c, ACC_I2CADDR, ACC_TYPE)) {
    LOG(LL_ERROR, ("Cannot create accelerometer on IMU"));
  }
  if (GYRO_I2CADDR>0 && !mgos_imu_gyroscope_create_i2c(imu, i2c, GYRO_I2CADDR, GYRO_TYPE)) {
    LOG(LL_ERROR, ("Cannot create gyroscope on IMU"));
  } else {
    float orient[9] = GYRO_ORIENT;
    mgos_imu_gyroscope_set_orientation(imu, orient);
  }
  if (MAG_I2CADDR>0 && !mgos_imu_magnetometer_create_i2c(imu, i2c, MAG_I2CADDR, MAG_TYPE)) {
    LOG(LL_ERROR, ("Cannot create magnetometer on IMU"));
  } else {
    float orient[9] = MAG_ORIENT;
    mgos_imu_magnetometer_set_orientation(imu, orient);
  }

  if (!(s_filter = mgos_imu_madgwick_create())) {
    LOG(LL_ERROR, ("Cannot create magnetometer on IMU"));
  }

  serial_init();

  // Install timers
  mgos_set_timer(s_imu_period, true, imu_cb, imu);
  mgos_set_timer(s_imu_info_period, true, imupacket_emit_info, imu);
  mgos_set_timer(s_imu_quat_period, true, imupacket_emit_quat, s_filter);
  mgos_set_timer(s_imu_angles_period, true, imupacket_emit_angles, s_filter);
  return MGOS_APP_INIT_SUCCESS;
}
