#include "mgos.h"
#include "mgos_i2c.h"
#include "mgos_imu.h"
#include "madgwick.h"
#include "imupacket.h"

#define IMU_HZ                    100
#define IMU_PACKET_INFO_PERIOD    5000     // msec

struct mgos_imu_madgwick *s_filter = NULL;

static void emit_imu_packet_data(struct imu_packet *p);
static void emit_imu_packet_info(void *user_data);
static void emit_imu_packet_log(const char *msg);

static void emit_imu_packet_data(struct imu_packet *p) {
  uint8_t packet[4 + 1 + sizeof(struct imu_packet)]; // HEADER + DATALEN + DATA

  packet[0] = '$';
  packet[1] = 'P';
  packet[2] = '>';
  packet[3] = 'A';
  packet[4] = sizeof(struct imu_packet);
  memcpy(packet + 5, p, packet[4]);
  mgos_uart_write(0, packet, sizeof(packet));
  return;
}

static void emit_imu_packet_info(void *user_data) {
  struct mgos_imu *imu = (struct mgos_imu *)user_data;
  const char *     a, *g, *m;
  uint8_t          len;

  a = mgos_imu_accelerometer_get_name(imu);
  g = mgos_imu_gyroscope_get_name(imu);
  m = mgos_imu_magnetometer_get_name(imu);

  len = strlen(a) + strlen(g) + strlen(m) + 2;
  mgos_uart_write(0, "$P>B", 4);
  mgos_uart_write(0, &len, 1);
  mgos_uart_write(0, a, strlen(a));
  mgos_uart_write(0, ",", 1);
  mgos_uart_write(0, g, strlen(g));
  mgos_uart_write(0, ",", 1);
  mgos_uart_write(0, m, strlen(m));

  emit_imu_packet_log("Hello world");
  return;
}

static void emit_imu_packet_log(const char *msg) {
  uint8_t len;

  len = strlen(msg);
  mgos_uart_write(0, "$P>C", 4);
  mgos_uart_write(0, &len, 1);
  mgos_uart_write(0, msg, strlen(msg));
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
