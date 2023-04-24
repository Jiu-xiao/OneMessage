#include "om_log.h"

#include "om_afl.h"
#include "om_fmt.h"
#include "om_msg.h"

#if OM_LOG_OUTPUT

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
static om_mutex_t om_log_mutex;
static om_topic_t om_log_topic;

om_status_t om_log_init() {
  om_core_topic_create_static(&om_log_topic, "om_log", sizeof(om_log_t));
  om_mutex_init(&om_log_mutex);
  om_mutex_unlock(&om_log_mutex);
  om_log_initd = true;

  return OM_OK;
}

inline om_topic_t* om_get_log_handle() { return &om_log_topic; }

static om_log_t log_buf;
static char fm_buf[OM_LOG_MAX_LEN];

om_status_t om_print_log(const char* name, om_log_level_t level, bool block,
                         bool in_isr, const char* format, ...) {
  if (!om_log_initd) return OM_ERROR_NOT_INIT;

  om_mutex_lock(&om_log_mutex);

  log_buf.level = level;

#if OM_TIME
  om_time_get(&log_buf.time);
#endif

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
  vsnprintf(log_buf.data, OM_LOG_MAX_LEN, fm_buf, vArgList);
  va_end(vArgList);
  om_status_t res =
      om_publish(&om_log_topic, &log_buf, sizeof(om_log_t), block, in_isr);

  om_mutex_unlock(&om_log_mutex);

  return res;
}

#endif
