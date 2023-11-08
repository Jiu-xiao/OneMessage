#ifndef __OM_CORE_H__
#define __OM_CORE_H__

#include "om_color.h"
#include "om_crc.h"
#include "om_def.h"
#include "om_fifo.h"
#include "om_lib.h"
#include "om_list.h"
#include "om_rbt.h"

#define OM_TOPIC_LOCK(_topic) om_mutex_lock(&((_topic)->mutex))
#define OM_TOPIC_UNLOCK(_topic) om_mutex_unlock(&((_topic)->mutex))

typedef struct {
  bool virtual_mode;
  uint32_t buff_len;
  om_mutex_t mutex;
  om_msg_t msg;
  char name[OM_TOPIC_MAX_NAME_LEN];
  uint32_t crc32;
  om_rbt_node_t self;
  om_list_head_t suber;
  om_list_head_t link; /* 指向本话题的订阅者 */
  struct {
    om_user_fun_t filter;
    void* filter_arg;
  } user_fun;
  void* afl;
} om_topic_t;

typedef enum {
  OM_SUBER_MODE_UNKNOW,
  OM_SUBER_MODE_LINK,
  OM_SUBER_MODE_EXPORT,
  OM_SUBER_MODE_FIFO,
  OM_SUBER_MODE_DEFAULT,
} om_suber_mode_t;

typedef struct {
  om_suber_mode_t mode;

  om_list_head_t self;
  om_topic_t* master;

  union {
    struct {
      om_user_fun_t sub_callback;
      void* sub_cb_arg;
    } as_suber;

    struct {
      om_topic_t* target;
    } as_link;

    struct {
      bool new_data;
    } as_export;

    struct {
      om_fifo_t* fifo;
    } as_queue;
  } data;
} om_suber_t;

typedef struct {
  om_list_head_t self;
  struct {
    om_suber_t* suber;
    om_topic_t* topic;
  } source;
} om_link_t;

om_status_t om_core_init();

om_topic_t* om_core_topic_create(const char* name, uint32_t buff_len);

om_topic_t* om_core_topic_create_static(om_topic_t* topic, const char* name,
                                        uint32_t buff_len);

om_status_t om_core_add_topic(om_topic_t* topic);

om_suber_t* om_core_suber_create(om_topic_t* link);

om_suber_t* om_core_suber_create_static(om_suber_t* suber, om_topic_t* link);

om_status_t om_core_add_suber(om_topic_t* topic, om_suber_t* sub);

om_link_t* om_core_link_create(om_suber_t* sub, om_topic_t* topic);

om_link_t* om_core_link_create_static(om_link_t* link, om_suber_t* sub,
                                      om_topic_t* topic);

om_status_t om_core_add_link(om_topic_t* topic, om_link_t* link);

om_status_t om_core_queue_init_fifo_static(om_topic_t* topic, om_fifo_t* fifo,
                                           void* buff, uint32_t len);

om_fifo_t* om_core_queue_add(om_topic_t* topic, uint32_t len);

om_fifo_t* om_core_queue_add_static(om_topic_t* topic, om_suber_t* sub,
                                    om_fifo_t* fifo);

om_status_t om_core_link(om_topic_t* source, om_topic_t* target);

om_status_t om_core_link_static(om_suber_t* suber, om_link_t* link,
                                om_topic_t* source, om_topic_t* target);

om_status_t om_core_delink(om_list_head_t* head);

om_status_t om_core_del_suber(om_list_head_t* head);

om_status_t om_core_del_topic(om_rbt_node_t* node);

om_status_t om_core_set_export_target(om_suber_t* suber);

om_topic_t* om_core_find_topic(const char* name, uint32_t timeout);

void om_error(const char* file, uint32_t line);
#endif
