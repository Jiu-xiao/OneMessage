#include "om_core.h"

#define om_topic_add_puber om_core_add_puber
#define om_topic_add_suber om_core_add_suber
#define om_add_topic om_core_add_topic
#define om_topic_link om_core_link
#define om_find_topic om_core_find_topic

typedef enum {
  OM_USER_FUN_FILTER,
  OM_USER_FUN_GET,
  OM_USER_FUN_DECODE,
  OM_USER_FUN_NEW,
  OM_USER_FUN_APPLY,
  OM_LINK,
  OM_ADD_SUBER,
  OM_ADD_PUBER,
  OM_PUB_FREQ,
  OM_CONFIG_END
} om_config_op_t;

typedef struct {
  om_config_op_t op;
  void* arg;
} om_config_t;

typedef struct {
  char data[OM_LOG_MAX_LEN];
  om_time_t time;
} om_log_t;

om_status_t om_init();

om_status_t om_config_topic(om_topic_t* topic, om_config_t* config);

om_status_t om_config_suber(om_suber_t* sub, om_config_t* config);

om_status_t om_config_puber(om_puber_t* pub, om_config_t* config);

om_status_t _om_publish(om_topic_t* topic, om_msg_t* msg);

om_status_t om_publish_with_name(const char* name, void* buff, size_t size);

om_status_t om_publish_with_handle(om_topic_t* topic, void* buff, size_t size);

om_status_t om_sync();

om_topic_t* om_create_topic(const char* name, om_config_t* config);

om_suber_t* om_create_suber(om_config_t* config);

om_puber_t* om_create_puber(om_config_t* config);

om_status_t om_deinit();

#if OM_LOG_OUTPUT
om_topic_t* om_get_log_handle();
om_status_t om_print_log(om_log_t* log, const char* format, ...);
#endif

extern om_config_t OM_EMPTY_CONFIG;