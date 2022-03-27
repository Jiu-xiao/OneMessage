#ifndef __OM_CORE_H__
#define __OM_CORE_H__

#include "om_def.h"
#include "om_lib.h"
#include "om_list.h"

typedef struct {
  uint32_t size;
  void* buff;
  om_time_t time;
} om_msg_t;

typedef om_status_t (*om_user_fun_t)(om_msg_t* msg, void* arg);

typedef struct {
  bool virtual;
  om_mutex_t mutex;
  om_msg_t msg;
  char name[OM_TOPIC_MAX_NAME_LEN];
  om_list_head_t self;
  om_list_head_t suber;
  om_list_head_t puber;
  om_list_head_t link; /* 指向本话题的订阅者 */
  struct {
    om_user_fun_t filter;
    void* filter_arg;
  } user_fun;
  void* afl;
} om_topic_t;

typedef struct {
  om_list_head_t self;
  struct {
    om_user_fun_t new_message;
    om_user_fun_t get_message;
    void* new_arg;
    void* get_arg;
  } user_fun;
  om_msg_t msg_buff;
  struct {
#if OM_FREQ_USE_FLOAT
    float reload;
    float counter;
#else
    uint32_t reload;
    uint32_t counter;
#endif
  } freq;
} om_puber_t;

typedef enum {
  OM_SUBER_MODE_UNKNOW,
  OM_SUBER_MODE_LINK,
  OM_SUBER_MODE_DUMP,
  OM_SUBER_MODE_DEPLOY,
} om_suber_mode_t;

typedef struct {
  om_suber_mode_t mode;

  om_list_head_t self;
  om_topic_t* master;

  union {
    struct {
      om_user_fun_t deploy;
      void* deploy_arg;
    } as_deploy;

    struct {
      om_topic_t* target;
    } as_link;

    struct {
      uint32_t max_size;
      uint8_t* buff;
      bool new;
    } as_dump;
  } data;
} om_suber_t;

typedef struct {
  om_list_head_t self;
  struct {
    om_suber_t* suber;
    om_topic_t* topic;
  } source;
} om_link_t;

om_topic_t* om_core_topic_create(const char* name);

om_status_t om_core_add_topic(om_topic_t* topic);

om_suber_t* om_core_suber_create(om_topic_t* link);

om_status_t om_core_add_suber(om_topic_t* topic, om_suber_t* sub);

om_puber_t* om_core_puber_create(float freq);

om_status_t om_core_add_puber(om_topic_t* topic, om_puber_t* pub);

om_link_t* om_core_link_create(om_suber_t* sub, om_topic_t* topic);

om_status_t om_core_add_link(om_topic_t* topic, om_link_t* link);

om_status_t om_core_link(om_topic_t* source, om_topic_t* target);

om_status_t om_core_delink(om_list_head_t* head);

om_status_t om_core_del_suber(om_list_head_t* head);

om_status_t om_core_del_puber(om_list_head_t* head);

om_status_t om_core_del_topic(om_list_head_t* head);

om_status_t om_core_set_dump_target(om_suber_t* suber, void* target,
                                    uint32_t max_size);

om_topic_t* om_core_find_topic(const char* name, uint32_t timeout);

void om_error(const char* file, uint32_t line);
#endif
