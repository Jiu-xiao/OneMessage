#ifndef __OM_CRC_H__
#define __OM_CRC_H__

#include "om_def.h"

void om_generate_crc8_table();

void om_generate_crc32_table();

uint8_t om_crc8_calc(const uint8_t* buf, size_t len);

bool om_crc8_verify(const uint8_t* buf, size_t len);

uint32_t om_crc32_calc(const uint8_t* buf, size_t len);

bool om_crc32_verify(const uint8_t* buf, size_t len);

#endif
