#include "mgos.h"
#include "mgos_i2c.h"
#include "mgos_imu.h"
#include "madgwick.h"
#include "imupacket.h"

static uint16_t s_imu_period        = 10;     // msec
static uint16_t s_imu_info_period   = 5000;
static uint16_t s_imu_quat_period   = 50;
static uint16_t s_imu_angles_period = 100;
static bool     s_calibrating       = false;

struct mgos_imu_madgwick *s_filter = NULL;

static void emit_imu_packet_data(struct imu_packet_data *p);
static void emit_imu_packet_info(void *user_data);
static void emit_imu_packet_quat(void *user_data);
static void emit_imu_packet_angles(void *user_data);
static void emit_imu_packet_log(const char *msg);
static void emit_imu_packet_offset(struct imu_packet_offset *p);

static void emit_imu_packet_quat(void *user_data) {
  struct imu_packet_quaternion p;
  struct mgos_imu_madgwick *   f = (struct mgos_imu_madgwick *)user_data;
  uint8_t packet[4 + 1 + sizeof(struct imu_packet_quaternion)]; // HEADER + DATALEN + DATA

  mgos_imu_madgwick_get_quaternion(f, &p.q0, &p.q1, &p.q2, &p.q3);

  packet[0] = '$';
  packet[1] = 'P';
  packet[2] = '>';
  packet[3] = 'D';
  packet[4] = sizeof(struct imu_packet_quaternion);
  memcpy(packet + 5, &p, packet[4]);
  mgos_uart_write(0, packet, sizeof(packet));
}

static void emit_imu_packet_angles(void *user_data) {
  struct imu_packet_angles  p;
  struct mgos_imu_madgwick *f = (struct mgos_imu_madgwick *)user_data;
  uint8_t packet[4 + 1 + sizeof(struct imu_packet_angles)]; // HEADER + DATALEN + DATA

  mgos_imu_madgwick_get_angles(f, &p.roll, &p.pitch, &p.yaw);

  packet[0] = '$';
  packet[1] = 'P';
  packet[2] = '>';
  packet[3] = 'E';
  packet[4] = sizeof(struct imu_packet_angles);
  memcpy(packet + 5, &p, packet[4]);
  mgos_uart_write(0, packet, sizeof(packet));
}

static void emit_imu_packet_offset(struct imu_packet_offset *p) {
  uint8_t packet[4 + 1 + sizeof(struct imu_packet_offset)]; // HEADER + DATALEN + DATA

  packet[0] = '$';
  packet[1] = 'P';
  packet[2] = '>';
  packet[3] = 'F';
  packet[4] = sizeof(struct imu_packet_offset);
  memcpy(packet + 5, p, packet[4]);
  mgos_uart_write(0, packet, sizeof(packet));
}

static void emit_imu_packet_data(struct imu_packet_data *p) {
  uint8_t packet[4 + 1 + sizeof(struct imu_packet_data)]; // HEADER + DATALEN + DATA

  packet[0] = '$';
  packet[1] = 'P';
  packet[2] = '>';
  packet[3] = 'A';
  packet[4] = sizeof(struct imu_packet_data);
  memcpy(packet + 5, p, packet[4]);
  mgos_uart_write(0, packet, sizeof(packet));
  return;
}

static void emit_imu_packet_info(void *user_data) {
  struct mgos_imu *imu = (struct mgos_imu *)user_data;
  const char *     a, *g, *m;
  uint8_t          len;

  a = mgos_imu_accelerometer_get_name(imu);
  g = mgos_imu_gyroscope_get_name(imu);
  m = mgos_imu_magnetometer_get_name(imu);


  len = strlen(a) + strlen(g) + strlen(m) + 2;
  mgos_uart_write(0, "$P>B", 4);
  mgos_uart_write(0, &len, 1);
  mgos_uart_write(0, a, strlen(a));
  mgos_uart_write(0, ",", 1);
  mgos_uart_write(0, g, strlen(g));
  mgos_uart_write(0, ",", 1);
  mgos_uart_write(0, m, strlen(m));

  emit_imu_packet_log("Hello world");
  return;
}

static void emit_imu_packet_log(const char *msg) {
  uint8_t len;

  len = strlen(msg);
  mgos_uart_write(0, "$P>C", 4);
  mgos_uart_write(0, &len, 1);
  mgos_uart_write(0, msg, strlen(msg));
}

