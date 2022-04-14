#include "om_run.h"

#include "om_afl.h"
#include "om_fmt.h"
#include "om_log.h"
#include "om_msg.h"

#define STR_SELECT(_bool, _str, _str1) ((_bool) ? "" _str : ""_str1)

#if OM_REPORT_ACTIVITY

static const char OM_REPORT_MAP_PREFIX[] = {'@', 'm', 's', 'g'};

static const char OM_REPORT_MAP_SUFFIX[] = {'@', 'e', 'n', 'd'};

static om_report_t om_report_buff[OM_REPORT_DATA_BUFF_NUM];

static uint8_t om_report_map[OM_REPORT_MAP_BUFF_SIZE +
                             sizeof(OM_REPORT_MAP_PREFIX) +
                             sizeof(OM_REPORT_MAP_SUFFIX)];

static uint32_t om_report_data_num = 0;

static uint32_t om_report_map_len = 0;

static om_mutex_t om_report_mutex;

#ifndef om_get_realtime
uint32_t om_get_realtime() {
  static uint32_t tick;

  return tick++;
}
#endif

#ifndef om_report_transmit
void om_report_transmit(uint8_t* buff, uint32_t size) {
  OM_UNUSED(buff);
  OM_UNUSED(size);
}
#endif

om_status_t om_run_init() {
  om_mutex_init(&om_report_mutex);
  om_mutex_unlock(&om_report_mutex);

  return OM_OK;
}

om_status_t _om_generate_map(om_topic_t* topic, void* arg) {
  uint8_t** buff = (uint8_t**)arg;
  uint32_t len = strlen(topic->name) + 1;

  if (*buff - om_report_map + len <= OM_REPORT_MAP_BUFF_SIZE) {
    sprintf((char*)*buff, "%s", topic->name);
    *buff += len;
    return OM_OK;
  }

  return OM_ERROR;
}

om_status_t om_generate_map() {
  uint8_t* buff = om_report_map + sizeof(OM_REPORT_MAP_PREFIX);
  om_msg_for_each(_om_generate_map, &buff);

  memcpy(om_report_map, OM_REPORT_MAP_PREFIX, sizeof(OM_REPORT_MAP_PREFIX));
  memcpy(buff, OM_REPORT_MAP_SUFFIX, sizeof(OM_REPORT_MAP_SUFFIX));

  buff += sizeof(OM_REPORT_MAP_SUFFIX);

  om_report_map_len = buff - om_report_map;

  return OM_OK;
}

om_status_t om_run_add_report(om_activity_t activity, uint32_t id) {
  om_mutex_lock(&om_report_mutex);

  if (om_report_data_num >= OM_REPORT_DATA_BUFF_NUM) {
    om_mutex_unlock(&om_report_mutex);
    return OM_ERROR;
  }

  om_report_buff[om_report_data_num].id_activity = activity + (id << 16);
  om_report_buff[om_report_data_num].time = om_get_realtime();

  om_report_data_num++;

  om_mutex_unlock(&om_report_mutex);

  return OM_OK;
}

om_status_t om_send_report_data() {
  om_mutex_lock(&om_report_mutex);

  if (om_report_data_num > 0) {
    om_generate_map();

    om_report_transmit(om_report_map, om_report_map_len);

    om_report_transmit((uint8_t*)om_report_buff,
                       om_report_data_num * sizeof(om_report_t));
  }

  om_report_data_num = 0;

  om_mutex_unlock(&om_report_mutex);

  return OM_OK;
}

#endif

