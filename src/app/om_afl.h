#ifndef __OM_AFL_H__
#define __OM_AFL_H__

#include "om_core.h"

typedef enum {
  OM_AFL_MODE_LIST,
  OM_AFL_MODE_RANGE,
  OM_AFL_MODE_DECOMPOSE,
  OM_AFL_MODE_NUMBER
} om_afl_mode_t;

typedef struct {
  om_topic_t* source_topic;
  om_list_head_t filter;
} om_afl_t;

typedef struct {
  uint32_t offset;
  uint32_t scope;
  void* fl_template;
} _om_afl_filter_list_t;

typedef struct {
  uint32_t offset;
  uint32_t start;
  uint32_t range;
} _om_afl_filter_range_t;

typedef struct {
  uint32_t offset;
  uint32_t size;
} _om_afl_filter_decompose_t;

typedef struct {
  uint32_t length;
  om_afl_mode_t mode;
  union {
    _om_afl_filter_list_t list;
    _om_afl_filter_range_t range;
    _om_afl_filter_decompose_t decomp;
  } data;
  om_list_head_t self;
  om_topic_t* target;
} om_afl_filter_t;

om_afl_t* om_afl_create(om_topic_t* source);

om_afl_filter_t* om_afl_filter_create(om_topic_t* target);

om_afl_t* om_afl_create_static(om_afl_t* afl, om_topic_t* source);

om_afl_filter_t* om_afl_filter_create_static(om_afl_filter_t* filter,
                                             om_topic_t* target);

om_status_t om_afl_add_filter(om_afl_t* afl, om_afl_filter_t* filter);

om_status_t om_afl_set_filter(om_afl_filter_t* filter, uint8_t mode,
                              uint32_t offset, uint32_t length, uint32_t scope,
                              uint32_t arg, void* fl_template);

om_status_t _om_afl_filter_check(om_afl_filter_t* filter, om_msg_t* msg);

om_status_t _om_afl_filter_apply(om_afl_filter_t* filter, om_msg_t* msg,
                                 bool block, bool in_isr);

om_status_t om_afl_apply(om_msg_t* msg, om_afl_t* afl, bool block, bool in_isr);

om_status_t om_afl_filter_del(om_list_head_t* filter);

uint32_t om_afl_get_num(om_afl_t* afl);

om_status_t om_afl_del(om_afl_t* afl);

#endif
