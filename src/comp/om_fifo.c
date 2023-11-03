#include "om_fifo.h"

#include <stdint.h>

#include "om_def.h"

inline void om_fifo_create(om_fifo_t* fifo, void* fifo_ptr, uint32_t item_sum,
                           uint32_t item_size) {
  fifo->item_sum = item_sum;
  fifo->item_size = item_size;
  fifo->ptr_write = 0;
  fifo->ptr_read = 0;
  fifo->is_full = false;
  fifo->fifo_ptr = fifo_ptr;
}

inline bool om_fifo_writeable(om_fifo_t* fifo) { return !fifo->is_full; }

inline om_status_t om_fifo_write(om_fifo_t* fifo, const void* data) {
  if (fifo->is_full) {
    return OM_ERROR_FULL;
  }

  memcpy((uint8_t*)(fifo->fifo_ptr) + fifo->ptr_write * fifo->item_size, data,
         fifo->item_size);

  fifo->ptr_write++;

  if (fifo->ptr_write >= fifo->item_sum) {
    fifo->ptr_write = 0;
  }

  if (fifo->ptr_write == fifo->ptr_read) {
    fifo->is_full = true;
  }

  return OM_OK;
}

inline om_status_t om_fifo_writes(om_fifo_t* fifo, const void* data,
                                  uint32_t item_num) {
  if (om_fifo_writeable_item_count(fifo) < item_num) {
    return OM_ERROR_FULL;
  }

  if (fifo->ptr_write + item_num < fifo->item_sum) {
    memcpy((uint8_t*)(fifo->fifo_ptr) + fifo->ptr_write * fifo->item_size, data,
           fifo->item_size * item_num);
  } else {
    memcpy((uint8_t*)(fifo->fifo_ptr) + fifo->ptr_write * fifo->item_size, data,
           fifo->item_size * (fifo->item_sum - fifo->ptr_write));

    memcpy(
        (uint8_t*)(fifo->fifo_ptr),
        (uint8_t*)(data) + fifo->item_size * (fifo->item_sum - fifo->ptr_write),
        fifo->item_size * (item_num - fifo->item_sum + fifo->ptr_write));
  }

  fifo->ptr_write += item_num;

  if (fifo->ptr_write >= fifo->item_sum) {
    fifo->ptr_write -= fifo->item_sum;
  }

  if (fifo->ptr_write == fifo->ptr_read) {
    fifo->is_full = true;
  }

  return OM_OK;
}

inline bool om_fifo_readable(om_fifo_t* fifo) {
  if (fifo->ptr_write == fifo->ptr_read && !fifo->is_full) {
    return false;
  }
  return true;
}

inline om_status_t om_fifo_read(om_fifo_t* fifo, void* data) {
  if (fifo->ptr_write == fifo->ptr_read && !fifo->is_full) {
    return OM_ERROR_EMPTY;
  }

  memcpy(data, (uint8_t*)(fifo->fifo_ptr) + fifo->ptr_read * fifo->item_size,
         fifo->item_size);
  fifo->ptr_read++;
  if (fifo->ptr_read >= fifo->item_sum) {
    fifo->ptr_read = 0;
  }
  fifo->is_full = false;
  return OM_OK;
}

om_status_t om_fifo_pop(om_fifo_t* fifo) {
  if (fifo->ptr_write == fifo->ptr_read && !fifo->is_full) {
    return OM_ERROR_EMPTY;
  }

  fifo->ptr_read++;
  if (fifo->ptr_read >= fifo->item_sum) {
    fifo->ptr_read = 0;
  }

  fifo->is_full = false;
  return OM_OK;
}

om_status_t om_fifo_pop_batch(om_fifo_t* fifo, uint32_t item_num) {
  if (om_fifo_readable_item_count(fifo) < item_num) {
    return OM_ERROR_EMPTY;
  }

  fifo->ptr_read += item_num;

  if (fifo->ptr_read >= fifo->item_sum) {
    fifo->ptr_read -= fifo->item_sum;
  }

  fifo->is_full = false;
  return OM_OK;
}

om_status_t om_fifo_push(om_fifo_t* fifo, const void* data) {
  return om_fifo_write(fifo, data);
}

inline om_status_t om_fifo_peek(om_fifo_t* fifo, void* data) {
  if (fifo->ptr_write == fifo->ptr_read && !fifo->is_full) {
    return OM_ERROR_EMPTY;
  }

  memcpy(data, (uint8_t*)(fifo->fifo_ptr) + fifo->ptr_read * fifo->item_size,
         fifo->item_size);
  return OM_OK;
}

