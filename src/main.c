#include "mgos.h"
#include "mgos_i2c.h"
#include "mgos_imu.h"
#include "madgwick.h"

#define IMU_HZ                    100
#define IMU_PACKET_INFO_PERIOD    5000     // msec

struct mgos_imu_madgwick *s_filter = NULL;

struct imu_packet {
  // IMU Data
  float    ax, ay, az;
  float    gx, gy, gz;
  float    mx, my, mz;

  // Quaternion
  float    q0, q1, q2, q3;

  // Roll/Pitch/Yaw in Rads
  float    roll, pitch, yaw;

  // Filter counter
  uint32_t filter_counter;
};

static void emit_imu_packet_info(void *user_data) {
  struct mgos_imu *imu = (struct mgos_imu *)user_data;
  const char *     a, *g, *m;
  uint8_t          len;

  a = mgos_imu_accelerometer_get_name(imu);
  g = mgos_imu_gyroscope_get_name(imu);
  m = mgos_imu_magnetometer_get_name(imu);

  len = strlen(a) + strlen(g) + strlen(m) + 2;
  write(1, "$P>B", 4);
  write(1, &len, 1);
  write(1, a, strlen(a));
  write(1, ",", 1);
  write(1, g, strlen(g));
  write(1, ",", 1);
  write(1, m, strlen(m));
  return;
}

static void emit_imu_packet_data(struct imu_packet *p) {
  uint8_t len;

  len = sizeof(struct imu_packet);
  write(1, "$P>A", 4);
  write(1, &len, 1);
  write(1, p, sizeof(struct imu_packet));
  return;
}

static void imu_cb(void *user_data) {
  struct mgos_imu * imu = (struct mgos_imu *)user_data;
  struct imu_packet imu_packet;

  if (!imu) {
    return;
  }

  mgos_imu_accelerometer_get(imu, &imu_packet.ax, &imu_packet.ay, &imu_packet.az);
  mgos_imu_gyroscope_get(imu, &imu_packet.gx, &imu_packet.gy, &imu_packet.gz);
  mgos_imu_magnetometer_get(imu, &imu_packet.mx, &imu_packet.my, &imu_packet.mz);

  mgos_imu_madgwick_update(s_filter,
                           imu_packet.gx, imu_packet.gy, imu_packet.gz,
                           imu_packet.ax, imu_packet.ay, imu_packet.az,
                           0, 0, 0);

  mgos_imu_madgwick_get_quaternion(s_filter, &imu_packet.q0, &imu_packet.q1, &imu_packet.q2, &imu_packet.q3);
  mgos_imu_madgwick_get_angles(s_filter, &imu_packet.roll, &imu_packet.pitch, &imu_packet.yaw);
  mgos_imu_madgwick_get_counter(s_filter, &imu_packet.filter_counter);

  emit_imu_packet_data(&imu_packet);
}

enum mgos_app_init_result mgos_app_init(void) {
  struct mgos_i2c *i2c = mgos_i2c_get_global();
  struct mgos_imu *imu = mgos_imu_create();

  if (!i2c) {
    LOG(LL_ERROR, ("I2C bus missing, set i2c.enable=true in mos.yml"));
    return false;
  }

  if (!imu) {
    LOG(LL_ERROR, ("Cannot create IMU"));
    return false;
  }

  if (!mgos_imu_accelerometer_create_i2c(imu, i2c, 0x68, ACC_MPU9250)) {
    LOG(LL_ERROR, ("Cannot create accelerometer on IMU"));
  }
  if (!mgos_imu_gyroscope_create_i2c(imu, i2c, 0x68, GYRO_MPU9250)) {
    LOG(LL_ERROR, ("Cannot create gyroscope on IMU"));
  }
  if (!mgos_imu_magnetometer_create_i2c(imu, i2c, 0x0C, MAG_AK8963)) {
    LOG(LL_ERROR, ("Cannot create magnetometer on IMU"));
  }

  if (!(s_filter = mgos_imu_madgwick_create())) {
    LOG(LL_ERROR, ("Cannot create magnetometer on IMU"));
  }

  // Install timers
  mgos_set_timer(1000 / IMU_HZ, true, imu_cb, imu);
  mgos_set_timer(IMU_PACKET_INFO_PERIOD, true, emit_imu_packet_info, imu);
  return MGOS_APP_INIT_SUCCESS;
}
