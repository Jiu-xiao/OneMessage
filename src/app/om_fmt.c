#include "om_fmt.h"

#include "om_msg.h"

#define GET_CAPITAL(_c) (isupper(_c) ? _c : toupper(_c))

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

om_topic_t* om_config_topic(om_topic_t* topic, const char* format, ...) {
  va_list valist;
  va_start(valist, format);

  if (!topic) topic = om_core_topic_create(va_arg(valist, const char*));
  if (format == NULL) {
    va_end(valist);
    return topic;
  }

  for (const uint8_t* index = (const uint8_t*)format; *index != '\0'; index++) {
    OM_ASSENT(isalpha(*index));

    switch (GET_CAPITAL(*index)) {
    case FILTER_FLAG:
      topic->user_fun.filter = va_arg(valist, om_user_fun_t);
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
      topic->virtual = true;
      break;
    case ADD2LIST:
      om_add_topic(topic);
      break;
    case DEPLOY_FLAG:
      om_config_suber(NULL, "DT", va_arg(valist, om_user_fun_t), topic);
      break;
    case NEW_FLAG:
    case GET_FLAG:
      om_config_puber(NULL, "NGT", va_arg(valist, om_user_fun_t), va_arg(valist, om_user_fun_t), topic);
      break;
    default:
      OM_ASSENT(false);
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
    OM_ASSENT(isalpha(*index));
    switch (GET_CAPITAL(*index)) {
    case FILTER_FLAG:
      suber->user_fun.filter = va_arg(valist, om_user_fun_t);
      break;
    case DEPLOY_FLAG:
      suber->user_fun.deploy = va_arg(valist, om_user_fun_t);
      break;
    case TOPIC_FLAG:
      om_core_add_suber(va_arg(valist, om_topic_t*), suber);
      break;
    default:
      OM_ASSENT(false);
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
    OM_ASSENT(isalpha(*index));
    switch (GET_CAPITAL(*index)) {
    case NEW_FLAG:
      puber->user_fun.new_message = va_arg(valist, om_user_fun_t);
      break;
    case GET_FLAG:
      puber->user_fun.get_message = va_arg(valist, om_user_fun_t);
      break;
    case TOPIC_FLAG:
      om_core_add_puber(va_arg(valist, om_topic_t*), puber);
      break;
    case FREQ_FLAG:
      puber->freq.reload = OM_CALL_FREQ / va_arg(valist, double);
      puber->freq.counter = puber->freq.reload;
      break;
    default:
      OM_ASSENT(false);
      va_end(valist);
      return NULL;
    }
  }
  va_end(valist);

  return puber;
}