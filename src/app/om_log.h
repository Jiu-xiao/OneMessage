#ifndef __OM_LOG_H__
#define __OM_LOG_H__

#include "om_core.h"

#if OM_LOG_OUTPUT

#if OM_LOG_LEVEL <= 1
#define OMLOG_DEFAULT(...) \
  om_print_log("Default", OM_LOG_LEVEL_DEFAULT, true, false, __VA_ARGS__)
#define OMLOG_DEFAULT_ISR(...) \
  om_print_log("Default", OM_LOG_LEVEL_DEFAULT, false, true, __VA_ARGS__)
#else
#define OMLOG_DEFAULT(...)
#define OMLOG_DEFAULT_ISR(...)
#endif

#if OM_LOG_LEVEL <= 2
#define OMLOG_NOTICE(...) \
  om_print_log("Notice", OM_LOG_LEVEL_NOTICE, true, false, __VA_ARGS__)
#define OMLOG_NOTICE_ISR(...) \
  om_print_log("Notice", OM_LOG_LEVEL_NOTICE, false, true, __VA_ARGS__)
#else
#define OMLOG_NOTICE(...)
#define OMLOG_NOTICE_ISR(...)
#endif

#if OM_LOG_LEVEL <= 3
#define OMLOG_PASS(...) \
  om_print_log("Pass", OM_LOG_LEVEL_PASS, true, false, __VA_ARGS__)
#define OMLOG_PASS_ISR(...) \
  om_print_log("Pass", OM_LOG_LEVEL_PASS, false, true, __VA_ARGS__)
#else
#define OMLOG_PASS(...)
#define OMLOG_PASS_ISR(...)
#endif

#if OM_LOG_LEVEL <= 4
#define OMLOG_WARNING(...) \
  om_print_log("Warning", OM_LOG_LEVEL_WARNING, true, false, __VA_ARGS__)
#define OMLOG_WARNING_ISR(...) \
  om_print_log("Warning", OM_LOG_LEVEL_WARNING, false, true, __VA_ARGS__)
#else
#define OMLOG_WARNING(...)
#define OMLOG_WARNING_ISR(...)
#endif

#if OM_LOG_LEVEL <= 5
#define OMLOG_ERROR(...) \
  om_print_log("Error", OM_LOG_LEVEL_ERROR, true, false, __VA_ARGS__)
#define OMLOG_ERROR_ISR(...) \
  om_print_log("Error", OM_LOG_LEVEL_ERROR, false, true, __VA_ARGS__)
#else
#define OMLOG_ERROR(...)
#define OMLOG_ERROR_ISR(...)
#endif

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

om_status_t om_print_log(const char* name, om_log_level_t level, bool block,
                         bool in_isr, const char* format, ...);

#endif

#endif
