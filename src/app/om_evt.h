#ifndef __OM_EVT_H__
#define __OM_EVT_H__

#include "om_core.h"

typedef om_topic_t* om_event_group_t;

typedef enum {
  OM_EVENT_START,
  OM_EVENT_PROGRESS,
  OM_EVENT_END,
} om_event_status_t;

typedef struct {
  om_user_fun_t callback;
  void* arg;
  uint32_t event;
  bool last;
  om_event_status_t status;
} om_event_t;

om_event_group_t om_event_create_group(char* name);

om_status_t om_event_register(om_event_group_t group, uint32_t event,
                              om_event_status_t status, om_user_fun_t fun,
                              void* arg);

om_status_t om_event_active(om_event_group_t group, uint32_t event, bool block,
                            bool in_isr);

#endif