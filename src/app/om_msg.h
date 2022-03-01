#ifndef __OM_MSG_H__
#define __OM_MSG_H__

#include "om_afl.h"
#include "om_core.h"

#define om_topic_add_puber om_core_add_puber
#define om_topic_add_suber om_core_add_suber
#define om_add_topic om_core_add_topic
#define om_topic_link om_core_link
#define om_find_topic om_core_find_topic

om_status_t om_msg_init();

om_status_t _om_publish_to_suber(om_suber_t* sub, om_topic_t* topic);

om_status_t _om_publish_to_topic(om_topic_t* topic, om_msg_t* msg);

om_status_t _om_publish(om_topic_t* topic, om_msg_t* msg);

om_status_t om_publish(om_topic_t* topic, void* buff, size_t size, bool block);

om_status_t _om_refresh_puber(om_puber_t* pub, om_topic_t* topic);

om_status_t om_sync();

om_status_t om_subscript(om_topic_t* topic, void* buff, size_t max_size,
                         om_user_fun_t filter);

om_status_t om_msg_deinit();

om_status_t om_msg_del_topic(om_topic_t* topic);

om_status_t om_msg_del_suber(om_suber_t* suber);

om_status_t om_msg_del_puber(om_puber_t* puber);

#endif