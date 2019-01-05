'use strict';

var SerialStats = {
    previous_received:   0,
    previous_sent:       0,
    previous_imu_count:  0,
    previous_time:       0,
    timer:              -1,

    start: function() {
        var self = this;

      this.previous_time = Date.now();
        self.timer = setInterval(function() {
            self.update();
        }, 1000);
    },
    update: function() {
        var now = Date.now();
        var down = parseInt(((Serial.bytesReceived - this.previous_received) * 10 / Serial.bitrate) * 100);
        var up = parseInt(((Serial.bytesSent - this.previous_sent) * 10 / Serial.bitrate) * 100);
        var freq = (1000*(IMUPacket.count - this.previous_imu_count)) / (now-this.previous_time);

        this.previous_received = Serial.bytesReceived;
        this.previous_sent = Serial.bytesSent;
        this.previous_imu_count = IMUPacket.count;
        this.previous_time = now;

        $('div#footer span.serial_up').text(up+'%');
        $('div#footer span.serial_down').text(down+'%');
        $('div#footer span.imu_hz').text(freq.toFixed(1)+'Hz');
    },
    stop: function() {
      this.previous_received = 0;
      this.previous_sent = 0;
      if (this.timer != -1) {
        clearInterval(this.timer);
        this.timer = -1;
      }
      $('div#footer span.serial_up').text('-');
      $('div#footer span.serial_down').text('-');
      $('div#footer span.imu_hz').text('-');
    }
};

