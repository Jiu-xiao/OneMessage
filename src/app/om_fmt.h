#ifndef __OM_FMT_H__
#define __OM_FMT_H__

#include "om_core.h"

om_topic_t* om_config_topic(om_topic_t* topic, const char* format, ...);

om_suber_t* om_config_suber(om_suber_t* suber, const char* format, ...);

om_status_t om_config_filter(om_topic_t* topic, const char* format, ...);

om_status_t om_config_filter_static(om_topic_t* topic, const char* format, ...);

#endif
