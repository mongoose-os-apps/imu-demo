'use strict';

    function initDataArray(length) {
        var data = new Array(length);
        for (var i = 0; i < length; i++) {
            data[i] = new Array();
            data[i].min = -1;
            data[i].max = 1;
        }
        return data;
    }

    function addSampleToData(data, sampleNumber, sensorData) {
        for (var i = 0; i < data.length; i++) {
            var dataPoint = sensorData[i];
            data[i].push([sampleNumber, dataPoint]);
            if (dataPoint < data[i].min) {
                data[i].min = dataPoint;
            }
            if (dataPoint > data[i].max) {
                data[i].max = dataPoint;
            }
        }
        while (data[0].length > 300) {
            for (i = 0; i < data.length; i++) {
                data[i].shift();
            }
        }
        return sampleNumber + 1;
    }

    var margin = {top: 20, right: 10, bottom: 10, left: 40};
    function updateGraphHelperSize(helpers) {
        helpers.width = helpers.targetElement.width() - margin.left - margin.right;
        helpers.height = helpers.targetElement.height() - margin.top - margin.bottom;

        helpers.widthScale.range([0, helpers.width]);
        helpers.heightScale.range([helpers.height, 0]);

        helpers.xGrid.tickSize(-helpers.height, 0, 0);
        helpers.yGrid.tickSize(-helpers.width, 0, 0);
    }

    function initGraphHelpers(selector, sampleNumber, heightDomain) {
        var helpers = {selector: selector, targetElement: $(selector), dynamicHeightDomain: !heightDomain};

        helpers.widthScale = d3.scale.linear()
            .clamp(true)
            .domain([(sampleNumber - 299), sampleNumber]);

        helpers.heightScale = d3.scale.linear()
            .clamp(true)
            .domain(heightDomain || [1, -1]);

        helpers.xGrid = d3.svg.axis();
        helpers.yGrid = d3.svg.axis();

        updateGraphHelperSize(helpers);

        helpers.xGrid
            .scale(helpers.widthScale)
            .orient("bottom")
            .ticks(5)
            .tickFormat("");

        helpers.yGrid
            .scale(helpers.heightScale)
            .orient("left")
            .ticks(5)
            .tickFormat("");

        helpers.xAxis = d3.svg.axis()
            .scale(helpers.widthScale)
            .ticks(5)
            .orient("bottom")
            .tickFormat(function (d) {return d;});

        helpers.yAxis = d3.svg.axis()
            .scale(helpers.heightScale)
            .ticks(5)
            .orient("left")
            .tickFormat(function (d) {return d;});

        helpers.line = d3.svg.line()
            .x(function (d) {return helpers.widthScale(d[0]);})
            .y(function (d) {return helpers.heightScale(d[1]);});

        return helpers;
    }

    function drawGraph(graphHelpers, data, sampleNumber) {
        var svg = d3.select(graphHelpers.selector);

        if (graphHelpers.dynamicHeightDomain) {
            var limits = [];
            $.each(data, function (idx, datum) {
                limits.push(datum.min);
                limits.push(datum.max);
            });
            graphHelpers.heightScale.domain(d3.extent(limits));
        }
        graphHelpers.widthScale.domain([(sampleNumber - 299), sampleNumber]);

        svg.select(".x.grid").call(graphHelpers.xGrid);
        svg.select(".y.grid").call(graphHelpers.yGrid);
        svg.select(".x.axis").call(graphHelpers.xAxis);
        svg.select(".y.axis").call(graphHelpers.yAxis);

        var group = svg.select("g.data");
        var lines = group.selectAll("path").data(data, function (d, i) {return i;});
        var newLines = lines.enter().append("path").attr("class", "line");
        lines.attr('d', graphHelpers.line);
    }

var raw_data_text_ements = {
  x: [],
  y: [],
  z: []
};

$('.options .x, .options .y, .options .z').each(function () {
            var el = $(this);
            if (el.hasClass('x')) {
                raw_data_text_ements.x.push(el);
            } else if (el.hasClass('y')) {
                raw_data_text_ements.y.push(el);
            } else {
                raw_data_text_ements.z.push(el);
            }
        });

var samples_accel_i = 0;
var samples_gyro_i = 0;
var samples_mag_i = 0;
var accel_data = initDataArray(3);
var gyro_data = initDataArray(3);
var mag_data = initDataArray(3);
var accelHelpers = initGraphHelpers('#accel', samples_accel_i, [-0.50, 0.50]); // m/s/s (1G == 9.807m/s/s)
var gyroHelpers = initGraphHelpers('#gyro', samples_gyro_i, [-0.035, 0.035]);  // Rads/sec (2000dps == 34.9Rad/sec)
var magHelpers = initGraphHelpers('#mag', samples_mag_i, [-500, 500]);   // uTesla

function update_imu_graphs() {
  updateGraphHelperSize(accelHelpers);
  samples_accel_i = addSampleToData(accel_data, samples_accel_i, imuData.a);
  drawGraph(accelHelpers, accel_data, samples_accel_i);
  raw_data_text_ements.x[0].text(imuData.a[0].toFixed(2));
  raw_data_text_ements.y[0].text(imuData.a[1].toFixed(2));
  raw_data_text_ements.z[0].text(imuData.a[2].toFixed(2));

  updateGraphHelperSize(gyroHelpers);
  samples_gyro_i = addSampleToData(gyro_data, samples_gyro_i, imuData.g);
  drawGraph(gyroHelpers, gyro_data, samples_gyro_i);
  raw_data_text_ements.x[1].text(imuData.g[0].toFixed(2));
  raw_data_text_ements.y[1].text(imuData.g[1].toFixed(2));
  raw_data_text_ements.z[1].text(imuData.g[2].toFixed(2));

  updateGraphHelperSize(magHelpers);
  samples_mag_i = addSampleToData(mag_data, samples_mag_i, imuData.m);
  drawGraph(magHelpers, mag_data, samples_mag_i);
  raw_data_text_ements.x[2].text(imuData.m[0].toFixed(2));
  raw_data_text_ements.y[2].text(imuData.m[1].toFixed(2));
  raw_data_text_ements.z[2].text(imuData.m[2].toFixed(2));
}
