#ifndef __OM_COM_H__
#define __OM_COM_H__

#include "om_core.h"

#define OM_COM_PACK_PREFIX 0x5C

#define OM_COM_TYPE(_data_type)       \
  struct __attribute__((packed)) {    \
    uint8_t prefix;                   \
    uint32_t topic_name_crc32;        \
    uint32_t data_len : 24;           \
    uint8_t pack_header_crc8;         \
    uint8_t data[sizeof(_data_type)]; \
    uint8_t pack_crc8;                \
  }

#define OM_COM_TYPE_SIZE(_data_type) sizeof(OM_COM_TYPE(_data_type))

typedef struct __attribute__((packed)) {
  uint8_t prefix;
  uint32_t topic_name_crc32;
  uint32_t data_len : 24;
  uint8_t pack_header_crc8;
  uint8_t pack_crc8;
} om_com_raw_type_t;

typedef enum {
  OM_COM_STATUS_WAIT_START,
  OM_COM_STATUS_WAIT_TOPIC,
  OM_COM_STATUS_WAIT_DATA_CRC
} om_com_recv_status_t;

typedef enum {
  OM_COM_RECV_SUCESS,
  OM_COM_RECV_NOT_FOUND,
  OM_COM_RECV_FAILED,
} om_com_recv_ans_t;

typedef struct {
  uint32_t crc32;
  om_topic_t* topic;
} om_com_map_item_t;

typedef struct {
  om_fifo_t recv_fifo;
  om_com_map_item_t* map;
  uint16_t map_len;
  uint16_t map_index;
  uint8_t* prase_buff;
  uint32_t prase_buff_len;
  om_com_recv_status_t status;
  uint32_t data_len;
  om_topic_t* topic;
} om_com_t;

om_status_t om_com_create_static(om_com_t* com, void* fifo_buff,
                                 uint32_t buffer_size, om_com_map_item_t* map,
                                 uint16_t map_len, uint8_t* prase_buff,
                                 uint32_t prase_buff_len);

om_status_t om_com_create(om_com_t* com, uint32_t buffer_size, uint16_t map_len,
                          uint32_t prase_buff_len);

om_status_t om_com_add_topic(om_com_t* com, om_topic_t* topic);

om_status_t om_com_add_topic_with_name(om_com_t* com, const char* topic_name);

om_status_t om_com_generate_pack(om_topic_t* topic, void* buff);

om_com_recv_ans_t om_com_prase_recv(om_com_t* com, uint8_t* buff, uint32_t size,
                                    bool block, bool in_isr);

#endif
