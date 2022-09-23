#include "om_log.h"

#include "om_afl.h"
#include "om_fmt.h"
#include "om_msg.h"

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

om_status_t om_log_deinit() {
  om_core_del_topic(&(om_log->self));
  om_log_initd = false;
  return OM_OK;
}
#endif
