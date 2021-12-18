#include "om_config.h"

#ifndef OM_USE_USER_MALLOC

#define om_malloc malloc
#define om_free free

#endif

typedef enum {
  OM_OK = 0,
  OM_ERROR = 1,
  OM_ERROR_NULL,
  OM_ERROR_BUSY,
  OM_ERROR_TIMEOUT
} om_status_t;

#ifdef DEBUG

#define OM_ASSENT(arg) \
  if (!(arg)) return OM_ERROR_NULL;
#define OM_CHECK(arg) \
  if (!(arg)) return OM_ERROR;

#else

#define OM_ASSENT(arg) (void)0;
#define OM_CHECK(arg) (void)0;

#endif
