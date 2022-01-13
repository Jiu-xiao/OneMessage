#include "om.h"

om_status_t om_init() {
  om_status_t res = OM_OK;
  res += om_msg_init();
#if OM_LOG_OUTPUT
  res += om_log_init();
#endif
  return res;
}

om_status_t om_deinit() {
  om_status_t res = OM_OK;
  res += om_msg_init();
#if OM_LOG_OUTPUT
  res += om_msg_deinit();
#endif
  return res;
}