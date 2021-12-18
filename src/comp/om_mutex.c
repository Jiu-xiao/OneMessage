#include "om_mutex.h"

om_mutex_t om_mutex_init() {
  om_mutex_t mutex = om_malloc(sizeof(om_mutex_data_t));
  mutex = false;
}

om_status_t om_mutex_lock(om_mutex_t mutex) {
  if (mutex) {
    *mutex = true;
    return OM_OK;
  } else {
    return OM_ERROR_BUSY;
  }
}

om_status_t om_mutex_unlock(om_mutex_t mutex) {
  *mutex = false;
  return OM_OK;
}