#include "mgos.h"
#include <errno.h>
#include <ncurses.h>
#include <fcntl.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>
#include "imupacket.h"


#define DEG2RAD    (.017453292519943f)      /* PI / 180 */
#define RAD2DEG    (57.2957795130823f)      /* 180 / PI */


WINDOW *log_window;
char *  portname = "/dev/ttyUSB1";

static int serial_interface_attribs(int fd, int speed, int parity) {
  struct termios tty;

  memset(&tty, 0, sizeof tty);
  if (tcgetattr(fd, &tty) != 0) {
    fprintf(stderr, "error from tcgetattr: %s", strerror(errno));
    return -1;
  }

  cfsetospeed(&tty, speed);
  cfsetispeed(&tty, speed);

  tty.c_cflag = (tty.c_cflag & ~CSIZE) | CS8;           // 8-bit chars
  // disable IGNBRK for mismatched speed tests; otherwise receive break
  // as \000 chars
  tty.c_iflag &= ~IGNBRK;                 // disable break processing
  tty.c_lflag  = 0;                       // no signaling chars, no echo,
                                          // no canonical processing
  tty.c_oflag     = 0;                    // no remapping, no delays
  tty.c_cc[VMIN]  = 0;                    // read doesn't block
  tty.c_cc[VTIME] = 5;                    // 0.5 seconds read timeout

  tty.c_iflag &= ~(IXON | IXOFF | IXANY); // shut off xon/xoff ctrl

  tty.c_cflag |= (CLOCAL | CREAD);        // ignore modem controls,
                                          // enable reading
  tty.c_cflag &= ~(PARENB | PARODD);      // shut off parity
  tty.c_cflag |= parity;
  tty.c_cflag &= ~CSTOPB;
  tty.c_cflag &= ~CRTSCTS;

  if (tcsetattr(fd, TCSANOW, &tty) != 0) {
    fprintf(stderr, "error from tcsetattr: %s", strerror(errno));
    return -1;
  }
  return 0;
}

static void serial_blocking(int fd, int should_block) {
  struct termios tty;

  memset(&tty, 0, sizeof tty);
  if (tcgetattr(fd, &tty) != 0) {
    fprintf(stderr, "error from tggetattr: %s", strerror(errno));
    return;
  }

  tty.c_cc[VMIN]  = should_block ? 1 : 0;
  tty.c_cc[VTIME] = 5;                  // 0.5 seconds read timeout

  if (tcsetattr(fd, TCSANOW, &tty) != 0) {
    fprintf(stderr, "error setting term attributes: %s", strerror(errno));
  }
}

static void handleIMUData(void *packet) {
  struct imu_packet_data *d = (struct imu_packet_data *)packet;

  mvwprintw(log_window, 2, 1, "IMU Data:");
  mvwprintw(log_window, 3, 3, "Accelerometer:");
  mvwprintw(log_window, 3, 18, "ax=%+ 9.3f  ", d->ax);
  mvwprintw(log_window, 3, 31, "ay=%+ 9.3f  ", d->ay);
  mvwprintw(log_window, 3, 44, "az=%+ 9.3f  ", d->az);
  mvwprintw(log_window, 4, 7, "Gyroscope:");
  mvwprintw(log_window, 4, 18, "gx=%+ 9.3f  ", d->gx);
  mvwprintw(log_window, 4, 31, "gy=%+ 9.3f  ", d->gy);
  mvwprintw(log_window, 4, 44, "gz=%+ 9.3f  ", d->gz);
  mvwprintw(log_window, 5, 4, "Magnetometer:");
  mvwprintw(log_window, 5, 18, "mx=%+ 9.3f", d->mx);
  mvwprintw(log_window, 5, 31, "my=%+ 9.3f", d->my);
  mvwprintw(log_window, 5, 44, "mz=%+ 9.3f", d->mz);
}

