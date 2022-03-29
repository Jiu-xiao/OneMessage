#ifndef __OM_MSG_H__
#define __OM_MSG_H__

#include "om_core.h"

#define om_topic_add_puber om_core_add_puber
#define om_topic_add_suber om_core_add_suber
#define om_add_topic om_core_add_topic
#define om_topic_link om_core_link
#define om_find_topic om_core_find_topic

om_status_t om_msg_init();

om_status_t _om_publish_to_suber(om_suber_t* sub, om_topic_t* topic, bool block,
                                 bool in_isr);

om_status_t _om_publish_to_topic(om_topic_t* topic, om_msg_t* msg, bool block,
                                 bool in_isr);

om_status_t _om_publish(om_topic_t* topic, om_msg_t* msg, bool block,
                        bool in_isr);

om_status_t om_publish(om_topic_t* topic, void* buff, uint32_t size, bool block,
                       bool in_isr);

om_status_t _om_refresh_puber(om_puber_t* pub, om_topic_t* topic, bool block,
                              bool in_isr);

om_status_t om_sync(bool in_isr);

om_suber_t* om_subscript(om_topic_t* topic, void* buff, uint32_t max_size);

om_status_t om_suber_dump(om_suber_t* suber, bool in_isr);

om_status_t om_msg_deinit();

om_status_t om_msg_del_topic(om_topic_t* topic);

om_status_t om_msg_del_suber(om_suber_t* suber);

om_status_t om_msg_del_puber(om_puber_t* puber);

uint32_t om_msg_get_topic_num();

uint32_t om_msg_get_suber_num(om_topic_t* topic);

uint32_t om_msg_get_puber_num(om_topic_t* topic);

uint32_t om_msg_get_link_num(om_topic_t* topic);

om_status_t om_msg_for_each(om_status_t (*fun)(om_topic_t*, void* arg),
                            void* arg);

om_time_t om_msg_get_last_time(om_topic_t* topic);

#endif
