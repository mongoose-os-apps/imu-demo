"use strict";

var imuData = {a: [0, 0, 0], g: [0, 0, 0], m: [0, 0, 0]};
var imuInfo = {};
var imuStats = {};
var imuQuat = {q: [1.0, 0.0, 0.0, 0.0]};
var imuAngles = {r: 0, p:0, y:0};

function imu_packet_info(packet) {
  try {
    imuInfo = JSON.parse(packet);
    $('div#hud td.imu_rate').text(imuInfo.imu_hz+'Hz')
    $('div#hud td.imu_accel_type').text(imuInfo.accelerometer.type)
    $('div#hud td.imu_gyro_type').text(imuInfo.gyroscope.type)
    $('div#hud td.imu_mag_type').text(imuInfo.magnetometer.type)
  } catch (ignore) {
    console.log("Malformed INFO Packet: ", packet);
  }
//  console.log("Info: " + packet);
}

function imu_packet_stats(packet) {
  try {
    imuStats = JSON.parse(packet);
  } catch (ignore) {
    console.log("Malformed STATS Packet: ", packet);
  }
//  console.log("Stats: " + packet);
}

function imu_packet_quat(packet) {
  try {
    imuQuat = JSON.parse(packet);
  } catch (ignore) {
    console.log("Malformed QUAT Packet: ", packet);
  }
//  console.log("Quat: " + packet);
}

function imu_packet_angles(packet) {
  try {
    imuAngles = JSON.parse(packet);
  } catch (ignore) {
    console.log("Malformed ANGLES Packet: ", packet);
  }
//  console.log("Angles: " + packet);
}

function imu_packet_data(packet) {
  try {
    imuData = JSON.parse(packet);
  } catch (ignore) {
    console.log("Malformed DATA Packet: ", packet);
  }

  Madgwick.updateAHRS(
    imuData.g[0],     // gx
    imuData.g[1],     // gy
    imuData.g[2],     // gz
    imuData.a[0],     // ax
    imuData.a[1],     // ay
    imuData.a[2],     // az
    0,                // imuData.m[0],     // mx
    0,                // imuData.m[1],     // my
    0);               // imuData.m[2]);    // mz

  // Update graphs
  update_imu_graphs();
}
