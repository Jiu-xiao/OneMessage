#ifndef __OM_EVT_H__
#define __OM_EVT_H__

#include "om_core.h"

typedef om_topic_t om_event_group_t;

typedef enum {
  OM_EVENT_START,
  OM_EVENT_PROGRESS,
  OM_EVENT_END,
} om_event_status_t;

typedef struct {
  void (*callback)(uint32_t event, void* arg);
  void* arg;
  uint32_t event;
  bool last;
  om_event_status_t status;
} om_event_t;

om_event_group_t* om_event_create_group(const char* name);

om_event_group_t* om_event_create_group_static(om_event_group_t* group,
                                               const char* name);

om_event_group_t* om_event_find_group(const char* name, uint32_t timeout);

om_status_t om_event_register(om_event_group_t* group, uint32_t event,
                              om_event_status_t status,
                              void (*callback)(uint32_t event, void* arg),
                              void* arg);

om_status_t om_event_register_static(
    om_event_t* handle, om_event_group_t* group, uint32_t event,
    om_event_status_t status, void (*callback)(uint32_t event, void* arg),
    void* arg);

om_status_t om_event_active(om_event_group_t* group, uint32_t event, bool block,
                            bool in_isr);

#endif
