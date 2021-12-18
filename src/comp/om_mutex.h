#include "om_def.h"
#include "om_lib.h"

typedef volatile bool om_mutex_data_t;
typedef om_mutex_data_t* om_mutex_t;

om_status_t om_mutex_lock(om_mutex_t mutex);

om_status_t om_mutex_unlock(om_mutex_t mutex);