om_status_t om_print_suber_message(om_suber_t* suber, char* buff,
                                   uint32_t buff_size) {
  switch (suber->mode) {
    case OM_SUBER_MODE_DEFAULT:
      snprintf(buff, buff_size,
               "\tsuber mode:\t[%s] --[sub_callback]--> user_fun[%p]\r\n",
               suber->master->name, suber->data.as_suber.sub_callback);
      break;
    case OM_SUBER_MODE_LINK:
      snprintf(buff, buff_size, "\t\tsuber mode:\t[%s] --[link]--> [%s]\r\n",
               suber->master->name, suber->data.as_link.target->name);
      break;
    case OM_SUBER_MODE_EXPORT:
      snprintf(buff, buff_size,
               "\tsuber mode:\t[%s] --[export]--> buffer:%d[%p]\r\n",
               suber->master->name, (int)suber->data.as_export.max_size,
               suber->data.as_export.buff);
      break;
    case OM_SUBER_MODE_UNKNOW:
      snprintf(buff, buff_size, "\t\tsuber mode:\t[%s] --[unknow]-- [%p]\r\n",
               suber->master->name, suber);
      break;
    default:
      OM_ASSERT(false);
      return OM_ERROR;
  }

  return OM_OK;
}

om_status_t om_print_link_message(om_link_t* link, char* buff,
                                  uint32_t buff_size) {
  snprintf(buff, buff_size, "\tlink mode:\tthis <-- [%s]\r\n",
           link->source.topic->name);

  return OM_OK;
}

om_status_t om_print_afl_message(om_filter_t* filter, char* buff,
                                 uint32_t buff_size) {
  char* mode_str;
  switch (filter->mode) {
    case OM_AFL_MODE_RANGE:
      mode_str = "range";
      snprintf(
          buff, buff_size, "\tfilter mode:\tthis --(%s %x:%x)--> [%s]\r\n",
          mode_str, (unsigned int)filter->data.range.start,
          (unsigned int)(filter->data.range.range + filter->data.range.start),
          filter->target->name);
      return OM_OK;
    case OM_AFL_MODE_LIST:
      mode_str = "list";
      break;
    case OM_AFL_MODE_DECOMPOSE:
      mode_str = "decompose";
      break;
    default:
      mode_str = "unknow";
      break;
  }

  snprintf(buff, buff_size, "\tfilter mode:\tthis --(%s)--> [%s]\r\n", mode_str,
           filter->target->name);

  return OM_OK;
}

om_status_t om_print_topic_message(om_topic_t* topic, char* buff,
                                   uint32_t buff_size) {
  char* buff4buff = (char*)om_malloc(buff_size);

  snprintf(buff, buff_size,
           "name: [%s]\r\n\t"
           "option:\t[ %s%s%s]\r\n\t"
           "buffer_size:[%x]\r\n\t"
           "suber:[%x] \tpuber:[%x] \tlink:[%x]\r\n",
           topic->name, STR_SELECT(topic->user_fun.filter, " filter_fun", ),
           STR_SELECT(topic->afl, "advanced_filter ", ),
           STR_SELECT(topic->virtual_mode, "virtual_mode ", "real "),
           (unsigned int)topic->msg.size,
           (unsigned int)om_msg_get_suber_num(topic),
           (unsigned int)om_msg_get_puber_num(topic),
           (unsigned int)om_msg_get_link_num(topic));

  om_list_head_t* pos;
  om_list_for_each(pos, &topic->suber) {
    om_suber_t* suber = om_list_entry(pos, om_suber_t, self);
    om_print_suber_message(suber, buff4buff, buff_size);
    strncat(buff, buff4buff, buff_size);
  }
  om_list_for_each(pos, &topic->link) {
    om_link_t* link = om_list_entry(pos, om_link_t, self);
    om_print_link_message(link, buff4buff, buff_size);
    strncat(buff, buff4buff, buff_size);
  }
  if (topic->afl) {
    snprintf(buff4buff, buff_size, "\tadvanced_filter:[%d]\r\n",
             (int)om_afl_get_num(topic->afl));
    strncat(buff, buff4buff, buff_size);
    om_list_for_each(pos, &((om_afl_t*)topic->afl)->filter) {
      om_filter_t* filter = om_list_entry(pos, om_filter_t, self);
      om_print_afl_message(filter, buff4buff, buff_size);
      strncat(buff, buff4buff, buff_size);
    }
  }

  om_free(buff4buff);

  return OM_OK;
}