var Serial = {
	connected:       false,
    connectionId:    false,
    openRequested:   false,
    openCanceled:    false,
    bitrate:         0,
    bytesReceived:   0,
    bytesSent:       0,
    failed:          0,
    transmitting:    false,
    path:            "",
    outputBuffer:    [],


    connect: function (path, options, callback) {
        var self = this;
        self.openRequested = true;

        chrome.serial.connect(path, options, function (connectionInfo) {
            if (chrome.runtime.lastError) {
                console.error(chrome.runtime.lastError.message);
            }
            self.path = path;

            if (connectionInfo && !self.openCanceled) {
	            self.connected = true;
                self.connectionId = connectionInfo.connectionId;
                self.bitrate = connectionInfo.bitrate;
                self.bytesReceived = 0;
                self.bytesSent = 0;
                self.failed = 0;
                self.openRequested = false;

                self.onReceive.addListener(function log_bytesReceived(info) {
                    self.bytesReceived += info.data.byteLength;
                });

                self.onReceiveError.addListener(function watch_for_on_receive_errors(info) {
                    console.error(info);

                    switch (info.error) {
                        case 'system_error': // we might be able to recover from this one
                            if (!self.failed++) {
                                chrome.serial.setPaused(self.connectionId, false, function () {
                                    self.getInfo(function (info) {
                                        if (info) {
                                            if (!info.paused) {
                                                console.log('SERIAL: Connection recovered from last onReceiveError');

                                                self.failed = 0;
                                            } else {
                                                console.log('SERIAL: Connection did not recover from last onReceiveError, disconnecting');
                                                self.disconnect();
                                            }
                                        } else {
                                            if (chrome.runtime.lastError) {
                                                console.error(chrome.runtime.lastError.message);
                                            }
                                        }
                                    });
                                });
                            }
                            break;

                        case 'overrun':
                            // wait 50 ms and attempt recovery
                            self.error = info.error;
                            setTimeout(function() {
                                chrome.serial.setPaused(info.connectionId, false, function() {
                                    self.getInfo(function (info) {
                                        if (info) {
                                            if (info.paused) {
                                                // assume unrecoverable, disconnect
                                                console.log('SERIAL: Connection did not recover from ' + self.error + ' condition, disconnecting');
                                                self.disconnect();
                                            }
                                            else {
                                                console.log('SERIAL: Connection recovered from ' + self.error + ' condition');
                                            }
                                        }
                                    });
                                });
                            }, 50);
                            break;
                            
                        case 'timeout':
                            // TODO
                            break;
                            
                        case 'break':
                        case 'device_lost':
                            self.disconnect();
                            break;
                            
                        case 'disconnected':
                            // TODO
                            break;
                    }
                });

                console.log('SERIAL: Connection opened with ID: ' + connectionInfo.connectionId + ', Baud: ' + connectionInfo.bitrate);

                self.open(connectionInfo, callback);
            } else if (connectionInfo && self.openCanceled) {
                // connection opened, but this connect sequence was canceled
                // we will disconnect without triggering any callbacks
                self.connectionId = connectionInfo.connectionId;
                console.log('SERIAL: Connection opened with ID: ' + connectionInfo.connectionId + ', but request was canceled, disconnecting');

                // some bluetooth dongles/dongle drivers really doesn't like to be closed instantly, adding a small delay
                setTimeout(function initialization() {
                    self.openRequested = false;
                    self.openCanceled = false;
                    self.disconnect(function resetUI() {
                        self.open(false, callback);
                    });
                }, 150);
            } else if (self.openCanceled) {
                // connection didn't open and sequence was canceled, so we will do nothing
                console.log('SERIAL: Connection didn\'t open and request was canceled');
                self.openRequested = false;
                self.openCanceled = false;
                if (callback) callback(false);
            } else {
                self.openRequested = false;
                console.log('SERIAL: Failed to open serial port');
                if (callback) callback(false);
            }
        });
    },
    disconnect: function (callback) {
        var self = this;
        self.connected = false;

        if (self.connectionId) {
            self.emptyOutputBuffer();

            // remove listeners
            for (var i = (self.onReceive.listeners.length - 1); i >= 0; i--) {
                self.onReceive.removeListener(self.onReceive.listeners[i]);
            }

            for (var i = (self.onReceiveError.listeners.length - 1); i >= 0; i--) {
                self.onReceiveError.removeListener(self.onReceiveError.listeners[i]);
            }

            chrome.serial.disconnect(this.connectionId, function (result) {
                if (chrome.runtime.lastError) {
                    console.error(chrome.runtime.lastError.message);
                }

                if (result) {
                    console.log('SERIAL: Connection with ID: ' + self.connectionId + ' closed, Sent: ' + self.bytesSent + ' bytes, Received: ' + self.bytesReceived + ' bytes');
                  self.path = '';
                } else {
                    console.log('SERIAL: Failed to close connection with ID: ' + self.connectionId + ' closed, Sent: ' + self.bytesSent + ' bytes, Received: ' + self.bytesReceived + ' bytes');
                }

                self.connectionId = false;
                self.bitrate = 0;

                if (callback) callback(result);
            });
        } else {
            // connection wasn't opened, so we won't try to close anything
            // instead we will rise canceled flag which will prevent connect from continueing further after being canceled
            self.openCanceled = true;
        }
        SerialStats.stop();
    },
    getDevices: function (callback) {
        chrome.serial.getDevices(function (devices_array) {
            var devices = [];
            devices_array.forEach(function (device) {
                devices.push(device.path);
            });
            callback(devices);
        });
    },
    getInfo: function (callback) {
        chrome.serial.getInfo(this.connectionId, callback);
    },
    getControlSignals: function (callback) {
        chrome.serial.getControlSignals(this.connectionId, callback);
    },
    setControlSignals: function (signals, callback) {
        chrome.serial.setControlSignals(this.connectionId, signals, callback);
    },
    sendString: function(str) {
        var buf=new ArrayBuffer(str.length);
        var bufView=new Uint8Array(buf);
        for (var i=0; i<str.length; i++) {
          bufView[i]=str.charCodeAt(i);
        }
        this.send(buf, null);
    },
    send: function (data, callback) {
        var self = this;
        this.outputBuffer.push({'data': data, 'callback': callback});

        function send() {
            // store inside separate variables in case array gets destroyed
            var data = self.outputBuffer[0].data,
                callback = self.outputBuffer[0].callback;
            
            if (!self.connected) {
                console.log('SERIAL: Attempting to send when disconnected');
                if (callback) callback({
                    bytesSent: 0,
                    error: 'undefined'
               });
               return;
            }

            chrome.serial.send(self.connectionId, data, function (sendInfo) {
                if (sendInfo === undefined) {
                    console.log('SERIAL: undefined send error');
                    if (callback) callback({
                        bytesSent: 0,
                        error: 'undefined'
                   });
                   return;
                }
                
                // track sent bytes for statistics
                self.bytesSent += sendInfo.bytesSent;

                // fire callback
                if (callback) callback(sendInfo);

                // remove data for current transmission form the buffer
                self.outputBuffer.shift();

                // if there is any data in the queue fire send immediately, otherwise stop trasmitting
                if (self.outputBuffer.length) {
                    // keep the buffer withing reasonable limits
                    if (self.outputBuffer.length > 100) {
                        var counter = 0;

                        while (self.outputBuffer.length > 100) {
                            self.outputBuffer.pop();
                            counter++;
                        }

                        console.log('SERIAL: Send buffer overflowing, dropped: ' + counter + ' entries');
                    }

                    send();
                } else {
                    self.transmitting = false;
                }
            });
        }

        if (!this.transmitting) {
            this.transmitting = true;
            send();
        }
    },
    onReceive: {
        listeners: [],

        addListener: function (function_reference) {
            chrome.serial.onReceive.addListener(function_reference);
            this.listeners.push(function_reference);
        },
        removeListener: function (function_reference) {
            for (var i = (this.listeners.length - 1); i >= 0; i--) {
                if (this.listeners[i] == function_reference) {
                    chrome.serial.onReceive.removeListener(function_reference);

                    this.listeners.splice(i, 1);
                    break;
                }
            }
        }
    },
    onReceiveError: {
        listeners: [],

        addListener: function (function_reference) {
            chrome.serial.onReceiveError.addListener(function_reference);
            this.listeners.push(function_reference);
        },
        removeListener: function (function_reference) {
            for (var i = (this.listeners.length - 1); i >= 0; i--) {
                if (this.listeners[i] == function_reference) {
                    chrome.serial.onReceiveError.removeListener(function_reference);

                    this.listeners.splice(i, 1);
                    break;
                }
            }
        }
    },
    open: function(openInfo, callback) {
      if (openInfo) {
          this.onReceive.addListener(callback);
          SerialStats.start();
      }
    },
    emptyOutputBuffer: function () {
        this.outputBuffer = [];
        this.transmitting = false;
    }
};

var onReceiveCallback = function (info) {
    if (info.connectionId === Serial.connectionId && info.data) {
        IMUPacket.readSerial(new Uint8Array(info.data));
    }
};
