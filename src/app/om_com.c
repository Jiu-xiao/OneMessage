#include "om_com.h"

#include "om_crc.h"
#include "om_def.h"
#include "om_fifo.h"
#include "om_msg.h"

om_status_t om_com_create_static(om_com_t* com, void* fifo_buff,
                                 uint32_t buffer_size, om_com_map_item_t* map,
                                 uint16_t map_len, uint8_t* prase_buff,
                                 uint32_t prase_buff_len) {
  OM_ASSERT(com);
  OM_ASSERT(fifo_buff);
  OM_ASSERT(map);
  OM_ASSERT(prase_buff);
  OM_ASSERT(buffer_size >= OM_COM_TYPE_SIZE(void));
  OM_ASSERT(map_len);
  OM_ASSERT(prase_buff_len >= OM_COM_TYPE_SIZE(void));

  memset(com, 0, sizeof(*com));

  om_fifo_create(&com->recv_fifo, fifo_buff, buffer_size, sizeof(uint8_t));
  com->map = map;
  com->map_len = map_len;
  com->map_index = 0;
  com->prase_buff = prase_buff;
  com->prase_buff_len = prase_buff_len;
  memset(map, 0, sizeof(om_com_map_item_t) * map_len);

  return OM_OK;
}

om_status_t om_com_create(om_com_t* com, uint32_t buffer_size, uint16_t map_len,
                          uint32_t prase_buff_len) {
  OM_ASSERT(com);
  OM_ASSERT(buffer_size >= OM_COM_TYPE_SIZE(void));
  OM_ASSERT(map_len);
  OM_ASSERT(prase_buff_len >= OM_COM_TYPE_SIZE(void));

  return om_com_create_static(com, om_malloc(buffer_size), buffer_size,
                              om_malloc(sizeof(om_com_map_item_t) * map_len),
                              map_len, om_malloc(prase_buff_len),
                              prase_buff_len);
}

om_status_t om_com_add_topic(om_com_t* com, om_topic_t* topic){
  OM_ASSERT(com);
  OM_ASSERT(topic);

  if (com->map_index >= com->map_len) {
    OM_ASSERT(false);
    return OM_ERROR_FULL;
  }

  topic->crc32 = om_crc32_calc((const uint8_t*)topic->name,
                               strnlen(topic->name, OM_TOPIC_MAX_NAME_LEN));

  /* The result of crc32 should not be 0, please change the topic name */
  OM_ASSERT(topic->crc32 != 0);
  com->map[com->map_index].topic = topic;
  com->map[com->map_index].crc32 = topic->crc32;
  com->map_index++;

  return OM_OK;
}


om_status_t om_com_add_topic_with_name(om_com_t* com, const char* topic_name) {
  OM_ASSERT(com);
  OM_ASSERT(topic_name);

  if (com->map_index >= com->map_len) {
    OM_ASSERT(false);
    return OM_ERROR_FULL;
  }

  om_topic_t* topic = om_core_find_topic(topic_name, 0);
  OM_ASSERT(topic);

  topic->crc32 = om_crc32_calc((const uint8_t*)topic->name,
                               strnlen(topic->name, OM_TOPIC_MAX_NAME_LEN));

  /* The result of crc32 should not be 0, please change the topic name */
  OM_ASSERT(topic->crc32 != 0);
  com->map[com->map_index].topic = topic;
  com->map[com->map_index].crc32 = topic->crc32;
  com->map_index++;

  return OM_OK;
}

