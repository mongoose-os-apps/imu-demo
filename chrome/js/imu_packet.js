"use strict";

var imuData = {a: [0, 0, 0], g: [0, 0, 0], m: [0, 0, 0]};

function imu_packet_info(packet) {
  console.log("INFO Packet:", packet);
}

function imu_packet_stats(packet) {
  console.log("STATS Packet:", packet);
}

function imu_packet_data(packet) {
  try {
    imuData = JSON.parse(myString.slice(5));
  } catch (ignore) {
    console.log("Malformed DATA Packet: ", myString);
  }
}
