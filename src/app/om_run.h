#ifndef __OM_RUN_H__
#define __OM_RUN_H__

#include "om_core.h"

#if OM_REPORT_ACTIVITY

typedef enum {
  OM_ACTIVITY_PUBLISH,
  OM_ACTIVITY_SUBSCRIBE,
  OM_ACTIVITY_FILTER,
  OM_ACTIVITY_LINK,
  OM_ACTIVITY_EXPORT
} om_activity_t;

typedef struct {
  uint32_t id_activity;
  uint32_t time;
} om_report_t;

om_status_t om_run_init();

om_status_t om_run_add_report(om_activity_t activity, uint32_t id, bool in_isr);

om_status_t om_send_report_data();

#endif

om_status_t om_print_topic_message(om_topic_t* topic, char* buff,
                                   uint32_t buff_size);

#endif