inline om_status_t om_fifo_jump_peek(om_fifo_t* fifo, uint32_t num,
                                     void* data) {
  if (om_fifo_readable_item_count(fifo) < num) {
    return OM_ERROR_EMPTY;
  }

  uint32_t index = fifo->ptr_read + num;

  if (index >= fifo->item_sum) {
    index -= fifo->item_sum;
  }

  memcpy(data, (uint8_t*)(fifo->fifo_ptr) + index * fifo->item_size,
         fifo->item_size);
  return OM_OK;
}

inline om_status_t om_fifo_peek_batch(om_fifo_t* fifo, void* data,
                                      uint32_t item_num) {
  if (om_fifo_readable_item_count(fifo) < item_num) {
    return OM_ERROR_EMPTY;
  }

  if (fifo->ptr_read + item_num < fifo->item_sum) {
    memcpy(data, (uint8_t*)(fifo->fifo_ptr) + fifo->ptr_read * fifo->item_size,
           fifo->item_size * item_num);
  } else {
    memcpy(data, (uint8_t*)(fifo->fifo_ptr) + fifo->ptr_read * fifo->item_size,
           fifo->item_size * (fifo->item_sum - fifo->ptr_read));

    memcpy(
        (uint8_t*)(data) + fifo->item_size * (fifo->item_sum - fifo->ptr_read),
        (uint8_t*)(fifo->fifo_ptr),
        fifo->item_size * (item_num - fifo->item_sum + fifo->ptr_read));
  }

  return OM_OK;
}

inline om_status_t om_fifo_reads(om_fifo_t* fifo, void* data,
                                 uint32_t item_num) {
  if (om_fifo_readable_item_count(fifo) < item_num) {
    return OM_ERROR_EMPTY;
  }

  if (fifo->ptr_read + item_num < fifo->item_sum) {
    memcpy(data, (uint8_t*)(fifo->fifo_ptr) + fifo->ptr_read * fifo->item_size,
           fifo->item_size * item_num);
  } else {
    memcpy(data, (uint8_t*)(fifo->fifo_ptr) + fifo->ptr_read * fifo->item_size,
           fifo->item_size * (fifo->item_sum - fifo->ptr_read));

    memcpy(
        (uint8_t*)(data) + fifo->item_size * (fifo->item_sum - fifo->ptr_read),
        (uint8_t*)(fifo->fifo_ptr),
        fifo->item_size * (item_num - fifo->item_sum + fifo->ptr_read));
  }

  fifo->ptr_read += item_num;
  if (fifo->ptr_read >= fifo->item_sum) {
    fifo->ptr_read -= fifo->item_sum;
  }
  fifo->is_full = false;
  return OM_OK;
}

inline uint32_t om_fifo_readable_item_count(om_fifo_t* fifo) {
  if (fifo->is_full) {
    return fifo->item_sum;
  } else {
    return (fifo->item_sum + fifo->ptr_write - fifo->ptr_read) % fifo->item_sum;
  }
}

inline uint32_t om_fifo_writeable_item_count(om_fifo_t* fifo) {
  if (fifo->is_full) {
    return 0;
  } else {
    if (fifo->ptr_write == fifo->ptr_read) {
      return fifo->item_sum;
    } else {
      return (fifo->item_sum + fifo->ptr_read - fifo->ptr_write) %
             fifo->item_sum;
    }
  }
}

inline om_status_t om_fifo_reset(om_fifo_t* fifo) {
  fifo->ptr_read = fifo->ptr_write;
  fifo->is_full = false;

  return OM_OK;
}

inline om_status_t om_fifo_overwrite(om_fifo_t* fifo, const void* data) {
  om_fifo_reset(fifo);

  return om_fifo_write(fifo, data);
}

void om_fifo_foreach(om_fifo_t* fifo, bool (*fun)(void* data, void* arg),
                     void* arg) {
  uint32_t index = fifo->ptr_read, num = om_fifo_readable_item_count(fifo);

  for (uint32_t i = 0; i < num; i++) {
    if (!fun((uint8_t*)(fifo->fifo_ptr) + fifo->item_size * index, arg)) {
      return;
    }
    index++;
    if (index >= fifo->item_sum) {
      index -= fifo->item_sum;
    }
  }
}

void* om_fifo_foreach_dist(om_fifo_t* fifo, void* data) {
  uint32_t index = fifo->ptr_read, num = om_fifo_readable_item_count(fifo);

  if (num == 0) {
    return NULL;
  }

  if (data == NULL) {
    return (uint8_t*)(fifo->fifo_ptr) + fifo->item_size * fifo->ptr_read;
  }

  index = ((data - fifo->fifo_ptr) / fifo->item_size + 1) % fifo->item_sum;

  if (index == fifo->ptr_write) {
    return NULL;
  } else {
    return (uint8_t*)(fifo->fifo_ptr) + fifo->item_size * index;
  }
}
