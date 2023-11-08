#ifndef __OM_MSG_H__
#define __OM_MSG_H__

#include "om_core.h"

#define om_topic_add_suber om_core_add_suber
#define om_add_topic om_core_add_topic
#define om_topic_link om_core_link
#define om_topic_link_static om_core_link_static
#define om_find_topic om_core_find_topic
#define om_create_topic om_core_topic_create
#define om_create_topic_static om_core_topic_create_static
#define om_create_suber om_core_suber_create
#define om_create_suber_static om_core_suber_create_static
#define om_create_link om_core_link_create
#define om_create_link_static om_core_link_create_static
#define om_queue_add om_core_queue_add
#define om_queue_add_static om_core_queue_add_static
#define om_queue_init_fifo_static om_core_queue_init_fifo_static

om_status_t _om_publish_to_suber(om_suber_t* sub, om_topic_t* topic, bool block,
                                 bool in_isr);

om_status_t _om_publish_to_topic(om_topic_t* topic, om_msg_t* msg, bool block,
                                 bool in_isr);

om_status_t _om_publish(om_topic_t* topic, om_msg_t* msg, bool block,
                        bool in_isr);

om_status_t om_publish(om_topic_t* topic, void* buff, uint32_t size, bool block,
                       bool in_isr);

om_status_t om_sync(bool in_isr);

om_suber_t* om_subscribe(om_topic_t* topic);

om_suber_t* om_subscribe_static(om_topic_t* topic, om_suber_t* sub);

bool om_suber_available(om_suber_t* suber);

om_status_t om_suber_export(om_suber_t* suber, void* buff, bool in_isr);

om_status_t om_msg_del_topic(om_topic_t* topic);

om_status_t om_msg_del_suber(om_suber_t* suber);

om_status_t om_msg_foreach_topic(bool (*fun)(om_topic_t* topic, void* arg),
                                 void* arg);

uint32_t om_msg_get_topic_num();

uint32_t om_msg_get_suber_num(om_topic_t* topic);

uint32_t om_msg_get_link_num(om_topic_t* topic);

#if OM_TIME
om_time_t om_msg_get_last_time(om_topic_t* topic);
#endif

#endif
