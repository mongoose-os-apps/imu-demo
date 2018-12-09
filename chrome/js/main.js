'use strict';

Serial.connect("/dev/ttyUSB1", {bitrate: 115200}, onReceiveCallback); // in serial.js

// Populate the serial ports dropdown
Serial.getDevices(function (ports) {
  $('div#ports #port').html(''); // clear list
  for (var i = 0; i < ports.length; i++) {
    $('div#ports #port').append($("<option/>", {value: ports[i], text: ports[i]}));
  }
});
