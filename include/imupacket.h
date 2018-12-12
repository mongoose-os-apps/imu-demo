#pragma once

#include "mgos.h"

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
