#include "om_msg.h"

#include "om_afl.h"
#include "om_fmt.h"
#include "om_log.h"
#include "om_run.h"

extern om_list_head_t om_topic_list;

extern om_mutex_t om_mutex_handle;

static bool om_msg_initd = false;

om_status_t om_msg_init() {
  om_msg_initd = true;

  return OM_OK;
}

inline om_status_t _om_publish_to_suber(om_suber_t* sub, om_topic_t* topic,
                                        bool block, bool in_isr) {
  OM_ASSERT(sub);
  OM_ASSERT(topic);

  switch (sub->mode) {
    case OM_SUBER_MODE_DEFAULT:
      sub->data.as_suber.sub_callback(&topic->msg,
                                      sub->data.as_suber.sub_cb_arg);
#if OM_REPORT_ACTIVITY
      om_run_add_report(OM_ACTIVITY_SUBSCRIBE, topic->id, in_isr);
#endif
      break;
    case OM_SUBER_MODE_LINK:
      if (!in_isr) {
        if (block)
          om_mutex_lock(&sub->data.as_link.target->mutex);
        else {
          if (om_mutex_trylock(&sub->data.as_link.target->mutex) != OM_OK)
            break;
        }
#if OM_REPORT_ACTIVITY
        om_run_add_report(OM_ACTIVITY_LINK, topic->id, in_isr);
#endif
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

  om_time_get(&msg->time);

  if (topic->user_fun.filter != NULL &&
      topic->user_fun.filter(msg, topic->user_fun.filter_arg) != OM_OK)
    return OM_ERROR;

  if (topic->virtual_mode)
    memcpy(&topic->msg, msg, sizeof(*msg));
  else {
    if (topic->msg.buff != NULL) om_free(topic->msg.buff);
    topic->msg.buff = om_malloc(msg->size);
    OM_ASSERT(topic->msg.buff);
    memcpy(topic->msg.buff, msg->buff, msg->size);
    topic->msg.size = msg->size;
    topic->msg.time = msg->time;
  }

#if OM_REPORT_ACTIVITY
  om_run_add_report(OM_ACTIVITY_PUBLISH, topic->id, in_isr);
#endif

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

inline om_status_t _om_refresh_puber(om_puber_t* pub, om_topic_t* topic,
                                     bool block, bool in_isr) {
  OM_ASSERT(pub->user_fun.refresh_callback);

  pub->freq.counter--;
  if (pub->freq.counter > 0) return OM_ERROR;

  pub->freq.counter += pub->freq.reload;

  if (pub->user_fun.refresh_callback(&pub->msg_buff,
                                     pub->user_fun.refresh_cb_arg) != OM_OK)
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
  _om_time_handle++;

  if (!om_msg_initd) return OM_ERROR_NOT_INIT;

  if (!in_isr) {
    om_mutex_lock(&om_mutex_handle);
  } else {
    if (om_mutex_lock_isr(&om_mutex_handle) != OM_OK) return OM_ERROR_BUSY;
  }

  om_list_head_t *pos1, *pos2;
  om_list_for_each(pos1, &om_topic_list) {
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

om_suber_t* om_subscript(om_topic_t* topic, void* buff, uint32_t max_size) {
  OM_ASSERT(topic);
  OM_ASSERT(buff);

  om_suber_t* sub = om_core_suber_create(NULL);
  om_core_set_export_target(sub, buff, max_size);
  om_core_add_suber(topic, sub);

  return sub;
}

om_status_t om_suber_export(om_suber_t* suber, bool in_isr) {
  OM_ASSERT(suber);
  OM_ASSERT(suber->mode == OM_SUBER_MODE_EXPORT);

#if OM_STRICT_LIMIT
  bool data_correct = suber->master->msg.size == suber->data.as_export.max_size;
  OM_ASSERT(!suber->data.as_export.new_data || data_correct);
#else
  bool data_correct = suber->master->msg.size <= suber->data.as_export.max_size;
#endif

  if (suber->data.as_export.new_data && data_correct) {
    if (!in_isr)
      om_mutex_lock(&suber->master->mutex);
    else {
      if (om_mutex_lock_isr(&suber->master->mutex) != OM_OK)
        return OM_ERROR_BUSY;
    }

    suber->data.as_export.new_data = false;

    memcpy(suber->data.as_export.buff, suber->master->msg.buff,
           suber->master->msg.size);

    if (!in_isr)
      om_mutex_unlock(&suber->master->mutex);
    else
      om_mutex_unlock_isr(&suber->master->mutex);

#if OM_REPORT_ACTIVITY
    om_run_add_report(OM_ACTIVITY_EXPORT, suber->master->id, in_isr);
#endif

    return OM_OK;
  } else {
    return OM_ERROR;
  }
}

om_status_t om_msg_deinit() {
  om_msg_initd = false;

  om_del_all(&om_topic_list, om_core_del_topic);

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

uint32_t om_msg_get_topic_num() { return om_list_get_num(&om_topic_list); }

uint32_t om_msg_get_suber_num(om_topic_t* topic) {
  return om_list_get_num(&topic->suber);
}

uint32_t om_msg_get_puber_num(om_topic_t* topic) {
  return om_list_get_num(&topic->puber);
}

uint32_t om_msg_get_link_num(om_topic_t* topic) {
  return om_list_get_num(&topic->link);
}

om_status_t om_msg_for_each(om_status_t (*fun)(om_topic_t*, void* arg),
                            void* arg) {
  OM_ASSERT(fun);

  om_list_head_t* pos;

  om_list_for_each(pos, &om_topic_list) {
    om_topic_t* topic = om_list_entry(pos, om_topic_t, self);
    if (fun(topic, arg) != OM_OK) return OM_ERROR;
  }

  return OM_OK;
}

om_time_t om_msg_get_last_time(om_topic_t* topic) { return topic->msg.time; }
