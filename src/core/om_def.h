#ifndef __OM_DEF_H_
#define __OM_DEF_H_

#include "om_lib.h"

#ifdef OM_TEST
#include "om_config_template.h"
#else
#include "om_config.h"
#endif

#if !OM_USE_USER_MALLOC
#define om_malloc malloc
#define om_free free
#endif

typedef enum {
  OM_OK = 0,
  OM_ERROR = 1,
  OM_ERROR_NULL,
  OM_ERROR_BUSY,
  OM_ERROR_TIMEOUT,
  OM_ERROR_NOT_INIT
} om_status_t;

typedef struct {
  uint32_t size;
  void* buff;
  om_time_t time;
} om_msg_t;

typedef om_status_t (*om_user_fun_t)(om_msg_t* msg, void* arg);

#if OM_DEBUG
#define OM_ASSERT(arg) \
  if (!(arg)) om_error(__FILE__, __LINE__);
#define OM_CHECK(arg) \
  if (!(arg)) om_error(__FILE__, __LINE__);

#else
#define OM_ASSERT(arg) (void)0;
#define OM_CHECK(arg) (void)0;
#endif

#define OM_UNUSED(X) ((void)X)

#if OM_VIRTUAL_TIME
#define om_time_t uint32_t
#define om_time_update(time) time++
#define om_time_get(time) *time = _om_time_handle
#endif

#define om_offset_of(type, member) ((size_t) & ((type*)0)->member)

#define om_member_size_of(type, member) (sizeof(typeof(((type*)0)->member)))

#define om_container_of(ptr, type, member)               \
  ({                                                     \
    const typeof(((type*)0)->member)* __mptr = (ptr);    \
    (type*)((char*)__mptr - om_offset_of(type, member)); \
  })

#endif
