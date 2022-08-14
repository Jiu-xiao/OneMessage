#include "om_evt.h"

#include "om_afl.h"
#include "om_fmt.h"
#include "om_log.h"
#include "om_msg.h"
#include "om_run.h"

static om_status_t om_event_check(om_msg_t* msg, void* arg) {
  om_event_t* evt = (om_event_t*)arg;
  if ((*((uint32_t*)msg->buff) & evt->event) == evt->event)
    evt->callback(msg, evt->arg);
  return OM_OK;
}

om_event_group_t om_event_create_group(char* name) {
  om_event_group_t group = om_config_topic(NULL, "VA", name);
  return group;
}

om_status_t om_event_register(om_event_group_t group, uint32_t event,
                              om_user_fun_t fun, void* arg) {
  OM_ASSERT(event < 32);

  om_event_t* evt = (om_event_t*)om_malloc(sizeof(om_event_t));

  evt->event = event;
  evt->callback = fun;
  evt->arg = arg;

  om_config_topic(group, "D", om_event_check, evt);
  return OM_OK;
}

om_status_t om_event_active(om_event_group_t group, uint32_t event, bool block,
                            bool in_isr) {
  om_publish(group, &event, sizeof(event), block, in_isr);
}