static void handleInfo(void *packet, uint8_t len) {
  mvwprintw(log_window, 1, 1, "Info:       %.*s", len, (char *)packet);
  (void)packet;
  (void)len;
}

static void handleLog(void *packet, uint8_t len) {
  mvwprintw(log_window, 14, 1, "Log: %.*s", len, (char *)packet);
  (void)packet;
  (void)len;
}

static void handleQuat(void *packet) {
  struct imu_packet_quaternion *d = (struct imu_packet_quaternion *)packet;

  mvwprintw(log_window, 7, 1, "Quaternion:");
  mvwprintw(log_window, 7, 13, "q0=%+ 6.3f   ", d->q0);
  mvwprintw(log_window, 7, 23, "q1=%+ 6.3f   ", d->q1);
  mvwprintw(log_window, 7, 33, "q2=%+ 6.3f   ", d->q2);
  mvwprintw(log_window, 7, 43, "q3=%+ 6.3f   ", d->q3);
  (void)d;
}

static void handleAngles(void *packet) {
  struct imu_packet_angles *d = (struct imu_packet_angles *)packet;

  mvwprintw(log_window, 9, 1, "Angles:");
  mvwprintw(log_window, 10, 3, "  Roll: %- 4d    ", (int)(RAD2DEG * d->roll));
  mvwprintw(log_window, 11, 3, " Pitch: %- 4d    ", (int)(RAD2DEG * d->pitch));
  mvwprintw(log_window, 12, 3, "   Yaw: %- 4d    ", (int)(RAD2DEG * d->yaw));
  (void)d;
}

static void handleOffset(void *packet) {
  (void)packet;
}

static void serial_handle(char *buf, int n) {
  int     i = 0;
  uint8_t packet[256];
  uint8_t type, len, index, state;

  i = 0; state = 0;
  for (i = 0; i < n; i++) {
    switch (state) {
    case 0: type = 0; len = 0; index = 0; memset(packet, 0, sizeof(packet));
      if (buf[i] == '$') {
        state = 1;
      }
      break;

    case 1:
      if (buf[i] == 'P') {
        state = 2;
      } else{
        state = 0;
      }
      break;

    case 2:
      if (buf[i] == '>') {
        state = 3;
      } else{
        state = 0;
      }
      break;

    case 3:
      type  = buf[i];
      state = 4;
      break;

    case 4:
      len   = buf[i];
      state = 5;
      break;

    case 5:
      packet[index] = buf[i];
      index++;
      if (index == len) {
        switch (type) {
        case 65: handleIMUData(packet); break;

        case 66: handleInfo(packet, len); break;

        case 67: handleLog(packet, len); break;

        case 68: handleQuat(packet); break;

        case 69: handleAngles(packet); break;

        case 70: handleOffset(packet); break;

        default:
                 break;
        }
        state = 0;
      }
      break;

    default:
      break;
    }
  }
}

int main() {
  int fd = open(portname, O_RDWR | O_NOCTTY | O_SYNC);
  int ch;

  if (fd < 0) {
    fprintf(stderr, "error opening %s: %s", portname, strerror(errno));
    return -1;
  }

  // Start ncurses
  initscr();
  noecho();
  refresh();
  timeout(1);

  log_window = newwin(20, COLS, 0, 0);
  box(log_window, 0, 0);
  mvwprintw(log_window, 19, COLS - 20, " Press 'q' to exit ");
  wrefresh(log_window);

  serial_interface_attribs(fd, 115200, 0); // set speed to 115,200 bps, 8n1 (no parity)
  serial_blocking(fd, 0);                  // set no blocking

  while ((ch = getch()) != 'q') {
    char buf [10000];
    int  n = read(fd, buf, sizeof buf);
    if (n > 0) {
      serial_handle(buf, n);
    }
    wrefresh(log_window);
    refresh();
  }
  endwin();

  printf("Thank you very much :)\n");
  return 0;

  (void)ch;
}
