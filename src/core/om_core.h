#include "om_def.h"
#include "om_lib.h"
#include "om_list.h"
#include "om_mutex.h"

#define OM_TOPIC_MAX_NAME_LEN (10)

typedef struct {
  size_t size;
  void* buff;
  om_time_t time;
} om_msg_t;

typedef om_status_t (*om_user_fun_t)(om_msg_t* msg);

typedef struct {
  char name[OM_TOPIC_MAX_NAME_LEN];
  bool enable;
  om_list_head_t self;
  om_list_head_t suber;
  om_list_head_t puber;
  om_list_head_t link; /* 指向本话题的订阅者 */
  struct {
    om_user_fun_t filter;
    om_user_fun_t decode;
  } user_fun;
} om_topic_t;

typedef struct {
  om_list_head_t self;
  struct {
    om_user_fun_t new;
    om_user_fun_t get;
  } user_fun;
  om_msg_t msg_buff;
} om_puber_t;

typedef struct {
  om_list_head_t self;
  struct {
    om_user_fun_t filter;
    om_user_fun_t apply;
  } user_fun;
  bool isLink;
  om_topic_t* target;
  om_msg_t msg_buff;
} om_suber_t;

typedef struct {
  om_list_head_t self;
  om_suber_t* source;
} om_link_t;

om_topic_t* om_core_topic_create(const char* name);

om_status_t om_core_add_topic(om_topic_t* topic);

om_suber_t* om_core_suber_create(om_topic_t* link);

om_status_t om_core_add_suber(om_topic_t* topic, om_suber_t* sub);

om_puber_t* om_core_puber_create();

om_status_t om_core_add_puber(om_topic_t* topic, om_puber_t* pub);

om_link_t* om_core_link_create(om_suber_t* sub);

om_status_t om_core_add_link(om_topic_t* topic, om_link_t* link);

om_status_t om_core_link(om_topic_t* source, om_topic_t* target);

om_status_t om_core_delink(om_list_head_t* head);

om_status_t om_core_del_suber(om_list_head_t* head);

om_status_t om_core_del_puber(om_list_head_t* head);

om_status_t om_core_del_topic(om_list_head_t* head);

om_topic_t* om_core_find_topic(const char* name);