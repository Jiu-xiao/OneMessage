#include "om.h"

om_status_t om_init() {
  om_status_t res = OM_OK;

  res += om_core_init();

#if OM_LOG_OUTPUT
  res += om_log_init();
#endif
  return res;
}
