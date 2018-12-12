#include "mgos.h"
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>
#include "hexdump.h"

char *portname = "/dev/ttyUSB0";

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
    LOG(LL_ERROR,("error %d from tggetattr", errno));
    return;
  }

  tty.c_cc[VMIN]  = should_block ? 1 : 0;
  tty.c_cc[VTIME] = 5;                  // 0.5 seconds read timeout

  if (tcsetattr(fd, TCSANOW, &tty) != 0) {
    LOG(LL_ERROR, ("error %d setting term attributes", errno));
  }
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
    if (n>0) {
      LOG(LL_INFO, ("%d bytes read, head=%c", n, buf[0]));
      hexdump (buf, n);
    }
//    mgos_usleep(10000);
  }
  return 0;
}
