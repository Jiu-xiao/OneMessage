#include "om_log.h"

#include "om_afl.h"
#include "om_fmt.h"
#include "om_msg.h"

#define STR_SELECT(_bool, _str, _str1) ((_bool) ? "" _str : ""_str1)

#if OM_LOG_OUTPUT
static om_topic_t* om_log;

#if OM_LOG_COLORFUL
static const om_log_format_t LOG_FORMAT[OM_LOG_COLOR_NUMBER] = {
    {
        .bg_color = OM_COLOR_BACKGROUND_NONE,
        .ft_color = OM_COLOR_FONT_RED,
        .fm_color = OM_COLOR_FORMAT_NONE,
    },
    {
        .bg_color = OM_COLOR_BACKGROUND_NONE,
        .ft_color = OM_COLOR_FONT_GREEN,
        .fm_color = OM_COLOR_FORMAT_NONE,
    },
    {
        .bg_color = OM_COLOR_BACKGROUND_NONE,
        .ft_color = OM_COLOR_FONT_BLUE,
        .fm_color = OM_COLOR_FORMAT_NONE,
    },
    {
        .bg_color = OM_COLOR_BACKGROUND_NONE,
        .ft_color = OM_COLOR_FONT_YELLOW,
        .fm_color = OM_COLOR_FORMAT_NONE,
    },
    {
        .bg_color = OM_COLOR_BACKGROUND_NONE,
        .ft_color = OM_COLOR_FONT_NONE,
        .fm_color = OM_COLOR_FORMAT_NONE,
    },
};
#endif

static bool om_log_initd = false;

om_status_t om_log_init() {
  om_log = om_core_topic_create("om_log");
  om_core_add_topic(om_log);
  om_log_initd = true;

  return OM_OK;
}

inline om_topic_t* om_get_log_handle() { return om_log; }

om_status_t om_print_log(char* name, om_log_level_t level, bool block,
                         bool in_isr, const char* format, ...) {
  if (!om_log_initd) return OM_ERROR_NOT_INIT;

  om_log_t log;
  log.level = level;
  om_time_get(&log.time);
  char* fm_buf = om_malloc(OM_LOG_MAX_LEN);
#if OM_LOG_COLORFUL
  snprintf(fm_buf, OM_LOG_MAX_LEN, "%s%s%s[%s]%s%s\r\n",
           OM_COLOR_BG[LOG_FORMAT[level].bg_color],
           OM_COLOR_FONT[LOG_FORMAT[level].ft_color],
           OM_COLOR_FORMAT[LOG_FORMAT[level].fm_color], name,
           OM_COLOR_FORMAT[OM_COLOR_FORMAT_RESET], format);
#else
  snprintf(fm_buf, OM_LOG_MAX_LEN, "[%s]%s\r\n", name, format);
#endif
  va_list vArgList;
  va_start(vArgList, format);
  vsnprintf(log.data, OM_LOG_MAX_LEN, fm_buf, vArgList);
  va_end(vArgList);
  om_free(fm_buf);
  return om_publish(om_log, &log, sizeof(om_log_t), block, in_isr);
}

om_status_t om_print_suber_message(om_suber_t* suber, char* buff,
                                   uint32_t buff_size) {
  switch (suber->mode) {
    case OM_SUBER_MODE_DEPLOY:
      snprintf(buff, buff_size,
               "\tsuber mode:\t[%s] --[deploy]--> user_fun[%p]\r\n",
               suber->master->name, suber->data.as_deploy.deploy);
      break;
    case OM_SUBER_MODE_LINK:
      snprintf(buff, buff_size, "\t\tsuber mode:\t[%s] --[link]--> [%s]\r\n",
               suber->master->name, suber->data.as_link.target->name);
      break;
    case OM_SUBER_MODE_DUMP:
      snprintf(buff, buff_size,
               "\tsuber mode:\t[%s] --[dump]--> buffer:%d[%p]\r\n",
               suber->master->name, (int)suber->data.as_dump.max_size,
               suber->data.as_dump.buff);
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
           STR_SELECT(topic->virtual, "virtual ", "real "),
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

om_status_t om_log_deinit() {
  om_log_initd = false;
  return OM_OK;
}
#endif
