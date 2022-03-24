#include "om_core.h"
#include "om_def.h"
#include "om_msg.h"

extern om_list_head_t topic_list;

om_mutex_t om_mutex_handle;

static bool om_msg_initd = false;

om_status_t om_msg_init() {
  om_mutex_init(&om_mutex_handle);
  om_mutex_unlock(&om_mutex_handle);

  om_msg_initd = true;

  return OM_OK;
}

inline om_status_t _om_publish_to_suber(om_suber_t* sub, om_topic_t* topic,
                                        bool block, bool in_isr) {
  OM_ASSERT(sub);
  OM_ASSERT(topic);

  if (sub->isLink) {
    if (!in_isr) {
      om_mutex_lock(&sub->target->mutex);
      _om_publish(sub->target, &topic->msg, block, in_isr);
      om_mutex_unlock(&sub->target->mutex);
    } else {
      if (om_mutex_lock_isr(&sub->target->mutex) != OM_OK) return OM_ERROR_BUSY;
      _om_publish(sub->target, &topic->msg, block, in_isr);
      om_mutex_unlock_isr(&sub->target->mutex);
    }

    return OM_OK;
  }

  if (sub->user_fun.filter != NULL &&
      sub->user_fun.filter(&topic->msg, sub->user_fun.filter_arg) != OM_OK)
    return OM_ERROR;

  if (sub->dump_target.enable) sub->dump_target.new = true;
  OM_ASSERT(topic->msg.buff);

  if (sub->user_fun.deploy)
    sub->user_fun.deploy(&topic->msg, sub->user_fun.deploy_arg);

  return OM_OK;
}