om_status_t om_com_generate_pack(om_topic_t* topic, void* buff) {
  OM_ASSERT(topic);
  OM_ASSERT(buff);

  if (topic->msg.buff == 0) {
    return OM_ERROR_NULL;
  }

  if (topic->crc32 == 0) {
    topic->crc32 = om_crc32_calc((const uint8_t*)topic->name,
                                 strnlen(topic->name, OM_TOPIC_MAX_NAME_LEN));
  }

  om_com_raw_type_t* raw_buff = buff;
  raw_buff->prefix = OM_COM_PACK_PREFIX;
  raw_buff->topic_name_crc32 = topic->crc32;
  raw_buff->data_len = topic->buff_len;
  memcpy(&raw_buff->pack_crc8, topic->msg.buff, topic->msg.size);

  raw_buff->pack_header_crc8 =
      om_crc8_calc(buff, om_offset_of(om_com_raw_type_t, pack_header_crc8));

  uint8_t* data_buff = (uint8_t*)buff;
  data_buff[sizeof(om_com_raw_type_t) + topic->buff_len - 1] =
      om_crc8_calc(buff, sizeof(om_com_raw_type_t) + topic->buff_len - 1);

  return OM_OK;
}

om_com_recv_ans_t om_com_prase_recv(om_com_t* com, uint8_t* buff, uint32_t size,
                                    bool block, bool in_isr) {
  OM_ASSERT(com);
  OM_ASSERT(buff);

  om_com_recv_ans_t ans = OM_COM_RECV_NOT_FOUND;

  om_fifo_writes(&com->recv_fifo, buff, size);

  while (om_fifo_readable(&com->recv_fifo)) {
    switch (com->status) {
      case OM_COM_STATUS_WAIT_START:
        om_fifo_peek(&com->recv_fifo, com->prase_buff);

        if (com->prase_buff[0] == OM_COM_PACK_PREFIX) {
          com->status = OM_COM_STATUS_WAIT_TOPIC;
        } else {
          om_fifo_pop(&com->recv_fifo);
        }

        continue;

      case OM_COM_STATUS_WAIT_TOPIC:
        if (om_fifo_readable_item_count(&com->recv_fifo) <
            om_offset_of(om_com_raw_type_t, pack_crc8)) {
          return ans;
        }

        om_fifo_peek_batch(&com->recv_fifo, com->prase_buff,
                           om_offset_of(om_com_raw_type_t, pack_crc8));

        if (om_crc8_verify(com->prase_buff,
                           om_offset_of(om_com_raw_type_t, pack_crc8))) {
          om_fifo_pop_batch(&com->recv_fifo,
                            om_offset_of(om_com_raw_type_t, pack_crc8));

          om_com_raw_type_t* pack = (om_com_raw_type_t*)com->prase_buff;

          for (int i = 0; i < com->map_index; i++) {
            if (pack->topic_name_crc32 == com->map[i].crc32) {
              if (pack->data_len + sizeof(om_com_raw_type_t) >
                      com->prase_buff_len ||
                  pack->data_len + sizeof(om_com_raw_type_t) >
                      com->recv_fifo.item_sum) {
                /* ERR: Buff is too small */
                OM_ASSERT(false);
              }

              com->topic = com->map[i].topic;
              com->data_len = pack->data_len;
              /* ERR: diff buff size in same topic */
              OM_ASSERT(com->data_len == com->topic->buff_len);
              com->status = OM_COM_STATUS_WAIT_DATA_CRC;
              continue;
            }
          }
        } else {
          om_fifo_pop(&com->recv_fifo);
        }

        continue;
      case OM_COM_STATUS_WAIT_DATA_CRC:
        if (om_fifo_readable_item_count(&com->recv_fifo) <
            com->data_len + sizeof(uint8_t)) {
          return ans;
        }

        om_fifo_reads(
            &com->recv_fifo,
            com->prase_buff + om_offset_of(om_com_raw_type_t, pack_crc8),
            com->data_len + sizeof(uint8_t));

        if (om_crc8_verify(com->prase_buff,
                           sizeof(om_com_raw_type_t) + com->data_len)) {
          om_publish(
              com->topic,
              com->prase_buff + om_offset_of(om_com_raw_type_t, pack_crc8),
              com->data_len, block, in_isr);
          com->status = OM_COM_STATUS_WAIT_START;
          ans = OM_COM_RECV_SUCESS;

        } else {
          com->status = OM_COM_STATUS_WAIT_START;
          ans = OM_COM_RECV_FAILED;
        }

        continue;
    }
  }

  return ans;
}
