#ifndef __OM_FIFO_H_
#define __OM_FIFO_H_

#include "om_def.h"

typedef struct {
  uint32_t ptr_write;
  uint32_t ptr_read;
  bool is_full;

  uint32_t item_sum;
  uint32_t item_size;
  void* fifo_ptr;
} om_fifo_t;

void om_fifo_create(om_fifo_t* fifo, void* fifo_ptr, uint32_t item_sum,
                    uint32_t item_size);

bool om_fifo_writeable(om_fifo_t* fifo);

om_status_t om_fifo_write(om_fifo_t* fifo, const void* data);

om_status_t om_fifo_writes(om_fifo_t* fifo, const void* data,
                           uint32_t item_num);

bool om_fifo_readable(om_fifo_t* fifo);

om_status_t om_fifo_read(om_fifo_t* fifo, void* data);

om_status_t om_fifo_pop(om_fifo_t* fifo);

om_status_t om_fifo_pop_batch(om_fifo_t* fifo, uint32_t item_num);

om_status_t om_fifo_push(om_fifo_t* fifo, const void* data);

om_status_t om_fifo_jump_peek(om_fifo_t* fifo, uint32_t num, void* data);

om_status_t om_fifo_peek(om_fifo_t* fifo, void* data);

om_status_t om_fifo_peek_batch(om_fifo_t* fifo, void* data, uint32_t item_num);

om_status_t om_fifo_reads(om_fifo_t* fifo, void* data, uint32_t item_num);

uint32_t om_fifo_readable_item_count(om_fifo_t* fifo);

uint32_t om_fifo_writeable_item_count(om_fifo_t* fifo);

om_status_t om_fifo_reset(om_fifo_t* fifo);

om_status_t om_fifo_overwrite(om_fifo_t* fifo, const void* data);

void om_fifo_foreach(om_fifo_t* fifo, bool (*fun)(void* data, void* arg),
                     void* arg);

void* om_fifo_foreach_dist(om_fifo_t* fifo, void* data);

#endif