inline om_status_t _om_publish_to_topic(om_topic_t* topic, om_msg_t* msg,
                                        bool block, bool in_isr) {
  OM_ASSERT(topic);
  OM_ASSERT(msg);

  OM_UNUSED(in_isr);
  OM_UNUSED(block);

  om_time_get(&msg->time);

  if (topic->user_fun.filter != NULL &&
      topic->user_fun.filter(msg, topic->user_fun.filter_arg) != OM_OK)
    return OM_ERROR;

  if (topic->virtual)
    memcpy(&topic->msg, msg, sizeof(*msg));
  else {
    if (topic->msg.buff != NULL) om_free(topic->msg.buff);
    topic->msg.buff = om_malloc(msg->size);
    OM_ASSERT(topic->msg.buff);
    memcpy(topic->msg.buff, msg->buff, msg->size);
    topic->msg.size = msg->size;
    topic->msg.time = msg->time;
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

  if (!om_msg_initd) return OM_ERROR_NOT_INIT;

  if (!in_isr) {
    if (block)
      om_mutex_lock(&topic->mutex);
    else if (om_mutex_trylock(&topic->mutex) != OM_OK)
      return OM_ERROR_BUSY;
  } else {
    if (om_mutex_lock_isr(&topic->mutex) != OM_OK) return OM_ERROR_BUSY;
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

inline om_status_t _om_refresh_puber(om_puber_t* pub, om_topic_t* topic,
                                     bool block, bool in_isr) {
  OM_ASSERT(pub->user_fun.get_message);
  OM_ASSERT(pub->user_fun.new_message);

  pub->freq.counter--;
  if (pub->freq.counter > 0) return OM_ERROR;

  pub->freq.counter += pub->freq.reload;

  if (pub->user_fun.new_message(&pub->msg_buff, pub->user_fun.new_arg) !=
          OM_OK ||
      pub->user_fun.get_message(&pub->msg_buff, pub->user_fun.get_arg) != OM_OK)
    return OM_ERROR;

  om_status_t res = OM_OK;

  if (in_isr) {
    res = om_mutex_lock_isr(&topic->mutex);
  } else {
    res = om_mutex_lock(&topic->mutex);
  }

  if (res != OM_OK) return OM_ERROR;

  _om_publish(topic, &pub->msg_buff, block, in_isr);

  if (in_isr) {
    om_mutex_unlock_isr(&topic->mutex);
  } else {
    om_mutex_unlock(&topic->mutex);
  }

  return OM_OK;
}

om_status_t om_sync(bool in_isr) {
#if OM_VIRTUAL_TIME
  om_time_update(_om_time_handle);
#endif

  if (!om_msg_initd) return OM_ERROR_NOT_INIT;

  if (!in_isr) {
    om_mutex_lock(&om_mutex_handle);
  } else {
    om_mutex_lock_isr(&om_mutex_handle);
  }

  om_list_head_t *pos1, *pos2;
  om_list_for_each(pos1, &topic_list) {
    om_topic_t* topic = om_list_entry(pos1, om_topic_t, self);
    om_list_for_each(pos2, &topic->puber) {
      om_puber_t* pub = om_list_entry(pos2, om_puber_t, self);
      _om_refresh_puber(pub, topic, true, in_isr);
    }
  }

  if (!in_isr) {
    om_mutex_unlock(&om_mutex_handle);
  } else {
    om_mutex_unlock_isr(&om_mutex_handle);
  }

  return OM_OK;
}

om_suber_t* om_subscript(om_topic_t* topic, void* buff, uint32_t max_size,
                         om_user_fun_t filter) {
  OM_ASSERT(topic);
  OM_ASSERT(buff);

  om_suber_t* sub = om_core_suber_create(NULL);
  om_core_set_dump_target(sub, buff, max_size);
  om_core_add_suber(topic, sub);
  sub->user_fun.filter = filter;
  sub->target = topic;

  return sub;
}

om_status_t om_suber_dump(om_suber_t* suber, bool in_isr) {
  OM_ASSERT(suber);
  OM_ASSERT(suber->dump_target.enable);

#if OM_STRICT_LIMIT
  bool data_correct = suber->target->msg.size == suber->dump_target.max_size;
  OM_ASSERT(!suber->dump_target.new || data_correct);
#else
  bool data_correct = suber->target->msg.size <= suber->dump_target.max_size;
#endif

  if (suber->dump_target.new&& data_correct) {
    if (!in_isr)
      om_mutex_lock(&suber->target->mutex);
    else
      om_mutex_lock_isr(&suber->target->mutex);

    suber->dump_target.new = false;

    memcpy(suber->dump_target.address, suber->target->msg.buff,
           suber->target->msg.size);

    if (!in_isr)
      om_mutex_unlock(&suber->target->mutex);
    else
      om_mutex_unlock_isr(&suber->target->mutex);

    return OM_OK;
  } else {
    return OM_ERROR;
  }
}

om_status_t om_msg_deinit() {
  om_msg_initd = false;

  om_del_all(&topic_list, om_core_del_topic);

  return OM_OK;
}

om_status_t om_msg_del_topic(om_topic_t* topic) {
  OM_ASSERT(topic);

  return om_core_del_topic(&topic->self);
}

om_status_t om_msg_del_suber(om_suber_t* suber) {
  OM_ASSERT(suber);

  return om_core_del_suber(&suber->self);
}

om_status_t om_msg_del_puber(om_puber_t* puber) {
  OM_ASSERT(puber);

  return om_core_del_puber(&puber->self);
}

uint16_t om_msg_get_topic_num() { return om_list_get_num(&topic_list); }

uint16_t om_msg_get_suber_num(om_topic_t* topic) {
  return om_list_get_num(&topic->suber);
}

uint16_t om_msg_get_puber_num(om_topic_t* topic) {
  return om_list_get_num(&topic->puber);
}

om_status_t om_msg_for_each(om_status_t (*fun)(om_topic_t*)) {
  OM_ASSERT(fun);

  om_list_head_t* pos;

  om_list_for_each(pos, &topic_list) {
    om_topic_t* topic = om_list_entry(pos, om_topic_t, self);
    if (fun(topic) != OM_OK) return OM_ERROR;
  }

  return OM_OK;
}
