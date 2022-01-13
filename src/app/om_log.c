#include "om_log.h"

#include "om_msg.h"

static om_topic_t *om_log;

static bool om_log_initd = false;

static const char *color_tab[OM_LOG_COLOR_NUMBER][3] = {
    {"\033[31m", "\033[0m", "Error"},
    {"\033[32m", "\033[0m", "Pass"},
    {"\033[34m", "\033[0m", "Notice"},
    {"\033[33m", "\033[0m", "Waring"},
    {"\0", "\0", "Default"}};

om_status_t om_log_init() {
  om_log = om_core_topic_create("om_log");
  om_core_add_topic(om_log);
  om_log_initd = true;

  return OM_OK;
}

inline om_topic_t *om_get_log_handle() { return om_log; }

om_status_t om_print_log(char *name, om_log_level_t level, const char *format,
                         ...) {
  if (!om_log_initd) return OM_ERROR_NOT_INIT;

  om_log_t log;
  log.level = level;
  om_time_get(&log.time);
  char fm_buf[OM_LOG_MAX_LEN];
#if OM_LOG_COLORFUL
  snprintf(fm_buf, OM_LOG_MAX_LEN, "\033[0m%s[%s][%s]%s%s\n",
           color_tab[level][0], color_tab[level][2], name, format,
           color_tab[level][1]);
#else
  snprintf(fm_buf, OM_LOG_MAX_LEN, "[%s][%s]%s\n", color_tab[level][2], name,
           format);
#endif
  va_list vArgList;
  va_start(vArgList, format);
  vsnprintf(log.data, OM_LOG_MAX_LEN, fm_buf, vArgList);
  va_end(vArgList);
  return om_publish(om_log, &log, sizeof(om_log_t), true);
}

om_status_t om_log_deinit() {
  om_log_initd = false;
  return OM_OK;
}