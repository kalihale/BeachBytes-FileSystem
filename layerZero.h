#ifndef __LAYER_ZERO__
#define __LAYER_ZERO__

#include "common_config.h"

static int64_t fs_ptr;
static char* fs_memory_ptr;
static bool inMemory = true;


bool fs_open();

bool fs_close();

bool fs_write_block(sType blockid, char *buffer);

bool fs_read_block(sType blockid, char *buffer);

bool fs_init();
bool fs_create_ilist();
bool fs_create_superblock();
bool fs_write_superblock();
bool load_FS();
void restartDisk();
#endif 
