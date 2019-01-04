#include "mgos.h"
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>
#include "hexdump.h"
#include "imupacket.h"

char *portname = "/dev/ttyUSB12";

static int serial_interface_attribs(int fd, int speed, int parity) {
  struct termios tty;

  memset(&tty, 0, sizeof tty);
  if (tcgetattr(fd, &tty) != 0) {
    LOG(LL_ERROR, ("error %d from tcgetattr", errno));
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
    LOG(LL_ERROR, ("error %d from tcsetattr", errno));
    return -1;
  }
  return 0;
}

static void serial_blocking(int fd, int should_block) {
  struct termios tty;

  memset(&tty, 0, sizeof tty);
  if (tcgetattr(fd, &tty) != 0) {
    LOG(LL_ERROR, ("error %d from tggetattr", errno));
    return;
  }

  tty.c_cc[VMIN]  = should_block ? 1 : 0;
  tty.c_cc[VTIME] = 5;                  // 0.5 seconds read timeout

  if (tcsetattr(fd, TCSANOW, &tty) != 0) {
    LOG(LL_ERROR, ("error %d setting term attributes", errno));
  }
}

static void handleIMUData(void *packet) {
  struct imu_packet_data *d = (struct imu_packet_data *)packet;

  LOG(LL_DEBUG, ("ax=%.3f ay=%.3f az=%.3f gx=%.3f gy=%.3f gz=%.3f mx=%.3f my=%.3f mz=%.3f",
                 d->ax, d->ay, d->az, d->gx, d->gy, d->gz, d->mx, d->my, d->mz));
}

static void handleInfo(void *packet, uint8_t len) {
  LOG(LL_INFO, ("info='%.*s'", len, (char *)packet));
}

static void handleLog(void *packet, uint8_t len) {
  LOG(LL_INFO, ("log='%.*s'", len, (char *)packet));
  (void)packet;
}

static void handleQuat(void *packet) {
  struct imu_packet_quaternion *d = (struct imu_packet_quaternion *)packet;

  LOG(LL_INFO, ("q={%.3f %.3f %.3f %.3f}", d->q0, d->q1, d->q2, d->q3));
}

static void handleAngles(void *packet) {
  struct imu_packet_angles *d = (struct imu_packet_angles *)packet;

  LOG(LL_INFO, ("roll=%.3f pitch=%.3f yaw=%.3f", d->roll, d->pitch, d->yaw));
}

static void handleOffset(void *packet) {
  (void)packet;
}

static void serial_handle(char *buf, int n) {
  int     i = 0;
  uint8_t packet[256];
  uint8_t type, len, index, state;

  // LOG(LL_INFO, ("%d bytes read, head=%c", n, buf[0]));

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
          LOG(LL_WARN, ("Packet complete, unkonwn type=%d len=%d", type, len));
        }
        state = 0;
      }
      break;

    default:
      break;
    }
  }
//  hexdump (buf, n);
}

int main() {
  int fd = open(portname, O_RDWR | O_NOCTTY | O_SYNC);

  if (fd < 0) {
    LOG(LL_ERROR, ("error %d opening %s: %s", errno, portname, strerror(errno)));
    return -1;
  }

  serial_interface_attribs(fd, 115200, 0); // set speed to 115,200 bps, 8n1 (no parity)
  serial_blocking(fd, 0);                  // set no blocking

  for (;;) {
    char buf [10000];
    int  n = read(fd, buf, sizeof buf);
    if (n > 0) {
      serial_handle(buf, n);
    }
  }
  return 0;
}
