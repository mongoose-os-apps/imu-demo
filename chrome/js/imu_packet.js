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

var IMUPacket = {
  state: 0,
  packetType: "",
  packetLength: 0,
  packetIndex: 0,
  packet: null,

  readSerial: function(inBytes) {
    for (var i = 0; i < inBytes.length; i++) {
      // Packets start with "$P>" (36,80,62), then a type (68), then a length (16), and then $(length) bytes.
      switch(this.state) {
        case 0:
          this.packetType="";
          this.packetLength=0;
          this.packetIndex=0;
          this.packet=null;
          if (inBytes[i] === 36) {
            this.state=1;
          }
          break;
        case 1:
          if (inBytes[i] === 80) {
            this.state=2;
          } else {
            this.state=0;
          }
          break;
        case 2:
          if (inBytes[i] === 62) {
            this.state=3;
          } else {
            this.state=0;
          }
          break;
        case 3:
          this.packetType = inBytes[i];
          this.state=4;
          break;
        case 4:
          this.packetLength = inBytes[i];
          this.packet = new Uint8Array(this.packetLength);
          this.state=5;
          break;
        case 5:
          this.packet[this.packetIndex]=inBytes[i];
          this.packetIndex++;
          if (this.packetIndex == this.packetLength) {
            // console.log("IMUPacket: Packet complete. type=" + this.packetType + ", len=", this.packetLength);
            switch(this.packetType) {
              case 65: this.handleStats(); break;
              case 66: this.handleInfo(); break;
              case 67: this.handleIMUData(); break;
              case 68: this.handleQuaternion(); break;
              case 69: this.handleAngles(); break;
            }
            this.state=0;
          }
          break;
      }
    }
  },
  handleStats: function() {
  },
  handleInfo: function() {
    var s = new TextDecoder("utf-8").decode(this.packet);
    console.log(s);
  },
  handleIMUData: function() {
  },
  handleQuaternion: function() {
  },
  handleAngles: function() {
  },
};
