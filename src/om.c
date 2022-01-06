#include "om.h"

om_time_t time_handle;
extern om_list_head_t topic_list;
#if OM_LOG_OUTPUT
static om_topic_t *om_log;
#endif
om_config_t OM_EMPTY_CONFIG = {.arg = NULL, .op = OM_CONFIG_END};

om_status_t om_init() {
#if OM_LOG_OUTPUT
  om_log = om_core_topic_create("om_log");
  om_core_add_topic(om_log);
#endif
  return OM_OK;
}

om_status_t om_config_topic(om_topic_t *topic, om_config_t *config) {
  OM_ASSENT(config);
  OM_ASSENT(topic);

  for (; config->op != OM_CONFIG_END; config++) {
    switch (config->op) {
      case OM_USER_FUN_FILTER:
        topic->user_fun.filter = config->arg;
        break;
      case OM_USER_FUN_DECODE:
        topic->user_fun.decode = config->arg;
        break;
      case OM_LINK: {
        om_topic_t **target = config->arg;
        om_core_link(topic, *target);
      } break;
      case OM_ADD_PUBER: {
        om_puber_t **pub = config->arg;
        om_core_add_puber(topic, *pub);
      } break;
      case OM_ADD_SUBER: {
        om_suber_t **sub = config->arg;
        om_core_add_suber(topic, *sub);
      } break;
      default:
        return OM_ERROR;
    }
  }

  return OM_OK;
}

om_status_t om_config_suber(om_suber_t *sub, om_config_t *config) {
  OM_ASSENT(config);
  OM_ASSENT(sub);

  for (; config->op != OM_CONFIG_END; config++) {
    switch (config->op) {
      case OM_USER_FUN_FILTER:
        sub->user_fun.filter = config->arg;
        break;
      case OM_USER_FUN_APPLY:
        sub->user_fun.apply = config->arg;
      default:
        return OM_ERROR;
    }
  }

  return OM_OK;
}

om_status_t om_config_puber(om_puber_t *pub, om_config_t *config) {
  OM_ASSENT(config);
  OM_ASSENT(pub);

  for (; config->op != OM_CONFIG_END; config++) {
    switch (config->op) {
      case OM_USER_FUN_NEW:
        pub->user_fun.new = config->arg;
        break;
      case OM_USER_FUN_GET:
        pub->user_fun.get = config->arg;
        break;
      default:
        return OM_ERROR;
    }
  }

  return OM_OK;
}

om_status_t _om_publish(om_topic_t *topic, om_msg_t *msg) {
  OM_ASSENT(topic);
  OM_ASSENT(msg);

  if (topic->user_fun.filter == NULL || topic->user_fun.filter(msg) == OM_OK) {
    if (topic->user_fun.decode) topic->user_fun.decode(msg);

    if (topic->msg.buff) om_free(topic->msg.buff);
    topic->msg.buff = om_malloc(msg->size);
    memcpy(topic->msg.buff, msg->buff, msg->size);
    topic->msg.size = msg->size;

    om_list_head_t *pos;
    om_list_for_each(pos, &topic->suber) {
      om_suber_t *sub = om_list_entry(pos, om_suber_t, self);
      if (sub->isLink) {
        _om_publish(sub->target, &topic->msg);
      } else {
        if (sub->user_fun.filter == NULL ||
            sub->user_fun.filter(&topic->msg) == OM_OK) {
          sub->msg_buff.buff = topic->msg.buff;
          sub->msg_buff.size = topic->msg.size;
          if (sub->user_fun.apply) sub->user_fun.apply(&topic->msg);
        }
      }
    }
  }

  return OM_OK;
}

om_status_t om_publish_with_name(const char *name, void *buff, size_t size) {
  OM_ASSENT(name);
  OM_ASSENT(buff);

  om_topic_t *topic = om_core_find_topic(name);
  if (topic == NULL) return OM_ERROR_NULL;

  om_msg_t msg = {.buff = buff, .size = size, .time = om_time_get(time_handle)};

  return _om_publish(topic, &msg);
}

om_status_t om_publish_with_handle(om_topic_t *topic, void *buff, size_t size) {
  OM_ASSENT(topic);
  OM_ASSENT(buff);

  om_msg_t msg = {.buff = buff, .size = size, .time = om_time_get(time_handle)};

  return _om_publish(topic, &msg);
}

om_status_t om_sync() {
#if OM_VIRTUAL_TIME
  om_time_update(time_handle);
#endif

  om_list_head_t *pos1, *pos2;
  om_list_for_each(pos1, &topic_list) {
    om_topic_t *topic = om_list_entry(pos1, om_topic_t, self);
    om_list_for_each(pos2, &topic->puber) {
      om_puber_t *pub = om_list_entry(pos2, om_puber_t, self);
      OM_ASSENT(pub->user_fun.get);
      OM_ASSENT(pub->user_fun.new);

      if (pub->user_fun.new(&pub->msg_buff) == OM_OK &&
          pub->user_fun.get(&pub->msg_buff) == OM_OK) {
        pub->msg_buff.time = om_time_get(time_handle);
        _om_publish(topic, &pub->msg_buff);
      }
    }
  }

  return OM_OK;
}

om_topic_t *om_create_topic(const char *name, om_config_t *config) {
  OM_ASSENT(name);
  OM_ASSENT(config);

  om_topic_t *topic = om_core_topic_create(name);
  om_config_topic(topic, config);

  return topic;
}

om_suber_t *om_create_suber(om_config_t *config) {
  OM_ASSENT(config);

  om_suber_t *sub = om_core_suber_create(NULL);
  om_config_suber(sub, config);

  return sub;
}

om_puber_t *om_create_puber(om_config_t *config) {
  OM_ASSENT(config);

  om_puber_t *pub = om_core_puber_create(NULL);
  om_config_puber(pub, config);

  return pub;
}

om_status_t om_deinit() {
  om_list_head_t *pos;
  om_del_all(pos, &topic_list, om_core_del_topic);
  return OM_OK;
}

#if OM_LOG_OUTPUT
inline om_topic_t *om_get_log_handle() { return om_log; }

om_status_t om_print_log(om_log_t *log, const char *format, ...) {
  va_list vArgList;
  va_start(vArgList, format);
  vsnprintf(log->data, OM_LOG_MAX_LEN, format, vArgList);
  va_end(vArgList);
  log->time = om_time_get(time_handle);
  return om_publish_with_handle(om_log, log, sizeof(om_log_t));
}
#endif