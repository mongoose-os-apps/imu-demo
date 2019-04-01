chrome.app.runtime.onLaunched.addListener(function () {
  chrome.app.window.create('index.html', {
    'bounds': {
      'width': 1000,
      'height': 600
    }
  }, function (w) {
    w.onClosed.addListener(function () {
      chrome.serial.getConnections(function (connections) {
        connections.forEach(function (c) {
          chrome.serial.disconnect(c.connectionId, function () { });
        });
      });
    });
  });
});
