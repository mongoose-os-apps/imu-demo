'use strict';

var PortStats = {
    counter:      0,

    initialize: function() {
        var self = this;

        self.main_timer_reference = setInterval(function() {
            self.update();
        }, 1000);
    },
    update: function() {
        $('span.foo').text(counter);
    },
    reset: function() {
        this.counter = 0;
    }
};

