#include "om_msg.h"

#include "om_afl.h"
#include "om_fmt.h"
#include "om_log.h"

extern om_rbt_root_t _topic_list;

inline om_status_t _om_publish_to_suber(om_suber_t* sub, om_topic_t* topic,
                                        bool block, bool in_isr) {
  OM_ASSERT(sub);
  OM_ASSERT(topic);

  switch (sub->mode) {
    case OM_SUBER_MODE_DEFAULT:
      sub->data.as_suber.sub_callback(&topic->msg,
                                      sub->data.as_suber.sub_cb_arg);
      break;
    case OM_SUBER_MODE_LINK:
      if (!in_isr) {
        if (block)
          om_mutex_lock(&sub->data.as_link.target->mutex);
        else {
          if (om_mutex_trylock(&sub->data.as_link.target->mutex) != OM_OK)
            break;
        }

        _om_publish(sub->data.as_link.target, &topic->msg, block, in_isr);

        om_mutex_unlock(&sub->data.as_link.target->mutex);
      } else {
        if (om_mutex_lock_isr(&sub->data.as_link.target->mutex) != OM_OK)
          return OM_ERROR_BUSY;
        _om_publish(sub->data.as_link.target, &topic->msg, block, in_isr);
        om_mutex_unlock_isr(&sub->data.as_link.target->mutex);
      }
      break;
    case OM_SUBER_MODE_EXPORT:
      sub->data.as_export.new_data = true;
      break;
    case OM_SUBER_MODE_FIFO:
      om_fifo_write(sub->data.as_queue.fifo, topic->msg.buff);
      break;
    case OM_SUBER_MODE_UNKNOW:
      break;
    default:
      OM_ASSERT(false);
      return OM_ERROR;
  }

  return OM_OK;
}

inline om_status_t _om_publish_to_topic(om_topic_t* topic, om_msg_t* msg,
                                        bool block, bool in_isr) {
  OM_ASSERT(topic);
  OM_ASSERT(msg);

  OM_UNUSED(in_isr);
  OM_UNUSED(block);

  if (msg->size > topic->buff_len) {
    OM_ASSERT(false);
    return OM_ERROR;
  }

#if OM_TIME
  om_time_get(&msg->time);
#endif

  if (topic->user_fun.filter != NULL &&
      topic->user_fun.filter(msg, topic->user_fun.filter_arg) != OM_OK)
    return OM_ERROR;

  if (topic->virtual_mode)
    memcpy(&topic->msg, msg, sizeof(*msg));
  else {
    OM_ASSERT(topic->msg.buff);
    memcpy(topic->msg.buff, msg->buff, msg->size);
    topic->msg.size = msg->size;
#if OM_TIME
    topic->msg.time = msg->time;
#endif
  }

  return OM_OK;
}

om_status_t _om_publish(om_topic_t* topic, om_msg_t* msg, bool block,
                        bool in_isr) {
  OM_ASSERT(topic);
  OM_ASSERT(msg);

  if (_om_publish_to_topic(topic, msg, block, in_isr) != OM_OK) return OM_ERROR;

  if (topic->afl) om_afl_apply(&(topic->msg), topic->afl, block, in_isr);

  om_list_head_t* pos;
  om_list_for_each(pos, &topic->suber) {
    om_suber_t* sub = om_list_entry(pos, om_suber_t, self);

    _om_publish_to_suber(sub, topic, block, in_isr);
  }

  return OM_OK;
}

om_status_t om_publish(om_topic_t* topic, void* buff, uint32_t size, bool block,
                       bool in_isr) {
  OM_ASSERT(topic);
  OM_ASSERT(buff);

  if (!in_isr) {
    if (block)
      om_mutex_lock(&topic->mutex);
    else if (om_mutex_trylock(&topic->mutex) != OM_OK)
      return OM_ERROR_BUSY;
  } else {
    if (om_mutex_lock_isr(&topic->mutex) != OM_OK) {
      return OM_ERROR_BUSY;
    }
  }

  om_msg_t msg = {.buff = buff, .size = size};

  om_status_t res = _om_publish(topic, &msg, block, in_isr);

  if (!in_isr) {
    om_mutex_unlock(&topic->mutex);
  } else {
    om_mutex_unlock_isr(&topic->mutex);
  }

  return res;
}

om_suber_t* om_subscribe(om_topic_t* topic) {
  OM_ASSERT(topic);

  om_suber_t* sub = om_core_suber_create(NULL);
  om_core_set_export_target(sub);
  om_core_add_suber(topic, sub);

  return sub;
}

om_suber_t* om_subscribe_static(om_topic_t* topic, om_suber_t* sub) {
  OM_ASSERT(topic);
  OM_ASSERT(sub);

  om_core_set_export_target(sub);
  om_core_add_suber(topic, sub);

  return sub;
}

inline bool om_suber_available(om_suber_t* suber) {
  return suber->data.as_export.new_data;
}

om_status_t om_suber_export(om_suber_t* suber, void* buff, bool in_isr) {
  OM_ASSERT(suber);
  OM_ASSERT(suber->mode == OM_SUBER_MODE_EXPORT);

  if (suber->data.as_export.new_data) {
    if (!in_isr)
      om_mutex_lock(&suber->master->mutex);
    else {
      if (om_mutex_lock_isr(&suber->master->mutex) != OM_OK)
        return OM_ERROR_BUSY;
    }

    suber->data.as_export.new_data = false;

    memcpy(buff, suber->master->msg.buff, suber->master->msg.size);

    if (!in_isr)
      om_mutex_unlock(&suber->master->mutex);
    else
      om_mutex_unlock_isr(&suber->master->mutex);

    return OM_OK;
  } else {
    return OM_ERROR;
  }
}

om_status_t om_msg_del_topic(om_topic_t* topic) {
  OM_ASSERT(topic);

  return om_core_del_topic(&topic->self);
}

om_status_t om_msg_del_suber(om_suber_t* suber) {
  OM_ASSERT(suber);

  return om_core_del_suber(&suber->self);
}

typedef struct {
  bool (*fun)(om_topic_t* topic, void* arg);
  void* arg;
} om_msg_cb_block;

static bool _om_msg_foreach_topic(om_rbt_node_t* node, void* arg) {
  om_msg_cb_block* block = (om_msg_cb_block*)arg;
  om_topic_t* topic = om_container_of(node, om_topic_t, self);

  return block->fun(topic, block->arg);
}

om_status_t om_msg_foreach_topic(bool (*fun)(om_topic_t* topic, void* arg),
                                 void* arg) {
  om_msg_cb_block block = {fun, arg};

  om_rbtree_foreach(&_topic_list, _om_msg_foreach_topic, &block);

  return OM_OK;
}

uint32_t om_msg_get_topic_num() { return om_rbtree_get_num(&_topic_list); }

uint32_t om_msg_get_suber_num(om_topic_t* topic) {
  return om_list_get_num(&topic->suber);
}

uint32_t om_msg_get_link_num(om_topic_t* topic) {
  return om_list_get_num(&topic->link);
}

#if OM_TIME
om_time_t om_msg_get_last_time(om_topic_t* topic) { return topic->msg.time; }
#endif
