#ifndef __LAYER_ZERO__
#define __LAYER_ZERO__

#include "common_config.h"
#include "fileStructure.h"

static int64_t fs_ptr;

static struct superblock* fs_superblock = NULL;

bool fs_open();

bool fs_close();

bool fs_write_block(sType blockid, char *buffer);

bool fs_read_block(sType blockid, char *buffer);

bool fs_init();
bool fs_create_ilist();
bool fs_create_superblock();
bool fs_write_superblock();
#endif 
