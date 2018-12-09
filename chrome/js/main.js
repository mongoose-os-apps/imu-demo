'use strict';

// Initialize Madgwick filter at 100Hz.
Madgwick.init(100);

Serial.connect("/dev/ttyUSB1", {bitrate: 115200}, onReceiveCallback); // in serial.js

// Populate the serial ports dropdown
Serial.getDevices(function (ports) {
  $('div#ports #port').html(''); // clear list
  for (var i = 0; i < ports.length; i++) {
    $('div#ports #port').append($("<option/>", {value: ports[i], text: ports[i]}));
  }
});

// THREE renderer
Scene.init("canvas");

// Update Cube from Madgwick Quaternion
setInterval(function () {
    cube.quaternion.set(Madgwick.q1, Madgwick.q2, Madgwick.q3, Madgwick.q0);
}, 1000/Madgwick.sampleFreq);
