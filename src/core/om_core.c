#include "om_core.h"

uint32_t _om_time_handle;

#if OM_REPORT_ACTIVITY
static uint32_t om_topic_num;
#endif

LIST_HEAD(om_topic_list);

om_status_t om_core_init() {
#if OM_REPORT_ACTIVITY
  om_topic_num = 0;
#endif

  return OM_OK;
}

om_topic_t* om_core_topic_create(const char* name) {
  om_topic_t* topic = om_malloc(sizeof(om_topic_t));
  OM_ASSERT(topic);

  memset(topic, 0, sizeof(*topic));
  strncpy(topic->name, name, OM_TOPIC_MAX_NAME_LEN);

  INIT_LIST_HEAD(&topic->puber);
  INIT_LIST_HEAD(&topic->suber);
  INIT_LIST_HEAD(&topic->link);

  om_mutex_init(&topic->mutex);
  om_mutex_unlock(&topic->mutex);

  return topic;
}

om_status_t om_core_add_topic(om_topic_t* topic) {
  OM_ASSERT(topic);

  if (om_core_find_topic(topic->name, 0) != NULL) {
    OM_ASSERT(false);
    return OM_ERROR;
  }

#if OM_REPORT_ACTIVITY
  topic->id = om_topic_num++;
#endif

  om_list_add_tail(&topic->self, &om_topic_list);

  return OM_OK;
}

om_suber_t* om_core_suber_create(om_topic_t* link) {
  om_suber_t* suber = om_malloc(sizeof(*suber));
  OM_ASSERT(suber);
  memset(suber, 0, sizeof(*suber));
  if (link) {
    suber->mode = OM_SUBER_MODE_LINK;
    suber->data.as_link.target = link;
  }
  return suber;
}

om_status_t om_core_add_suber(om_topic_t* topic, om_suber_t* sub) {
  OM_ASSERT(topic);
  OM_ASSERT(sub);

  sub->master = topic;

  om_list_add_tail(&sub->self, &(topic->suber));

  return OM_OK;
}

om_link_t* om_core_link_create(om_suber_t* sub, om_topic_t* topic) {
  om_link_t* link = om_malloc(sizeof(*link));
  OM_ASSERT(link);
  link->source.suber = sub;
  link->source.topic = topic;

  return link;
}

om_status_t om_core_add_link(om_topic_t* topic, om_link_t* link) {
  OM_ASSERT(topic);
  OM_ASSERT(link);

  om_list_add_tail(&link->self, &(topic->link));

  return OM_OK;
}

om_status_t om_core_link(om_topic_t* source, om_topic_t* target) {
  OM_ASSERT(source);
  OM_ASSERT(target);

  om_suber_t* sub = om_core_suber_create(target);
  om_core_add_suber(source, sub);
  om_link_t* link = om_core_link_create(sub, source);
  om_core_add_link(target, link);

  return OM_OK;
}

om_status_t om_core_delink(om_list_head_t* head) {
  OM_ASSERT(head);
  om_link_t* link = om_list_entry(head, om_link_t, self);

  om_list_del(&link->self);
  om_list_del(&link->source.suber->self);

  om_free(link->source.suber);
  om_free(link);

  return OM_OK;
}

om_status_t om_core_del_suber(om_list_head_t* head) {
  OM_ASSERT(head);
  om_suber_t* sub = om_list_entry(head, om_suber_t, self);

  om_list_del(&sub->self);
  if (sub->mode == OM_SUBER_MODE_LINK) {
    OM_ASSERT(sub->master);
    om_list_head_t* pos;
    om_list_for_each(pos, &sub->data.as_link.target->link) {
      om_link_t* link = om_list_entry(pos, om_link_t, self);
      if (link->source.suber == sub) {
        om_list_del(&link->self);
        om_free(link);
        break;
      }
    }
  }

  om_free(sub);

  return OM_OK;
}

om_status_t om_core_del_topic(om_list_head_t* head) {
  OM_ASSERT(head);
  om_topic_t* topic = om_list_entry(head, om_topic_t, self);
  OM_CHECK((void*)topic != (void*)&om_topic_list);

  om_list_del(&topic->self);
  om_del_all(&topic->link, om_core_delink);
  om_del_all(&topic->suber, om_core_del_suber);
  if (!topic->virtual_mode && topic->msg.buff) om_free(topic->msg.buff);
  om_free(topic);

  return OM_OK;
}

om_topic_t* om_core_find_topic(const char* name, uint32_t timeout) {
  om_list_head_t* pos;
  do {
    om_list_for_each(pos, &om_topic_list) {
      om_topic_t* topic = om_list_entry(pos, om_topic_t, self);
      if (!strncmp(name, topic->name, OM_TOPIC_MAX_NAME_LEN)) return topic;
    }
    if (timeout) {
      om_delay_ms(1);
      timeout--;
    }
  } while (timeout);
  return NULL;
}

om_status_t om_core_set_export_target(om_suber_t* suber, void* target,
                                      uint32_t max_size) {
  OM_ASSERT(target);
  OM_ASSERT(suber);

  suber->mode = OM_SUBER_MODE_EXPORT;
  suber->data.as_export.max_size = max_size;
  suber->data.as_export.buff = target;
  return OM_OK;
}

inline uint32_t om_core_get_time() { return _om_time_handle; }

void om_error(const char* file, uint32_t line) {
  (void)(file);
  (void)(line);
  while (1) {
  };
}
