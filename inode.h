//
// Created by kali on 10/28/24.
//

#ifndef INODE_H
#define INODE_H
//#include <cstdint>
#include "common_config.h"
#include "data_block_operation.h"
#include "fileStructure.h"
#include "layerZero.h"

inodeStruct* loadINodeFromDisk(sType inodeNum);

bool writeINodeToDisk(sType inodeNum, inodeStruct* inode);

sType getNextFreeINode();

sType createInode();

bool free_indirect_block(sType blockNum, sType recurse);

bool free_block(inodeStruct* iNode);

bool delete_inode(sType inodeNum);
bool free_All_data_blocks_in_inode(inodeStruct* node);

sType get_datablock_from_inode(const inodeStruct* const node, sType find_blocknum, sType* prev_indirect_block);


bool add_datablock_to_inode(inodeStruct *inodeObj, const sType data_block_num);

bool add_directory_entry(inodeStruct** dir_inode, sType child_inum, char* file_name);

bool remove_from_directory(inodeStruct** dir_inode, char* file_name);
bool remove_datablocks_nested(inodeStruct* inodeObj, sType p_block_num, sType indirectionlevel);
bool remove_datablocks_range_from_inode(inodeStruct* inodeObj, sType logical_block_num);

sType get_last_index_of_parent_path(const char* const path, sType path_length);
bool copy_parent_path(char* const buffer, const char* const path, sType path_len);
bool copy_file_name(char* const buffer, const char* const path, sType path_len);
sType get_inode_of_File(const char* const file_path);


sType get_datablock_from_inode(const inodeStruct* const node, sType find_blocknum, sType* prev_indirect_block);


bool add_datablock_to_inode(inodeStruct *inodeObj, const sType data_block_num);

bool add_directory_entry(inodeStruct** dir_inode, sType child_inum, char* file_name);

bool remove_from_directory(inodeStruct** dir_inode, char* file_name);
bool remove_datablocks_nested(inodeStruct* inodeObj, sType p_block_num, sType indirectionlevel);
bool remove_datablocks_range_from_inode(inodeStruct* inodeObj, sType logical_block_num);

sType get_last_index_of_parent_path(const char* const path, sType path_length);
bool copy_parent_path(char* const buffer, const char* const path, sType path_len);
bool copy_file_name(char* const buffer, const char* const path, sType path_len);
sType get_inode_of_File(const char* const file_path);

#endif //INODE_H