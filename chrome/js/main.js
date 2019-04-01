'use strict';

window.addEventListener("resize", function () {
  const height = window.innerHeight / 3;

  $('.grid.x').attr('transform', `translate(40, ${height - 30})`);
  $('.axis.x').attr('transform', `translate(40, ${height - 30})`);
});

// Initialize Madgwick filter at 100Hz.
Madgwick.init(100);

function connect(port) {
  Serial.connect(port, { bitrate: 115200 }, function (info) {
    if (info.connectionId === Serial.connectionId && info.data) {
      IMUPacket.readSerial(new Uint8Array(info.data));
    }
  });

  chrome.storage.local.set({ port });
}

chrome.storage.local.get(['port'], function ({ port }) {
  Serial.getDevices(function (ports) {
    const prevUsedPort = port;

    $('#port').html(''); // clear list
    for (var i = 0; i < ports.length; i++) {
      $('#port').append($("<option/>", { value: ports[i], text: ports[i] }));
    }

    if (Serial.connected) {
      return;
    }

    if (prevUsedPort) {
      $('#port').val(prevUsedPort);
      connect(prevUsedPort);
    } else {
      connect(ports[0]);
    }
  });
});

$('#port').change(function () {
  const port = $(this).val();

  if (Serial.connected) {
    Serial.disconnect(() => connect(port));
  } else {
    connect(port);
  }
});


// THREE renderer
Scene.init("canvas");
function animate() {
  requestAnimationFrame(animate);
  Scene.renderer.render(Scene.scene, Scene.camera);
}
animate();


// Update Cube from Madgwick Quaternion
setInterval(function () {
  Scene.cube.quaternion.set(imuQuat.q[1], imuQuat.q[2], imuQuat.q[3], imuQuat.q[0]);
}, 1000 / Madgwick.sampleFreq);
