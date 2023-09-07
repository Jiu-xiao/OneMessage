#include "om_crc.h"

#define CRC8_INIT 0xFF
#define CRC32_INIT 0xFFFFFFFF

uint8_t om_crc8_tab[256];

uint32_t om_crc32_tab[256];

void om_generate_crc8_table() {
  uint8_t crc = 0;

  for (int i = 0; i < 256; i++) {
    om_crc8_tab[i] = i;
  }

  for (int i = 0; i < 256; i++) {
    for (int j = 7; j >= 0; j--) {
      crc = om_crc8_tab[i] & 0x01;

      if (crc) {
        om_crc8_tab[i] = om_crc8_tab[i] >> 1;
        om_crc8_tab[i] ^= 0x8c;
      } else {
        om_crc8_tab[i] = om_crc8_tab[i] >> 1;
      }
    }
  }
}

void om_generate_crc32_table() {
  uint32_t crc = 0;
  for (int i = 0; i < 256; ++i) {
    crc = i;
    for (int j = 0; j < 8; ++j) {
      if (crc & 1) {
        crc = (crc >> 1) ^ 0xEDB88320;
      } else {
        crc >>= 1;
      }
    }
    om_crc32_tab[i] = crc;
  }
}

uint8_t om_crc8_calc(const uint8_t* buf, size_t len) {
  uint8_t crc = CRC8_INIT;

  /* loop over the buffer data */
  while (len-- > 0) {
    crc = om_crc8_tab[(crc ^ *buf++) & 0xff];
  }

  return crc;
}

bool om_crc8_verify(const uint8_t* buf, size_t len) {
  if (len < 2) {
    return false;
  }
  uint8_t expected = om_crc8_calc(buf, len - sizeof(uint8_t));
  return expected == buf[len - sizeof(uint8_t)];
}

static inline uint32_t crc32_byte(uint32_t crc, const uint8_t data) {
  return om_crc32_tab[(crc ^ data) & 0xff] ^ (crc >> 8);
}

uint32_t om_crc32_calc(const uint8_t* buf, size_t len) {
  uint32_t crc = CRC32_INIT;
  while (len--) {
    crc = crc32_byte(crc, *buf++);
  }
  return crc;
}

bool om_crc32_verify(const uint8_t* buf, size_t len) {
  if (len < 2) return false;

  uint32_t expected = om_crc32_calc(buf, len - sizeof(uint32_t));
  return expected == ((const uint32_t*)((const uint8_t*)buf +
                                        (len % 4)))[len / sizeof(uint32_t) - 1];
}
