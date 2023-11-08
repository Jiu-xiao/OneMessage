#include "om_core.h"
#include "om_crc.h"

RBT_ROOT(_topic_list);

static om_mutex_t core_lock;

om_status_t om_core_init() {
  om_mutex_init(&core_lock);
  om_mutex_unlock(&core_lock);
  om_generate_crc32_table();
  om_generate_crc8_table();
  return OM_OK;
}

om_topic_t* om_core_topic_create(const char* name, uint32_t buff_len) {
  om_topic_t* topic = om_malloc(sizeof(om_topic_t));

  return om_core_topic_create_static(topic, name, buff_len);
}

om_topic_t* om_core_topic_create_static(om_topic_t* topic, const char* name,
                                        uint32_t buff_len) {
  OM_ASSERT(topic);

  memset(topic, 0, sizeof(*topic));
  strncpy(topic->name, name, OM_TOPIC_MAX_NAME_LEN);

  topic->self.key = topic->name;
  topic->virtual_mode = true;
  topic->buff_len = buff_len;

  OM_INIT_LIST_HEAD(&topic->suber);
  OM_INIT_LIST_HEAD(&topic->link);

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

  om_mutex_lock(&core_lock);

  om_rbtree_insert(&_topic_list, &topic->self);

  om_mutex_unlock(&core_lock);

  return OM_OK;
}

om_suber_t* om_core_suber_create(om_topic_t* link) {
  om_suber_t* suber = om_malloc(sizeof(*suber));

  return om_core_suber_create_static(suber, link);
}

om_suber_t* om_core_suber_create_static(om_suber_t* suber, om_topic_t* link) {
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

  om_mutex_lock(&core_lock);

  om_list_add(&sub->self, &(topic->suber));

  om_mutex_unlock(&core_lock);

  return OM_OK;
}

om_link_t* om_core_link_create(om_suber_t* sub, om_topic_t* topic) {
  om_link_t* link = om_malloc(sizeof(*link));

  return om_core_link_create_static(link, sub, topic);
}

om_link_t* om_core_link_create_static(om_link_t* link, om_suber_t* sub,
                                      om_topic_t* topic) {
  OM_ASSERT(link);
  link->source.suber = sub;
  link->source.topic = topic;

  return link;
}

om_status_t om_core_add_link(om_topic_t* topic, om_link_t* link) {
  OM_ASSERT(topic);
  OM_ASSERT(link);

  om_mutex_lock(&core_lock);

  om_list_add(&link->self, &(topic->link));

  om_mutex_unlock(&core_lock);

  return OM_OK;
}

om_status_t om_core_queue_init_fifo_static(om_topic_t* topic, om_fifo_t* fifo,
                                           void* buff, uint32_t len) {
  OM_ASSERT(topic);
  OM_ASSERT(fifo);
  OM_ASSERT(buff);
  OM_ASSERT(len);

  om_fifo_create(fifo, buff, len, topic->buff_len);

  return OM_OK;
}

om_fifo_t* om_core_queue_add(om_topic_t* topic, uint32_t len) {
  om_fifo_t* fifo = om_malloc(sizeof(om_fifo_t));
  om_suber_t* suber = om_malloc(sizeof(om_suber_t));
  om_core_queue_init_fifo_static(topic, fifo, om_malloc(topic->buff_len * len),
                                 len);
  om_core_queue_add_static(topic, suber, fifo);

  return fifo;
}

om_fifo_t* om_core_queue_add_static(om_topic_t* topic, om_suber_t* sub,
                                    om_fifo_t* fifo) {
  om_core_suber_create_static(sub, NULL);
  sub->mode = OM_SUBER_MODE_FIFO;
  sub->data.as_queue.fifo = fifo;

  om_core_add_suber(topic, sub);

  return fifo;
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

om_status_t om_core_link_static(om_suber_t* suber, om_link_t* link,
                                om_topic_t* source, om_topic_t* target) {
  OM_ASSERT(source);
  OM_ASSERT(target);

  om_core_suber_create_static(suber, target);
  om_core_add_suber(source, suber);
  om_core_link_create_static(link, suber, source);
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

  om_mutex_lock(&core_lock);
  om_list_del(&sub->self);
  om_mutex_unlock(&core_lock);

  if (sub->mode == OM_SUBER_MODE_LINK) {
    OM_ASSERT(sub->master);
    om_list_head_t* pos;
    om_list_for_each(pos, &sub->data.as_link.target->link) {
      om_link_t* link = om_list_entry(pos, om_link_t, self);
      if (link->source.suber == sub) {
        om_mutex_lock(&core_lock);
        om_list_del(&link->self);
        om_mutex_unlock(&core_lock);
        om_free(link);
        break;
      }
    }
  } else if (sub->mode == OM_SUBER_MODE_FIFO) {
    om_free(sub->data.as_queue.fifo);
  }

  om_free(sub);

  return OM_OK;
}

om_status_t om_core_del_topic(om_rbt_node_t* node) {
  OM_ASSERT(node);
  om_topic_t* topic = om_container_of(node, om_topic_t, self);

  om_mutex_lock(&core_lock);
  om_rbtree_delete(&_topic_list, &topic->self);
  om_mutex_unlock(&core_lock);

  om_del_all(&topic->link, om_core_delink);
  om_del_all(&topic->suber, om_core_del_suber);
  if (!topic->virtual_mode && topic->msg.buff) om_free(topic->msg.buff);
  om_mutex_delete(&topic->mutex);
  om_free(topic);

  return OM_OK;
}

om_topic_t* om_core_find_topic(const char* name, uint32_t timeout) {
  om_rbt_node_t* node;
  do {
    om_mutex_lock(&core_lock);
    node = om_rbtree_search(&_topic_list, name);
    if (node != NULL) {
      om_mutex_unlock(&core_lock);
      return om_container_of(node, om_topic_t, self);
    }
    om_mutex_unlock(&core_lock);
    if (timeout) {
      om_delay_ms(1);
      timeout--;
    }
  } while (timeout);
  return NULL;
}

om_status_t om_core_set_export_target(om_suber_t* suber) {
  OM_ASSERT(suber);

  suber->mode = OM_SUBER_MODE_EXPORT;
  suber->data.as_export.new_data = false;
  return OM_OK;
}

void om_error(const char* file, uint32_t line) {
  (void)(file);
  (void)(line);
  while (1) {
  };
}
