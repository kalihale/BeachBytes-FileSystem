#ifndef __DATA_BLOCK_OPS__
#define __DATA_BLOCK_OPS__

#include <sys/types.h>

#include "common_config.h"

sType allocate_data_block();
char* read_data_block(sType index);
bool write_data_block(sType index, char* buffer);
bool free_data_block(sType index);
sType getFreeListLength();

#endif
