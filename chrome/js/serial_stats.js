'use strict';

var SerialStats = {
    previous_received: 0,
    previous_sent:     0,

    initialize: function() {
        var self = this;

        self.main_timer_reference = setInterval(function() {
            self.update();
        }, 250);
    },
    update: function() {
        var down = parseInt(((bytesReceived - this.previous_received) * 10 / bitrate) * 100);
        var up = parseInt(((bytesSent - this.previous_sent) * 10 / bitrate) * 100);

        this.previous_received = bytesReceived;
        this.previous_sent = bytesSent;

        $('div#footer span.serial_up').text('U: '+up+'%');
        $('div#footer span.serial_down').text('D: '+down+'%');
    },
    reset: function() {
        this.previous_received = 0;
        this.previous_sent = 0;
    }
};

SerialStats.initialize();
