#ifndef __LINKFUSE_FS__
#define __LINKFUSE_FS__

#include <sys/types.h>
#include <string.h>
#include "common_config.h"
#include "fileStructure.h"

bool bootUpFileSytem();

static void inode_to_stat(inodeStruct** node, struct stat** st);
sType fs_chmod(const char* path, mode_t mode);
bool fs_mknod(const char* path, mode_t mode, dev_t dev);
sType create_new_file(const char* const path, inodeStruct** buff, mode_t mode, sType* parent_inum);
sType fs_getattr(const char* path, struct stat** st);
bool fs_mkdir(const char* path, mode_t mode);
sType fs_unlink(const char* path);
bool is_empty_dir(inodeStruct** dir_inode);
sType fs_truncate(const char* path, off_t length);
sType fs_write(const char* path, const char* buff, size_t nbytes, off_t offset);
sType fs_readdir(const char* path, void* buff, fuse_fill_dir_t filler);
sType fs_openFile(const char* path, sType oflag);
sType fs_read(const char* path, char* buff, size_t nbytes, off_t offset);
sType fs_rename(const char *from, const char *to);
sType fs_access(const char* path);
void fs_destroy();
#endif