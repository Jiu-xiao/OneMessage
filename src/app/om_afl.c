#include "om_afl.h"

#include "om_fmt.h"
#include "om_log.h"
#include "om_msg.h"

om_afl_t* om_afl_create(om_topic_t* source) {
  om_afl_t* afl = om_malloc(sizeof(om_afl_t));

  return om_afl_create_static(afl, source);
}

om_afl_filter_t* om_afl_filter_create(om_topic_t* target) {
  om_afl_filter_t* filter = om_malloc(sizeof(om_afl_filter_t));

  return om_afl_filter_create_static(filter, target);
}

om_afl_t* om_afl_create_static(om_afl_t* afl, om_topic_t* source) {
  OM_ASSERT(afl);
  memset(afl, 0, sizeof(om_afl_t));

  OM_INIT_LIST_HEAD(&afl->filter);
  afl->source_topic = source;
  source->afl = afl;

  return afl;
}

om_afl_filter_t* om_afl_filter_create_static(om_afl_filter_t* filter,
                                             om_topic_t* target) {
  OM_ASSERT(filter);
  memset(filter, 0, sizeof(om_afl_filter_t));

  filter->target = target;

  return filter;
}

om_status_t om_afl_add_filter(om_afl_t* afl, om_afl_filter_t* filter) {
  OM_ASSERT(afl);
  OM_ASSERT(filter);

  om_list_add(&filter->self, &afl->filter);

  return OM_OK;
}

om_status_t om_afl_set_filter(om_afl_filter_t* filter, uint8_t mode,
                              uint32_t offset, uint32_t length, uint32_t scope,
                              uint32_t arg, void* fl_template) {
  OM_ASSERT(filter);

  filter->mode = mode;
  filter->length = length;

  switch (mode) {
    case OM_AFL_MODE_LIST:
      filter->data.list.scope = scope;
      filter->data.list.offset = offset;
      filter->data.list.fl_template = fl_template;
      break;
    case OM_AFL_MODE_RANGE:
      filter->data.range.offset = offset;
      filter->data.range.start = scope;
      filter->data.range.range = arg;
      break;

    case OM_AFL_MODE_DECOMPOSE:
      filter->data.decomp.offset = offset;
      filter->data.decomp.size = scope;
      break;
    default:
      OM_ASSERT(false);
      return OM_ERROR;
  }

  return OM_OK;
}

om_status_t _om_afl_filter_check(om_afl_filter_t* filter, om_msg_t* msg) {
  if (filter->length && msg->size != filter->length) return OM_ERROR;

  uint8_t* buff = msg->buff;

  switch (filter->mode) {
    case OM_AFL_MODE_LIST:
      if (msg->size < filter->data.list.scope + filter->data.list.offset)
        return OM_ERROR;
      buff += filter->data.list.offset;
      if (!memcmp(buff, filter->data.list.fl_template, filter->data.list.scope))
        return OM_OK;
      break;
    case OM_AFL_MODE_RANGE:
      if (msg->size < filter->data.range.offset + sizeof(uint32_t)) {
        OM_ASSERT(false);
        return OM_ERROR;
      }
      buff += filter->data.range.offset;
      if (*((uint32_t*)buff) - filter->data.range.start <
          filter->data.range.range)
        return OM_OK;
      break;
    case OM_AFL_MODE_DECOMPOSE:
      if (msg->size < filter->data.decomp.offset + filter->data.decomp.size)
        return OM_ERROR;
      return OM_OK;

    default:
      return OM_ERROR;
  }

  return OM_ERROR;
}

om_status_t _om_afl_filter_apply(om_afl_filter_t* filter, om_msg_t* msg,
                                 bool block, bool in_isr) {
  switch (filter->mode) {
    case OM_AFL_MODE_LIST:
    case OM_AFL_MODE_RANGE:
      om_publish(filter->target, msg->buff, msg->size, block, in_isr);
      break;
    case OM_AFL_MODE_DECOMPOSE:
      om_publish(filter->target, msg->buff + filter->data.decomp.offset,
                 filter->data.decomp.size, block, in_isr);
      break;
    default:
      OM_ASSERT(false);
      return OM_ERROR;
  }

  return OM_OK;
}

om_status_t om_afl_apply(om_msg_t* msg, om_afl_t* afl, bool block,
                         bool in_isr) {
  OM_ASSERT(msg);
  OM_ASSERT(afl);

  om_list_head_t* pos;
  om_list_for_each(pos, &afl->filter) {
    om_afl_filter_t* filter = om_list_entry(pos, om_afl_filter_t, self);
    if (_om_afl_filter_check(filter, msg) == OM_OK)
      _om_afl_filter_apply(filter, msg, block, in_isr);
  }

  return OM_OK;
}

om_status_t om_afl_filter_del(om_list_head_t* filter) {
  om_list_del(filter);

  om_afl_filter_t* t = om_list_entry(filter, om_afl_filter_t, self);

  om_free(t);

  return OM_OK;
}

uint32_t om_afl_get_num(om_afl_t* afl) { return om_list_get_num(&afl->filter); }

om_status_t om_afl_del(om_afl_t* afl) {
  om_del_all(&afl->filter, om_afl_filter_del);

  om_free(afl);

  return OM_OK;
}
