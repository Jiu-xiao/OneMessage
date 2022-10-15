#include "om_evt.h"

#include "om_afl.h"
#include "om_fmt.h"
#include "om_log.h"
#include "om_msg.h"

static om_status_t om_event_check(om_msg_t* msg, void* arg) {
  om_event_t* evt = (om_event_t*)arg;

  bool active = *((uint32_t*)msg->buff) == evt->event;

  switch (evt->status) {
    case OM_EVENT_START:
      if (!evt->last && active) evt->callback(evt->event, evt->arg);
      break;
    case OM_EVENT_PROGRESS:
      if (active) evt->callback(evt->event, evt->arg);
      break;
    case OM_EVENT_END:
      if (evt->last && !active) evt->callback(evt->event, evt->arg);
      break;
  }

  evt->last = active;

  return OM_OK;
}

om_event_group_t* om_event_create_group(const char* name) {
  om_event_group_t* group = om_config_topic(NULL, "A", name, sizeof(uint32_t));
  return group;
}

om_event_group_t* om_event_create_group_static(om_event_group_t* group,
                                               const char* name) {
  om_create_topic_static(group, name, sizeof(uint32_t));
  om_config_topic(group, "A");
  return group;
}

om_event_group_t* om_event_find_group(const char* name, uint32_t timeout) {
  return om_find_topic(name, timeout);
}

om_status_t om_event_register(om_event_group_t* group, uint32_t event,
                              om_event_status_t status,
                              void (*callback)(uint32_t event, void* arg),
                              void* arg) {
  om_event_t* handle = (om_event_t*)om_malloc(sizeof(om_event_t));

  handle->event = event;
  handle->callback = callback;
  handle->arg = arg;
  handle->status = status;

  om_config_topic(group, "D", om_event_check, handle);
  return OM_OK;
}

om_status_t om_event_register_static(
    om_event_t* handle, om_event_group_t* group, uint32_t event,
    om_event_status_t status, void (*callback)(uint32_t event, void* arg),
    void* arg) {
  handle->event = event;
  handle->callback = callback;
  handle->arg = arg;
  handle->status = status;

  om_config_topic(group, "D", om_event_check, handle);
  return OM_OK;
}

om_status_t om_event_active(om_event_group_t* group, uint32_t event, bool block,
                            bool in_isr) {
  return om_publish(group, &event, sizeof(event), block, in_isr);
}