static void imu_calibrate(struct mgos_imu *imu, struct imu_packet_data *p) {
  static struct imu_packet_offset o   = { 0, 0, 0, 0, 0, 0, 0 };
  static uint32_t calibration_samples = 300;

  if (!imu || !p) {
    return;
  }

  o.ax += p->ax;
  o.ay += p->ay;
  o.az += p->az;
  o.gx += p->gx;
  o.gy += p->gy;
  o.gz += p->gz;
  o.samples++;

  if (o.samples >= calibration_samples) {
    float ax, ay, az, gx, gy, gz;
    // One of the accel axes will be pulling G
    o.ax /= (float)o.samples;
    o.ay /= (float)o.samples;
    o.az /= (float)o.samples;
    if (fabs(o.ax) > 9.0f) {
      o.ax = 0;
    }
    if (fabs(o.ay) > 9.0f) {
      o.ay = 0;
    }
    if (fabs(o.az) > 9.0f) {
      o.az = 0;
    }

    o.gx /= (float)o.samples;
    o.gy /= (float)o.samples;
    o.gz /= (float)o.samples;

    emit_imu_packet_offset(&o);
    mgos_imu_gyroscope_get_offset(imu, &gx, &gy, &gz);
    mgos_imu_accelerometer_get_offset(imu, &ax, &ay, &az);

    gx -= o.gx;
    gy -= o.gy;
    gz -= o.gz;
    ax -= o.ax;
    ay -= o.ay;
    az -= o.az;
    mgos_imu_gyroscope_set_offset(imu, gx, gy, gz);
    mgos_imu_accelerometer_set_offset(imu, ax, ay, az);

    // Clear up counter for subsequent calls to imu_calibrate()
    memset(&o, 0, sizeof(struct imu_packet_offset));
    calibration_samples = 300;
    s_calibrating       = false;
  }
}

static void imu_cb(void *user_data) {
  struct mgos_imu *      imu = (struct mgos_imu *)user_data;
  struct imu_packet_data p;

  if (!imu) {
    return;
  }

  mgos_imu_accelerometer_get(imu, &p.ax, &p.ay, &p.az);
  mgos_imu_gyroscope_get(imu, &p.gx, &p.gy, &p.gz);
  mgos_imu_magnetometer_get(imu, &p.mx, &p.my, &p.mz);

  if (s_calibrating) {
    imu_calibrate(imu, &p);
  }

  mgos_imu_madgwick_update(s_filter,
                           p.gx, p.gy, p.gz,
                           p.ax, p.ay, p.az,
                           0, 0, 0);

  mgos_imu_madgwick_get_counter(s_filter, &p.filter_counter);
  emit_imu_packet_data(&p);
}

static void uart_dispatcher(int uart_no, void *arg) {
  static struct mbuf lb    = { 0 };
  size_t             rx_av = mgos_uart_read_avail(uart_no);
  char *        nl;
  size_t        llen;
  struct mg_str line;

  if (rx_av == 0) {
    return;
  }
  mgos_uart_read_mbuf(uart_no, &lb, rx_av);
  nl = (char *)mg_strchr(mg_mk_str_n(lb.buf, lb.len), '\n');
  if (nl == NULL) {
    return;
  }
  *nl  = '\0';
  llen = nl - lb.buf;
  line = mg_mk_str_n(lb.buf, llen);

  /* Also strip off CR */
  if (nl > lb.buf && *(nl - 1) == '\r') {
    *(nl - 1) = '\0';
    line.len--;
  }

  LOG(LL_INFO, ("UART%d> '%.*s'", uart_no, (int)line.len, line.p));
  if (mg_str_starts_with(line, mg_mk_str("calibrate"))) {
    emit_imu_packet_log("Starting calibration");
    s_calibrating = true;
  }

  /* Remove the line data from the buffer. */
  mbuf_remove(&lb, llen + 1);
  (void)arg;
}

enum mgos_app_init_result mgos_app_init(void) {
  struct mgos_i2c *i2c = mgos_i2c_get_global();
  struct mgos_imu *imu = mgos_imu_create();

  if (!i2c) {
    LOG(LL_ERROR, ("I2C bus missing, set i2c.enable=true in mos.yml"));
    return false;
  }

  if (!imu) {
    LOG(LL_ERROR, ("Cannot create IMU"));
    return false;
  }

  if (!mgos_imu_accelerometer_create_i2c(imu, i2c, 0x68, ACC_MPU9250)) {
    LOG(LL_ERROR, ("Cannot create accelerometer on IMU"));
  }
  if (!mgos_imu_gyroscope_create_i2c(imu, i2c, 0x68, GYRO_MPU9250)) {
    LOG(LL_ERROR, ("Cannot create gyroscope on IMU"));
  }
  if (!mgos_imu_magnetometer_create_i2c(imu, i2c, 0x0C, MAG_AK8963)) {
    LOG(LL_ERROR, ("Cannot create magnetometer on IMU"));
  }

  if (!(s_filter = mgos_imu_madgwick_create())) {
    LOG(LL_ERROR, ("Cannot create magnetometer on IMU"));
  }

  mgos_uart_set_dispatcher(0, uart_dispatcher, NULL /* arg */);
  mgos_uart_set_rx_enabled(0, true);

  // Install timers
  mgos_set_timer(s_imu_period, true, imu_cb, imu);
  mgos_set_timer(s_imu_info_period, true, emit_imu_packet_info, imu);
  mgos_set_timer(s_imu_quat_period, true, emit_imu_packet_quat, s_filter);
  mgos_set_timer(s_imu_angles_period, true, emit_imu_packet_angles, s_filter);
  return MGOS_APP_INIT_SUCCESS;
}
