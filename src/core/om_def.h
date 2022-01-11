#ifndef __OM_CONFIG_H_
#define __OM_CONFIG_H_

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

#ifdef OM_DEBUG
#define OM_ASSENT(arg) \
  if (!(arg)) om_error(__FILE__, __LINE__);
#define OM_CHECK(arg) \
  if (!(arg)) om_error(__FILE__, __LINE__);

#else
#define OM_ASSENT(arg) (void)0;
#define OM_CHECK(arg) (void)0;
#endif

#if OM_VIRTUAL_TIME
#define om_time_t uint32_t
#define om_time_update(time) time++
#define om_time_get(time) *time
#endif

#endif
