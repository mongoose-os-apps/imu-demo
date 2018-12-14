#include "mgos.h"
#include "mgos_i2c.h"
#include "mgos_imu.h"
#include "madgwick.h"
#include "imupacket.h"

void imupacket_emit_data(struct imu_packet_data *p) {
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

void imupacket_emit_info(void *user_data) {
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

  imupacket_emit_log("Hello world");
  return;
}

void imupacket_emit_log(const char *msg) {
  uint8_t len;

  len = strlen(msg);
  mgos_uart_write(0, "$P>C", 4);
  mgos_uart_write(0, &len, 1);
  mgos_uart_write(0, msg, strlen(msg));
}

void imupacket_emit_quat(void *user_data) {
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

void imupacket_emit_angles(void *user_data) {
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

void imupacket_emit_offset(struct imu_packet_offset *p) {
  uint8_t packet[4 + 1 + sizeof(struct imu_packet_offset)]; // HEADER + DATALEN + DATA

  packet[0] = '$';
  packet[1] = 'P';
  packet[2] = '>';
  packet[3] = 'F';
  packet[4] = sizeof(struct imu_packet_offset);
  memcpy(packet + 5, p, packet[4]);
  mgos_uart_write(0, packet, sizeof(packet));
}
