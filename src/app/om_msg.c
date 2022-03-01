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

inline om_status_t _om_publish_to_suber(om_suber_t* sub, om_topic_t* topic) {
  if (sub->isLink) {
    _om_publish(sub->target, &topic->msg);
    return OM_ERROR;
  }

  if (sub->user_fun.filter != NULL &&
      sub->user_fun.filter(&topic->msg) != OM_OK)
    return OM_ERROR;

  if (sub->dump_target.enable && topic->msg.size <= sub->dump_target.max_size)
    memcpy(sub->dump_target.address, topic->msg.buff, topic->msg.size);
  OM_ASSERT(topic->msg.buff);

  if (sub->user_fun.deploy) sub->user_fun.deploy(&topic->msg);

  return OM_OK;
}

inline om_status_t _om_publish_to_topic(om_topic_t* topic, om_msg_t* msg) {
  om_time_get(&msg->time);

  if (topic->user_fun.filter != NULL && topic->user_fun.filter(msg) != OM_OK)
    return OM_ERROR;

  if (topic->virtual)
    memcpy(&topic->msg, msg, sizeof(*msg));
  else {
    topic->msg.buff = om_malloc(msg->size);
    OM_ASSERT(topic->msg.buff);
    memcpy(topic->msg.buff, msg->buff, msg->size);
    topic->msg.size = msg->size;
    topic->msg.time = msg->time;
  }

  return OM_OK;
}

om_status_t _om_publish(om_topic_t* topic, om_msg_t* msg) {
  if (_om_publish_to_topic(topic, msg) != OM_OK) return OM_ERROR;

  if (topic->afl) om_afl_apply(&(topic->msg), topic->afl);

  om_list_head_t* pos;
  om_list_for_each(pos, &topic->suber) {
    om_suber_t* sub = om_list_entry(pos, om_suber_t, self);

    _om_publish_to_suber(sub, topic);
  }

  if (!topic->virtual) {
    om_free(topic->msg.buff);
    topic->msg.buff = NULL;
  }

  return OM_OK;
}

om_status_t om_publish(om_topic_t* topic, void* buff, size_t size, bool block) {
  OM_ASSERT(topic);
  OM_ASSERT(buff);

  if (!om_msg_initd) return OM_ERROR_NOT_INIT;

  if (block)
    om_mutex_lock(&topic->mutex);
  else if (om_mutex_trylock(&om_mutex_handle) != OM_OK)
    return OM_ERROR_BUSY;

  om_msg_t msg = {.buff = buff, .size = size};

  om_status_t res = _om_publish(topic, &msg);

  om_mutex_unlock(&topic->mutex);

  return res;
}

inline om_status_t _om_refresh_puber(om_puber_t* pub, om_topic_t* topic) {
  OM_ASSERT(pub->user_fun.get_message);
  OM_ASSERT(pub->user_fun.new_message);

  pub->freq.counter--;
  if (pub->freq.counter > 0) return OM_ERROR;

  pub->freq.counter += pub->freq.reload;

  if (pub->user_fun.new_message(&pub->msg_buff) != OM_OK ||
      pub->user_fun.get_message(&pub->msg_buff) != OM_OK)
    return OM_ERROR;

  _om_publish(topic, &pub->msg_buff);

  return OM_OK;
}

om_status_t om_sync() {
#if OM_VIRTUAL_TIME
  om_time_update(_om_time_handle);
#endif

  if (!om_msg_initd) return OM_ERROR_NOT_INIT;

  om_mutex_lock(&om_mutex_handle);

  om_list_head_t *pos1, *pos2;
  om_list_for_each(pos1, &topic_list) {
    om_topic_t* topic = om_list_entry(pos1, om_topic_t, self);
    om_list_for_each(pos2, &topic->puber) {
      om_puber_t* pub = om_list_entry(pos2, om_puber_t, self);
      _om_refresh_puber(pub, topic);
    }
  }

  om_mutex_unlock(&om_mutex_handle);

  return OM_OK;
}

om_suber_t* om_subscript(om_topic_t* topic, void* buff, size_t max_size,
                         om_user_fun_t filter) {
  OM_ASSERT(topic);
  OM_ASSERT(buff);

  om_suber_t* sub = om_core_suber_create(NULL);
  om_core_set_dump_target(sub, buff, max_size);
  om_core_add_suber(topic, sub);
  sub->user_fun.filter = filter;

  return sub;
}

om_status_t om_msg_deinit() {
  om_msg_initd = false;

  om_list_head_t* pos;
  om_del_all(pos, &topic_list, om_core_del_topic);

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