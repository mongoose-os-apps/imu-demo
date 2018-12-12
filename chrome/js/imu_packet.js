"use strict";

var imuData = {a: [0, 0, 0], g: [0, 0, 0], m: [0, 0, 0]};
var imuInfo = {accelerometer: {type: ""}, gyroscope: {type: ""}, magnetometer: {type: ""}};
var imuQuat = {q: [1.0, 0.0, 0.0, 0.0]};
var imuAngles = {roll: 0, pitch:0, yaw:0};

var IMUPacket = {
  state: 0,
  packetType: "",
  packetLength: 0,
  packetIndex: 0,
  packet: null,
  count: 0,

  readSerial: function(inBytes) {
//    console.log("SERIAL Read: " + inBytes.length);
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
              case 65: this.handleIMUData(); break;
              case 66: this.handleInfo(); break;
            }
            this.state=0;
          }
          break;
      }
    }
  },
  handleInfo: function() {
    var s = new TextDecoder("utf-8").decode(this.packet).split(',');
    imuInfo.gyroscope.type = s[0];
    imuInfo.accelerometer.type = s[1];
    imuInfo.magnetometer.type = s[2];
    $('div#info td.imu_accel_type').text(imuInfo.accelerometer.type)
    $('div#info td.imu_gyro_type').text(imuInfo.gyroscope.type)
    $('div#info td.imu_mag_type').text(imuInfo.magnetometer.type)
  },
  handleIMUData: function() {
    var view = new DataView(this.packet.buffer);
    imuData.a[0] = view.getFloat32(0, true);
    imuData.a[1] = view.getFloat32(4, true);
    imuData.a[2] = view.getFloat32(8, true);
    imuData.g[0] = view.getFloat32(12, true);
    imuData.g[1] = view.getFloat32(16, true);
    imuData.g[2] = view.getFloat32(20, true);
    imuData.m[0] = view.getFloat32(24, true);
    imuData.m[1] = view.getFloat32(28, true);
    imuData.m[2] = view.getFloat32(32, true);
    imuQuat.q[0] = view.getFloat32(36, true);
    imuQuat.q[1] = view.getFloat32(40, true);
    imuQuat.q[2] = view.getFloat32(44, true);
    imuQuat.q[3] = view.getFloat32(48, true);
    imuAngles.roll = view.getFloat32(52, true) * 180 / Math.PI;
    imuAngles.pitch = view.getFloat32(56, true) * 180 / Math.PI;
    imuAngles.yaw = view.getFloat32(60, true) * 180 / Math.PI;
    this.count = view.getUint32(64, true);

    // Update Madgwick filter
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

    // Update roll/pitch/yaw
    $('span.imu_roll').text(imuAngles.roll.toFixed(1))
    $('span.imu_pitch').text(imuAngles.pitch.toFixed(1))
    $('span.imu_yaw').text(imuAngles.yaw.toFixed(1))
  },
};
