#include "om_fmt.h"

#include "om_afl.h"
#include "om_log.h"
#include "om_msg.h"
#include "om_run.h"

#define GET_CAPITAL(_c) (isupper(_c) ? _c : toupper(_c))
/* MSG */
#define ADD2LIST ('A')
#define FILTER_FLAG ('F')
#define NEW_FLAG ('N')
#define GET_FLAG ('G')
#define DEPLOY_FLAG ('D')
#define LINK_FLAG ('L')
#define SUBER_FLAG ('S')
#define PUBER_FLAG ('P')
#define TOPIC_FLAG ('T')
#define FREQ_FLAG ('Q')
#define VIRTUAL_FLAG ('V')

/* AFL */
#define DECOMPOSE_FLAG ('D')
#define LIST_FLAG ('L')
#define RANGE_FLAG ('R')

om_topic_t* om_config_topic(om_topic_t* topic, const char* format, ...) {
  va_list valist;
  va_start(valist, format);

  if (!topic) topic = om_core_topic_create(va_arg(valist, const char*));
  if (format == NULL) {
    va_end(valist);
    return topic;
  }

  for (const uint8_t* index = (const uint8_t*)format; *index != '\0'; index++) {
    OM_ASSERT(isalpha(*index));

    switch (GET_CAPITAL(*index)) {
      case FILTER_FLAG:
        topic->user_fun.filter = va_arg(valist, om_user_fun_t);
        topic->user_fun.filter_arg = va_arg(valist, void*);

        OM_ASSERT(topic->user_fun.filter);
        break;
      case LINK_FLAG:
        om_core_link(topic, va_arg(valist, om_topic_t*));
        break;
      case SUBER_FLAG:
        om_core_add_suber(topic, va_arg(valist, om_suber_t*));
        break;
      case PUBER_FLAG:
        om_core_add_puber(topic, va_arg(valist, om_puber_t*));
        break;
      case TOPIC_FLAG:
        om_core_link(va_arg(valist, om_topic_t*), topic);
        break;
      case VIRTUAL_FLAG:
        topic->virtual_mode = true;
        break;
      case ADD2LIST:
        om_add_topic(topic);
        break;
      case DEPLOY_FLAG: {
        om_user_fun_t fun = va_arg(valist, om_user_fun_t);
        void* arg = va_arg(valist, void*);
        om_config_suber(NULL, "DT", fun, arg, topic);
      } break;
      case NEW_FLAG:
      case GET_FLAG: {
        om_user_fun_t fun_new = va_arg(valist, om_user_fun_t);
        void* arg_new = va_arg(valist, void*);
        om_user_fun_t fun_get = va_arg(valist, om_user_fun_t);
        void* arg_get = va_arg(valist, void*);
        om_config_puber(NULL, "NGT", fun_new, arg_new, fun_get, arg_get, topic);
      } break;
      default:
        OM_ASSERT(false);
        va_end(valist);
        return NULL;
    }
  }
  va_end(valist);

  return topic;
}

om_suber_t* om_config_suber(om_suber_t* suber, const char* format, ...) {
  if (!suber) suber = om_core_suber_create(NULL);
  if (format == NULL) return suber;

  va_list valist;
  va_start(valist, format);

  for (const uint8_t* index = (const uint8_t*)format; *index != '\0'; index++) {
    OM_ASSERT(isalpha(*index));
    switch (GET_CAPITAL(*index)) {
      case DEPLOY_FLAG:
        suber->mode = OM_SUBER_MODE_DEPLOY;

        suber->data.as_deploy.deploy = va_arg(valist, om_user_fun_t);
        suber->data.as_deploy.deploy_arg = va_arg(valist, void*);

        OM_ASSERT(suber->data.as_deploy.deploy);
        break;
      case TOPIC_FLAG:
        om_core_add_suber(va_arg(valist, om_topic_t*), suber);
        break;
      default:
        OM_ASSERT(false);
        va_end(valist);
        return NULL;
    }
  }
  va_end(valist);

  return suber;
}

om_puber_t* om_config_puber(om_puber_t* puber, const char* format, ...) {
  if (!puber) puber = om_core_puber_create(OM_CALL_FREQ);
  if (format == NULL) return puber;

  va_list valist;
  va_start(valist, format);

  for (const uint8_t* index = (const uint8_t*)format; *index != '\0'; index++) {
    OM_ASSERT(isalpha(*index));
    switch (GET_CAPITAL(*index)) {
      case NEW_FLAG:
        puber->user_fun.new_message = va_arg(valist, om_user_fun_t);
        puber->user_fun.new_arg = va_arg(valist, void*);

        OM_ASSERT(puber->user_fun.new_message);

        break;
      case GET_FLAG:
        puber->user_fun.get_message = va_arg(valist, om_user_fun_t);
        puber->user_fun.get_arg = va_arg(valist, void*);

        OM_ASSERT(puber->user_fun.get_message);

        break;
      case TOPIC_FLAG:
        om_core_add_puber(va_arg(valist, om_topic_t*), puber);
        break;
      case FREQ_FLAG:
        puber->freq.reload = OM_CALL_FREQ / va_arg(valist, double);
        puber->freq.counter = puber->freq.reload;
        break;
      default:
        OM_ASSERT(false);
        va_end(valist);
        return NULL;
    }
  }
  va_end(valist);

  return puber;
}

om_status_t om_config_filter(om_topic_t* topic, const char* format, ...) {
  OM_ASSERT(topic);
  OM_ASSERT(format);

  va_list valist;
  va_start(valist, format);

  if (topic->afl == NULL) {
    om_afl_create(topic);
  }

  for (const uint8_t* index = (const uint8_t*)format; *index != '\0'; index++) {
    OM_ASSERT(isalpha(*index));
    switch (GET_CAPITAL(*index)) {
      case LIST_FLAG: {
        om_filter_t* filter = om_afl_filter_create(va_arg(valist, om_topic_t*));
        uint32_t length = va_arg(valist, uint32_t);
        uint32_t offset = va_arg(valist, uint32_t);
        uint32_t scope = va_arg(valist, uint32_t);
        void* fl_template = va_arg(valist, void*);
        om_afl_set_filter(filter, OM_AFL_MODE_LIST, offset, length, scope, 0,
                          fl_template);
        om_afl_add_filter(topic->afl, filter);
        break;
      }
      case DECOMPOSE_FLAG: {
        om_filter_t* filter = om_afl_filter_create(va_arg(valist, om_topic_t*));
        uint32_t length = va_arg(valist, uint32_t);
        uint32_t offset = va_arg(valist, uint32_t);
        uint32_t scope = va_arg(valist, uint32_t);
        om_afl_set_filter(filter, OM_AFL_MODE_DECOMPOSE, offset, length, scope,
                          0, NULL);
        om_afl_add_filter(topic->afl, filter);
        break;
      }
      case RANGE_FLAG: {
        om_filter_t* filter = om_afl_filter_create(va_arg(valist, om_topic_t*));
        uint32_t length = va_arg(valist, uint32_t);
        uint32_t offset = va_arg(valist, uint32_t);
        uint32_t scope = va_arg(valist, uint32_t);
        uint32_t start = va_arg(valist, uint32_t);
        uint32_t arg = va_arg(valist, uint32_t);

        OM_UNUSED(scope);

        om_afl_set_filter(filter, OM_AFL_MODE_RANGE, offset, length, start, arg,
                          NULL);
        om_afl_add_filter(topic->afl, filter);
        break;
      }

      default:
        OM_ASSERT(false);
        return OM_ERROR;
    }
  }

  va_end(valist);

  return OM_OK;
}
