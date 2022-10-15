#ifndef __OM_LOG_H__
#define __OM_LOG_H__

#include "om_core.h"

#if OM_LOG_OUTPUT
#define OMLOG_DEFAULT(format, args...) \
  om_print_log("Default", OM_LOG_LEVEL_DEFAULT, true, false, format, ##args)
#define OMLOG_WARNING(format, args...) \
  om_print_log("Warning", OM_LOG_LEVEL_WARNING, true, false, format, ##args)
#define OMLOG_ERROR(format, args...) \
  om_print_log("Error", OM_LOG_LEVEL_ERROR, true, false, format, ##args)
#define OMLOG_PASS(format, args...) \
  om_print_log("Pass", OM_LOG_LEVEL_PASS, true, false, format, ##args)
#define OMLOG_NOTICE(format, args...) \
  om_print_log("Notice", OM_LOG_LEVEL_NOTICE, true, false, format, ##args)

#define OMLOG_DEFAULT_ISR(format, args...) \
  om_print_log("Default", OM_LOG_LEVEL_DEFAULT, false, true, format, ##args)
#define OMLOG_WARNING_ISR(format, args...) \
  om_print_log("Warning", OM_LOG_LEVEL_WARNING, false, true, format, ##args)
#define OMLOG_ERROR_ISR(format, args...) \
  om_print_log("Error", OM_LOG_LEVEL_ERROR, false, true, format, ##args)
#define OMLOG_PASS_ISR(format, args...) \
  om_print_log("Pass", OM_LOG_LEVEL_PASS, false, true, format, ##args)
#define OMLOG_NOTICE_ISR(format, args...) \
  om_print_log("Notice", OM_LOG_LEVEL_NOTICE, false, true, format, ##args)

typedef enum {
  OM_LOG_COLOR_RED,
  OM_LOG_COLOR_GREEN,
  OM_LOG_COLOR_BLUE,
  OM_LOG_COLOR_YELLOW,
  OM_LOG_COLOR_DEFAULT,
  OM_LOG_COLOR_NUMBER
} om_log_color_t;

typedef enum {
  OM_LOG_LEVEL_DEFAULT = OM_LOG_COLOR_DEFAULT,
  OM_LOG_LEVEL_WARNING = OM_LOG_COLOR_YELLOW,
  OM_LOG_LEVEL_ERROR = OM_LOG_COLOR_RED,
  OM_LOG_LEVEL_PASS = OM_LOG_COLOR_GREEN,
  OM_LOG_LEVEL_NOTICE = OM_LOG_COLOR_BLUE
} om_log_level_t;

typedef struct {
  char data[OM_LOG_MAX_LEN];
#if OM_TIME
  om_time_t time;
#endif
  om_log_level_t level;
} om_log_t;

typedef struct {
  om_color_background_t bg_color;
  om_color_font_t ft_color;
  om_color_format_t fm_color;
} om_log_format_t;

om_status_t om_log_init();

om_topic_t* om_get_log_handle();

om_status_t om_print_log(char* name, om_log_level_t level, bool block,
                         bool in_isr, const char* format, ...);

#endif

#endif
