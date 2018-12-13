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
