/* Some functions mocked from MGOS, so we can run unit tests standalone.
 */

#include "mgos_mock.h"

int _mgos_timers = 0;

int log_print_prefix(enum cs_log_level l, const char *func, const char *file) {
  char ll_str[6];
  char fn_str[16];
  char fu_str[41];

  switch (l) {
  case LL_ERROR:
    strncpy(ll_str, "ERROR", sizeof(ll_str));
    break;

  case LL_WARN:
    strncpy(ll_str, "WARN", sizeof(ll_str));
    break;

  case LL_INFO:
    strncpy(ll_str, "INFO", sizeof(ll_str));
    break;

  case LL_DEBUG:
    strncpy(ll_str, "DEBUG", sizeof(ll_str));
    break;

  case LL_VERBOSE_DEBUG:
    strncpy(ll_str, "VERB", sizeof(ll_str));
    break;

  default:   // LL_NONE
    strncpy(ll_str, "?????", sizeof(ll_str));
    return 0;
  }

  memset(fu_str, 0, sizeof(fu_str));
  strncpy(fu_str, func, sizeof(fu_str) - 1);

  memset(fn_str, 0, sizeof(fn_str));
  strncpy(fn_str, file, sizeof(fn_str) - 1);

  printf("%-5s %-15s %-40s| ", ll_str, fn_str, fu_str);
  return 1;
}

mgos_timer_id mgos_set_timer(int msecs, int flags, timer_callback cb, void *cb_arg) {
  _mgos_timers++;
  LOG(LL_INFO, ("Installing timer -- %d timers currently installed", _mgos_timers));
  (void)msecs;
  (void)flags;
  (void)cb;
  (void)cb_arg;

  return _mgos_timers;
}

void mgos_clear_timer(mgos_timer_id id) {
  _mgos_timers--;
  LOG(LL_INFO, ("Clearing timer -- %d timers currently installed", _mgos_timers));
  (void)id;

  return;
}

double mg_time() {
  return (float)time(NULL);
}

double mgos_uptime() {
  return (double)time(NULL);
}

char *mgos_sys_ro_vars_get_mac_address() {
  return "00:11:22:33:44:55";
}

char *mgos_sys_ro_vars_get_arch() {
  return "esp32";
}

bool mgos_net_get_ip_info(enum mgos_net_if_type if_type, int if_instance, struct mgos_net_ip_info *ip_info) {
  return true;
}

void mgos_net_ip_to_str(const struct sockaddr_in *sin, char *out) {
  return;
}

void mgos_usleep(uint32_t usecs) {
  usleep(usecs);
}

void mgos_msleep(uint32_t msecs) {
  usleep(1000 * msecs);
}
