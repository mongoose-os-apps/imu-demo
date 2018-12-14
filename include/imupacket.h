#pragma once

#include "mgos.h"

struct imu_packet_data {
  // IMU Data
  float    ax, ay, az;
  float    gx, gy, gz;
  float    mx, my, mz;
  // Filter counter
  uint32_t filter_counter;
};

struct imu_packet_quaternion {
  // Quaternion
  float q0, q1, q2, q3;
};

struct imu_packet_angles {
  // Roll/Pitch/Yaw in Rads
  float roll, pitch, yaw;
};

struct imu_packet_offset {
  // Average over $(samples) data from the IMU in Accel and Gyro sensors
  float    ax, ay, az;
  float    gx, gy, gz;

  // IMU sample count
  uint32_t samples;
};

void imupacket_emit_data(struct imu_packet_data *p);
void imupacket_emit_info(void *user_data);
void imupacket_emit_quat(void *user_data);
void imupacket_emit_angles(void *user_data);
void imupacket_emit_log(const char *msg);
void imupacket_emit_offset(struct imu_packet_offset *p);
