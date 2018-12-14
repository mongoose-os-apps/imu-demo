#include "mgos.h"
#include "mgos_i2c.h"
#include "mgos_imu.h"
#include "madgwick.h"
#include "imupacket.h"
#include "serial.h"

extern bool s_calibrating;
extern struct mgos_imu_madgwick *s_filter;

static void serial_dispatcher(int uart_no, void *arg) {
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
    imupacket_emit_log("Starting calibration");
    s_calibrating = true;
  } else if (mg_str_starts_with(line, mg_mk_str("resetquat"))) {
    mgos_imu_madgwick_reset(s_filter);
    imupacket_emit_log("Resetting Quaternion");
  }

  /* Remove the line data from the buffer. */
  mbuf_remove(&lb, llen + 1);
  (void)arg;
}

void serial_init() {
  mgos_uart_set_dispatcher(0, serial_dispatcher, NULL /* arg */);
  mgos_uart_set_rx_enabled(0, true);
}
