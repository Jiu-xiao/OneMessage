#include "om_core.h"

#if OM_LOG_OUTPUT
typedef enum {
  OM_LOG_COLOR_RED,
  OM_LOG_COLOR_GREEN,
  OM_LOG_COLOR_BLUE,
  OM_LOG_COLOR_YELLOW,
  OM_LOG_COLOR_DEFAULT,
  OM_LOG_COLOR_NUMBER
} om_log_color_t;

typedef enum {
  OM_LOG_DEFAULT = OM_LOG_COLOR_DEFAULT,
  OM_LOG_WARNING = OM_LOG_COLOR_YELLOW,
  OM_LOG_ERROR = OM_LOG_COLOR_RED,
  OM_LOG_PASS = OM_LOG_COLOR_GREEN,
  OM_LOG_NOTICE = OM_LOG_COLOR_BLUE
} om_log_level_t;

typedef struct {
  char data[OM_LOG_MAX_LEN];
  om_time_t time;
  om_log_level_t level;
} om_log_t;

om_status_t om_log_init();

om_topic_t *om_get_log_handle();

om_status_t om_print_log(char *name, om_log_level_t level, const char *format,
                         ...);

om_status_t om_log_deinit();
#endif