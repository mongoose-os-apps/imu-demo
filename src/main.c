#include "mgos.h"
#include "mgos_i2c.h"
#include "mgos_imu.h"

#define IMU_HZ           100
#define IMU_PACKET_STATS_PERIOD 1.00 // Seconds
#define IMU_PACKET_INFO_PERIOD  5.00 // Seconds
#define IMU_PACKET_ID_INFO  1
#define IMU_PACKET_ID_STATS 2
#define IMU_PACKET_ID_DATA  3

struct imu_packet {
  float ax, ay, az;
  float gx, gy, gz;
  float mx, my, mz;
};

static void emit_imu_packet(const uint8_t id, const char *msg) {
  printf("$P>%d:", id);
  printf(msg);
  printf("\r\n");
}

static void emit_imu_packet_stats() {
  char *msg;

  msg = json_asprintf("{}");
  if (!msg)
    return;

  emit_imu_packet(IMU_PACKET_ID_STATS, msg);
  free (msg);
}

static void emit_imu_packet_data(struct imu_packet *p) {
  char *msg;
  if (!p) return;

  msg = json_asprintf("{a: [%f, %f, %f], g: [%f, %f, %f], m: [%f, %f, %f]}",
      p->ax, p->ay, p->az,
      p->gx, p->gy, p->gz,
      p->mx, p->my, p->mz);
  if (!msg)
    return;

  emit_imu_packet(IMU_PACKET_ID_DATA, msg);
  free (msg);
}

static void emit_imu_packet_info(struct mgos_imu *imu) {
  char *msg;
  if (!imu) return;
  msg = json_asprintf("{accelerometer: { type: \"%s\" }, gyroscope: { type: \"%s\" }, magnetometer: { type: \"%s\" }, imu_hz: %d}",
      mgos_imu_accelerometer_get_name(imu),
      mgos_imu_gyroscope_get_name(imu),
      mgos_imu_magnetometer_get_name(imu),
      IMU_HZ);

  if (!msg) return;
  emit_imu_packet(IMU_PACKET_ID_INFO, msg);
  free(msg);
}

static void imu_cb(void *user_data) {
  static double last_info_emission = 0;
  static double last_stats_emission = 0;
  struct mgos_imu *imu = (struct mgos_imu *)user_data;
  struct imu_packet imu_packet;

  if (!imu) return;

  mgos_imu_accelerometer_get(imu, &imu_packet.ax, &imu_packet.ay, &imu_packet.az);
  mgos_imu_gyroscope_get(imu, &imu_packet.gx, &imu_packet.gy, &imu_packet.gz);
  mgos_imu_magnetometer_get(imu, &imu_packet.mx, &imu_packet.my, &imu_packet.mz);

  emit_imu_packet_data(&imu_packet);

  if (last_info_emission == 0 || (mg_time() - last_info_emission > IMU_PACKET_INFO_PERIOD)) {
    emit_imu_packet_info(imu);
    last_info_emission = mg_time();
  }
  if (last_stats_emission == 0 || (mg_time() - last_stats_emission > IMU_PACKET_STATS_PERIOD)) {
    emit_imu_packet_stats();
    last_stats_emission = mg_time();
  }
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

  if (!mgos_imu_accelerometer_create_i2c(imu, i2c, 0x68, ACC_MPU9250))
    LOG(LL_ERROR, ("Cannot create accelerometer on IMU"));
  if (!mgos_imu_gyroscope_create_i2c(imu, i2c, 0x68, GYRO_MPU9250))
    LOG(LL_ERROR, ("Cannot create gyroscope on IMU"));
  if (!mgos_imu_magnetometer_create_i2c(imu, i2c, 0x0C, MAG_AK8963))
    LOG(LL_ERROR, ("Cannot create magnetometer on IMU"));

  mgos_set_timer(1000 / IMU_HZ, true, imu_cb, imu);
  return MGOS_APP_INIT_SUCCESS;
}

