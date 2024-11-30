#include "../header/common_config.h"
#include "../header/layerZero.h"
#include "../header/data_block_operation.h"
#include "../header/inode.h"

bool inodeNum_valid(sType inodeNum){
    return inodeNum >= 0 && inodeNum < INODE_BLOCK_COUNT;
}

void gen_block_offset(sType inodeNum, sType *block, sType *offset){
    *block = (inodeNum / INODES_PER_BLOCK) + 1; //+1 for super block;
    *offset = inodeNum % INODES_PER_BLOCK;
}

inodeStruct* loadINodeFromDisk(sType inodeNum){
    if(!inodeNum_valid(inodeNum)){
        return NULL;
    }

    sType block; 
    sType offset;

    gen_block_offset(inodeNum, &block, &offset);

    char buffer[BLOCK_SIZE];

    if(!fs_read_block(block, buffer)){
        printf("block read based on inode number falied\n\n");
        //How to throw an error in fuse
        return NULL;
    }

    inodeStruct* node= (inodeStruct*)buffer;
    node = node + offset;

    inodeStruct* iNode = (inodeStruct*)malloc(sizeof(inodeStruct));
    memcpy(iNode, node, sizeof(inodeStruct));
    if(!iNode->is_allocated){
        free(iNode);
        return NULL;
    }
    node = NULL;
    return iNode;

}

bool writeINodeToDisk(sType inodeNum, inodeStruct* inode){
    if(!inodeNum_valid(inodeNum)){
        return false;
    }

    sType block; 
    sType offset;
    gen_block_offset(inodeNum, &block, &offset);

    char buffer[BLOCK_SIZE];
    if(!fs_read_block(block, buffer)){
        //How to log an error in fuse
        return false;
    }

    inodeStruct* node= (inodeStruct*)buffer;
    node = node + offset;
    printf("WRITING TIME %d\n\n", inode->atime);

    memcpy(node, inode, sizeof(inodeStruct));

    if(!fs_write_block(block, buffer)){
        //How to log an error in fuse
        return false;
    }

    return true;

}

sType getNextFreeINode(){
    sType curBlock = 1;
    sType curOffset = 0;
    char buffer[BLOCK_SIZE];
    inodeStruct* blockNode;
    inodeStruct* curNode;

    while(curBlock< INODE_BLOCK_COUNT){
        if(!fs_read_block(curBlock, buffer)){
        //How to throw an error in fuse
            return -1;
        }
        blockNode = (inodeStruct*) buffer;
        while(curOffset < INODES_PER_BLOCK){
            curNode = blockNode + curOffset;
            if(!curNode->is_allocated){
                return (curBlock-1)*INODES_PER_BLOCK + curOffset;
                return (curBlock-1)*INODES_PER_BLOCK + curOffset;
            }
            curOffset++;

        }
        curOffset = 0;
        curBlock += 1;
    }

    return -1;
}

sType createInode(){
    sType inodeNum = getNextFreeINode();

    if(!inodeNum_valid(inodeNum)){
        return -1;
    }

    sType block;
    sType offset;
    gen_block_offset(inodeNum, &block, &offset);

    char buffer[BLOCK_SIZE];

    if(!fs_read_block(block, buffer)){
        //How to throw an error in fuse
        return -1;
    }

    inodeStruct* node = (inodeStruct*)(buffer + offset * sizeof(inodeStruct));
    node -> linkCount = 0;
    node -> is_allocated = true;

    if(!fs_write_block(block, buffer)){
        //How to throw an error in fuse
        return -1;
    }

    return inodeNum;

}

bool free_All_data_blocks_in_inode(inodeStruct* node)
{
    if(!remove_datablocks_range_from_inode(node, 0)){
        return false;
    }
    return true;
}

bool delete_inode(sType inodeNum){
    if(!inodeNum_valid(inodeNum)){
        return false;
    }

    sType block; 
    sType offset;
    gen_block_offset(inodeNum, &block, &offset);

    char buffer[BLOCK_SIZE];
    if(!fs_read_block(block, buffer)){
        //How to log an error in fuse
        return false;
    }

    inodeStruct* node= (inodeStruct*)buffer;
    node = node + offset;

    if(!free_All_data_blocks_in_inode(node))
    {
        return false;
    }

    node->deviceID = 0;
    node->fileSerialNum = 0;
    node->ownerPermission = 7;
    node->groupPermission=7;
    node->worldPermission=7;
    node->linkCount=0;
    node->fileSize=0;
    node->blocks=0;
    node->ctime=0;
    node->mtime=0;
    node->atime=0;
    for(sType i = 0; i < NUM_DIRECT_BLOCKS; i++)
        node->directAddresses[i] = 0;
    node->singleIndirect=0;
    node->doubleIndirect=0;
    node->tripleIndirect=0;
    node->is_allocated=false;
    node->filesCount=0; // number of files or directory in this inode
    if(!fs_write_block(block, buffer)){
        return false;
    }

    return true;
}

bool add_datablock_to_inode(inodeStruct *inodeObj, const sType data_block_num)
{
    // Add data block to inode after incrementing num of blocks in inode struct
    sType reference_blockId = inodeObj->blocks;

    // Follow same structure of translating to data blocks as in directory_ops
    if(reference_blockId < NUM_DIRECT_BLOCKS){
        inodeObj->directAddresses[reference_blockId] = data_block_num;
        inodeObj->blocks++;
        return true;
    }

    reference_blockId -= NUM_DIRECT_BLOCKS;

    if(reference_blockId < NUM_OF_SINGLE_INDIRECT_BLOCK_ADDR){
        
        if(reference_blockId == 0)
        {
            sType sInd_block_num = allocate_data_block();
            if(sInd_block_num == -1)
            {
                return false;
            }
            inodeObj->singleIndirect = sInd_block_num;
        }

        sType* sInd_block_arr = (sType*) read_data_block(inodeObj->singleIndirect);
        sInd_block_arr[reference_blockId] = data_block_num;
        
        if(!write_data_block(inodeObj->singleIndirect, (char*)sInd_block_arr))
        {
            return false;
        }

        free(sInd_block_arr);
        inodeObj->blocks++;
        return true;
    }

    // Adjust logical block number for double indirect
    reference_blockId -= NUM_OF_SINGLE_INDIRECT_BLOCK_ADDR;

    if(reference_blockId < NUM_OF_DOUBLE_INDIRECT_BLOCK_ADDR)
    {
        // if file block num == 12 + 512 => need a new double indirect block
        if(reference_blockId == 0)
        {
            sType dInd_block_num = allocate_data_block();
            if(dInd_block_num == -1)
            {
                return false;
            }
            inodeObj->doubleIndirect = dInd_block_num;
            
        }

        sType double_blockId = reference_blockId / NUM_OF_ADDRESSES_PER_BLOCK;
        sType inner_offset = reference_blockId % NUM_OF_ADDRESSES_PER_BLOCK;

        sType* dInd_block_arr = (sType*) read_data_block(inodeObj->doubleIndirect);

        // This is to create the first indirect block in the single indirect block
        if(inner_offset == 0)
        { 
            sType sInd_block = allocate_data_block();
            if(sInd_block == -1)
            {
                return false;
            }
            
            dInd_block_arr[double_blockId] = sInd_block;
            
            if(!write_data_block(inodeObj->doubleIndirect, (char*)dInd_block_arr))
            {
                return false;
            }
        }
        
        sType sInd_block_num = dInd_block_arr[double_blockId];
        free(dInd_block_arr);
        
        if(sInd_block_num <= 0)
        {
            return false;
        }

        sType* sInd_block_arr = (sType*) read_data_block(sInd_block_num);
        sInd_block_arr[inner_offset] = data_block_num;
        
        if(!write_data_block(sInd_block_num, (char*)sInd_block_arr))
        {
            return false;
        }
        
        free(sInd_block_arr);
        inodeObj->blocks++;
        return true;
    }

    // Adjust logical block number for triple indirect
    reference_blockId -= NUM_OF_DOUBLE_INDIRECT_BLOCK_ADDR;
    
    if (reference_blockId < NUM_OF_TRIPLE_INDIRECT_BLOCK_ADDR)
    {
        if(reference_blockId == 0){
            sType triple_data_block_num = allocate_data_block();
            if(triple_data_block_num == -1)
            {
               return false;
            }
            inodeObj->tripleIndirect = triple_data_block_num;
            
        }

        sType* tInd_block = (sType*) read_data_block(inodeObj->tripleIndirect);

        sType triple_blockId = reference_blockId / NUM_OF_DOUBLE_INDIRECT_BLOCK_ADDR;
        sType double_blockId = (reference_blockId / NUM_OF_ADDRESSES_PER_BLOCK) % NUM_OF_ADDRESSES_PER_BLOCK;
        sType inner_offset = reference_blockId % NUM_OF_ADDRESSES_PER_BLOCK;
        
        if(reference_blockId % NUM_OF_DOUBLE_INDIRECT_BLOCK_ADDR == 0){
            sType dInd_block_num = allocate_data_block();
            
            if(dInd_block_num == -1)
            {
                return false;
            }
            
            tInd_block[triple_blockId] = dInd_block_num;
            
            if(!write_data_block(inodeObj->tripleIndirect, (char *)tInd_block))
            {
                return false;
            }
        }
        
        sType* dInd_block = (sType*) read_data_block(tInd_block[triple_blockId]);
        
        if(inner_offset == 0){
            sType sInd_block_num = allocate_data_block();
            if(sInd_block_num == -1)
            {
                return false;
            }
            
            dInd_block[double_blockId] = sInd_block_num;
            
            if(!write_data_block(tInd_block[triple_blockId], (char*)dInd_block))
            {
                return false;
            }
        }
        free(tInd_block);

        sType sInd_block_num = dInd_block[double_blockId];
        free(dInd_block);
        
        if(sInd_block_num <= 0)
        {
            return false;
        }

        sType* sInd_block = (sType*) read_data_block(sInd_block_num);
        sInd_block[inner_offset] = data_block_num;

        if(!write_data_block(sInd_block_num, (char*)sInd_block))
        {
            return false;
        }
        free(sInd_block);
        inodeObj->blocks++;
        return true;
    }
    return false;
}

sType get_datablock_from_inode(const inodeStruct* const node, sType find_blocknum, sType* prev_indirect_block)
{
    sType data_block_num = -1;

    if(find_blocknum > node->blocks)
    {
        return data_block_num;
    }
    

    if(find_blocknum < NUM_DIRECT_BLOCKS)
    {
        data_block_num = node->directAddresses[find_blocknum];
        
        return data_block_num;
    }

    find_blocknum -= NUM_DIRECT_BLOCKS;

    if(find_blocknum < NUM_OF_SINGLE_INDIRECT_BLOCK_ADDR)
    {
        if(node->singleIndirect == 0)
        {
            return -1;
        }
        sType* sInd_block = (sType*) read_data_block(node->singleIndirect);
        data_block_num = sInd_block[find_blocknum];
        free(sInd_block);

        return data_block_num;
    }

    find_blocknum -= NUM_OF_SINGLE_INDIRECT_BLOCK_ADDR;
    
    if(find_blocknum < NUM_OF_DOUBLE_INDIRECT_BLOCK_ADDR)
    {
        if(node->doubleIndirect == 0)
        {
            return -1;
        }
        sType double_blockId = find_blocknum / NUM_OF_ADDRESSES_PER_BLOCK;
        sType inner_offset = find_blocknum % NUM_OF_ADDRESSES_PER_BLOCK;

        if(inner_offset != 0 && *prev_indirect_block != 0)
        {
            sType* sInd_block = (sType*) read_data_block(*prev_indirect_block);
            data_block_num = sInd_block[inner_offset];
            free(sInd_block);

            return data_block_num;
        }

        sType* dInd_block = (sType*) read_data_block(node->doubleIndirect);
        data_block_num = dInd_block[double_blockId];
        free(dInd_block);

        if(data_block_num <= 0){
            return -1;
        }
        *prev_indirect_block = data_block_num;

        sType* sInd_block = (sType*) read_data_block(data_block_num);
        data_block_num = sInd_block[inner_offset];
        free(sInd_block);
        
        return data_block_num;
    }

    find_blocknum -= NUM_OF_DOUBLE_INDIRECT_BLOCK_ADDR;

    if(node->tripleIndirect == 0)
    {
        return data_block_num;
    }

    sType triple_blockId = find_blocknum / NUM_OF_DOUBLE_INDIRECT_BLOCK_ADDR;
    sType double_blockId = (find_blocknum / NUM_OF_ADDRESSES_PER_BLOCK) % NUM_OF_ADDRESSES_PER_BLOCK;
    sType inner_offset = find_blocknum % NUM_OF_ADDRESSES_PER_BLOCK;

    if(inner_offset != 0 && *prev_indirect_block != 0)
    {
        sType* sInd_block = (sType*) read_data_block(*prev_indirect_block);
        data_block_num = sInd_block[inner_offset];
        free(sInd_block);

        return data_block_num;
    }

    sType* tInd_block = (sType*) read_data_block(node->tripleIndirect);
    data_block_num = tInd_block[triple_blockId];
    free(tInd_block);

    if(data_block_num <= 0)
    {
        return -1;
    }
    
    sType* dInd_block = (sType*) read_data_block(data_block_num);
    data_block_num = dInd_block[double_blockId];
    free(dInd_block);
    
    if(data_block_num<=0)
    {
        return -1;
    }
    *prev_indirect_block = data_block_num;

    sType* sInd_block = (sType*) read_data_block(data_block_num);
    data_block_num = sInd_block[inner_offset];
    free(sInd_block);

    return data_block_num;
}

bool add_directory_entry(inodeStruct** dir_inode, sType child_inum, char* file_name)
{
    if(!S_ISDIR((*dir_inode)->i_mode))
    {
        return false;
    }
    sType fileNameLen = strlen(file_name);
    if(fileNameLen > FILE_NAME_MAX_LENGTH)
    {
        return false;
    }
    fileNameLen++; 

    if((*dir_inode)->blocks > 0)
    {
        sType prev_block = 0;
        for(sType l_block_num = 0; l_block_num < (*dir_inode)->blocks; l_block_num++)
        {
            sType p_block_num = get_datablock_from_inode((*dir_inode), l_block_num, &prev_block);
            if(p_block_num <= 0)
            {
                return false;
            }

            char* assignBlock = read_data_block(p_block_num);
            sType curr_pos = sizeof(sType)+((sType*) assignBlock)[0];
            sType freeSpace = BLOCK_SIZE-curr_pos-RECORD_FIXED_LEN-sizeof(sType);
            if(freeSpace > fileNameLen){
                sType record_length = RECORD_FIXED_LEN + fileNameLen;
                char* record = assignBlock + curr_pos;


                memcpy(record, &record_length, sizeof(sType));
                memcpy(record + RECORD_LENGTH, &child_inum, sizeof(sType));
                ((sType*) assignBlock)[0]+=record_length;
                strncpy(record + RECORD_FIXED_LEN, file_name, fileNameLen);

                if(!write_data_block(p_block_num, assignBlock)){
                    free(assignBlock);
                    return false;
                }

                (*dir_inode)->filesCount++;
                free(assignBlock);
                return true;
            }
            free(assignBlock);
        }
    }

    sType data_block_num = allocate_data_block();
    if(data_block_num <= 0)
    {
        return false;
    }

    char data_block[BLOCK_SIZE];
    memset(data_block, 0, BLOCK_SIZE);

    sType record_length = RECORD_FIXED_LEN + fileNameLen;
    ((sType*) data_block)[0]=record_length;
    memcpy(data_block  + RECORD_LENGTH, &record_length, sizeof(sType));
    memcpy(data_block  + RECORD_LENGTH*2, &child_inum, sizeof(sType));

  
    strncpy((char*)(data_block + RECORD_FIXED_LEN+RECORD_LENGTH), file_name, fileNameLen);

    if(!write_data_block(data_block_num, data_block))
    {
        return false;
    }

    if(!add_datablock_to_inode((*dir_inode), data_block_num))
    {
        return false;
    }

    (*dir_inode)->fileSize += BLOCK_SIZE; 
    (*dir_inode)->filesCount++;
    return true;
}

bool remove_from_directory(inodeStruct** dir_inode, char* file_name)
{
    sType offset = -1;
    char * p_block = NULL;
    sType p_plock_num=-1;

    if (!S_ISDIR((*dir_inode)->i_mode))
    {
        return false;
    }

    sType file_name_len = strlen(file_name);
    if(file_name_len > FILE_NAME_MAX_LENGTH){
        return false;
    }

    sType prev_block = 0;
    for(sType l_block_num = 0; l_block_num < (*dir_inode)->blocks; l_block_num++)
    {
        p_plock_num = get_datablock_from_inode(*dir_inode, l_block_num, &prev_block);

        if(p_plock_num <= 0)
        {
            return false;
        }

        p_block = read_data_block(p_plock_num);
        if((sType*)p_block==0){
            free(p_block);
            continue;
        }
        sType curr_pos = sizeof(sType);
        while(curr_pos <= (BLOCK_SIZE-RECORD_FIXED_LEN))
        {
            sType record_len = ((sType*)(p_block + curr_pos))[0];
            char* curr_file_name = p_block + curr_pos + RECORD_FIXED_LEN;
            sType curr_file_name_len = ((sType)(record_len - RECORD_FIXED_LEN - 1));  // adjust for \0
            
            if (record_len == 0)
                break;

            // If the file name matches the input file name => we have found our file
            if (curr_file_name_len == strlen(file_name) && 
                strncmp(curr_file_name, file_name, curr_file_name_len) == 0) {
                offset = curr_pos;
                break;
            }
            else {
                curr_pos += record_len;
            }
        } 
        if(offset>0){
            break;
        }
        free(p_block);
    }
    if(offset==-1){
        return false;
    }

    char buffer[BLOCK_SIZE];
    memset(buffer, 0, BLOCK_SIZE);
    sType rec_len = ((sType*)(p_block + offset))[0];
    ((sType*)p_block)[0]-=rec_len;
    memcpy(buffer, p_block, offset);
    
    sType next_offset = offset + rec_len;
    if(next_offset < BLOCK_SIZE)
    {
        memcpy(buffer + offset, p_block + next_offset, BLOCK_SIZE - next_offset);
    }
    write_data_block(p_plock_num, buffer);
    free(p_block);

    time_t curr_time = time(NULL);
    (*dir_inode)->ctime = curr_time;
    (*dir_inode)->mtime = curr_time;
    (*dir_inode)->filesCount--;
    return true;
}


bool remove_datablocks_nested(inodeStruct* inodeObj, sType p_block_num, sType indirectionlevel)
{
    if (p_block_num <= 0)
    {
        return false;
    }

    sType *buffer = (sType*) read_data_block(p_block_num);
    for(sType i = 0; i < NUM_OF_SINGLE_INDIRECT_BLOCK_ADDR; i++)
    {
        // we have reached the end of the data in the block 
        if (buffer[i] == 0)
            break;
        switch (indirectionlevel)
        {
            case 1:
                free_data_block(buffer[i]);
                break;
            default: 
                remove_datablocks_nested(inodeObj, buffer[i], indirectionlevel-1);
                break;
        }
    }
    free_data_block(p_block_num);
    free(buffer);
    return true;
}

bool remove_datablocks_range_from_inode(inodeStruct* inodeObj, sType logical_block_num)
{
    if (logical_block_num > inodeObj->blocks)
    {
        printf("inodeNum invalid\n");
        return false;
    }

    sType ending_block_num = inodeObj->blocks;
    sType starting_block_num = logical_block_num;
    
    if (logical_block_num < NUM_DIRECT_BLOCKS)
    {
        for (sType i = logical_block_num; i < NUM_DIRECT_BLOCKS && i < ending_block_num; i++)
        {
            if (!free_data_block(inodeObj->directAddresses[i]))
            {
                return false;
            }
            inodeObj->directAddresses[i] = 0;
        }

        if (ending_block_num <= NUM_DIRECT_BLOCKS)
        {
            inodeObj->blocks = starting_block_num;
            return true;
        }

        if (!remove_datablocks_nested(inodeObj, inodeObj->singleIndirect, 1))
        {
            return false;
        }
        inodeObj->singleIndirect = 0;

        ending_block_num -= NUM_DIRECT_BLOCKS;

        if (ending_block_num <= NUM_OF_SINGLE_INDIRECT_BLOCK_ADDR)
        {
            inodeObj->blocks = starting_block_num;
            return true;
        }

        if (!remove_datablocks_nested(inodeObj, inodeObj->doubleIndirect, 2))
        {
            return false;
        }
        inodeObj->doubleIndirect = 0;
        // Adjusting for double indirect block
        ending_block_num -= NUM_OF_SINGLE_INDIRECT_BLOCK_ADDR;

        if (ending_block_num <= NUM_OF_DOUBLE_INDIRECT_BLOCK_ADDR)
        {
            inodeObj->blocks = starting_block_num;
            return true;
        }

        if (!remove_datablocks_nested(inodeObj, inodeObj->tripleIndirect, 3))
        {
            return false;
        }
        
        inodeObj->tripleIndirect = 0;
        inodeObj->blocks = starting_block_num;
        return true;
    }

    logical_block_num -= NUM_DIRECT_BLOCKS;
    
    if (logical_block_num < NUM_OF_SINGLE_INDIRECT_BLOCK_ADDR)
    {
        if(inodeObj->singleIndirect == 0)
        {
            return false;
        }

        
        sType* single_indirect_block_arr = (sType*) read_data_block(inodeObj->singleIndirect);
     
        ending_block_num -= NUM_DIRECT_BLOCKS;

        for(sType i = logical_block_num; i < ending_block_num && i < NUM_OF_SINGLE_INDIRECT_BLOCK_ADDR; i++)
        {
            if (!free_data_block(single_indirect_block_arr[i]))
            {
                return false;
            }
            single_indirect_block_arr[i] = 0; 
        }

        if(!write_data_block(inodeObj->singleIndirect, (char*)single_indirect_block_arr))
        {
            return false;
        }

        if (ending_block_num <= NUM_OF_SINGLE_INDIRECT_BLOCK_ADDR)
        {
            inodeObj->blocks = starting_block_num;
            return true;
        }

        if (!remove_datablocks_nested(inodeObj, inodeObj->doubleIndirect, 2))
        {
            return false;
        }
        inodeObj->doubleIndirect = 0;

        ending_block_num -= NUM_OF_SINGLE_INDIRECT_BLOCK_ADDR;

        if (ending_block_num <= NUM_OF_DOUBLE_INDIRECT_BLOCK_ADDR)
        {
            inodeObj->blocks = starting_block_num;
            return true;
        }

        if (!remove_datablocks_nested(inodeObj, inodeObj->tripleIndirect, 3))
        {
            return false;
        }
        
        inodeObj->tripleIndirect = 0;
        inodeObj->blocks = starting_block_num;
        return true;
    }

    
    logical_block_num -= NUM_OF_SINGLE_INDIRECT_BLOCK_ADDR;

    if (logical_block_num < NUM_OF_DOUBLE_INDIRECT_BLOCK_ADDR)
    {
        if(inodeObj->doubleIndirect == 0)
        {
            return false;
        }

        sType double_i_idx = logical_block_num / NUM_OF_ADDRESSES_PER_BLOCK;
        sType inner_idx = logical_block_num % NUM_OF_ADDRESSES_PER_BLOCK;

        sType* double_indirect_block_arr = (sType*) read_data_block(inodeObj->doubleIndirect);

        ending_block_num -= NUM_OF_SINGLE_INDIRECT_BLOCK_ADDR + NUM_DIRECT_BLOCKS;

        
        ending_block_num -= NUM_OF_SINGLE_INDIRECT_BLOCK_ADDR * double_i_idx;

        for(sType i = double_i_idx; i < NUM_OF_SINGLE_INDIRECT_BLOCK_ADDR; i++)
        {
            sType j = (i == double_i_idx) ? inner_idx : 0;

            sType data_block_num = double_indirect_block_arr[i];
            if(data_block_num <= 0)
            {
                return false;
            }

            sType* single_indirect_block_arr = (sType*) read_data_block(data_block_num);

            for(;j < ending_block_num && j < NUM_OF_SINGLE_INDIRECT_BLOCK_ADDR; j++) 
            {
                if (!free_data_block(single_indirect_block_arr[j]))
                {
                   return false;
                }
                single_indirect_block_arr[j] = 0; 
            }

            if(!write_data_block(data_block_num, (char*)single_indirect_block_arr))
            {
                return false;
            }

            free(single_indirect_block_arr);
            if (!(i == double_i_idx && inner_idx > 0))
            {
                if (!free_data_block(data_block_num))
                {
                    return false;
                }
            }
            
            if (j == ending_block_num)
                break;

            
            ending_block_num -= NUM_OF_SINGLE_INDIRECT_BLOCK_ADDR;

            //Updated COndition
            
        }
        free(double_indirect_block_arr);
        if(logical_block_num == 0){
            if (!free_data_block(inodeObj->doubleIndirect))
            {
                return false;
            }
            inodeObj->doubleIndirect = 0;
        }

        if (ending_block_num <= NUM_OF_DOUBLE_INDIRECT_BLOCK_ADDR)
        {
            inodeObj->blocks = starting_block_num;
            return true;
        }        
        
        if (!remove_datablocks_nested(inodeObj, inodeObj->tripleIndirect, 3))
        {
            return false;
        }
        inodeObj->tripleIndirect = 0;
        inodeObj->blocks = starting_block_num;
        //We need to free the double indirect block as well
        
        return true;
    }

    logical_block_num -= NUM_OF_DOUBLE_INDIRECT_BLOCK_ADDR;

    if (logical_block_num < NUM_OF_TRIPLE_INDIRECT_BLOCK_ADDR)
    {
        if(inodeObj->tripleIndirect == 0)
        {
            return false;
        }

        sType triple_i_idx = logical_block_num / NUM_OF_DOUBLE_INDIRECT_BLOCK_ADDR;
        sType double_i_idx = (logical_block_num / NUM_OF_ADDRESSES_PER_BLOCK) % NUM_OF_ADDRESSES_PER_BLOCK;
        sType inner_idx = logical_block_num % NUM_OF_ADDRESSES_PER_BLOCK;

        sType* triple_indirect_block_arr = (sType*) read_data_block(inodeObj->tripleIndirect);

        
        ending_block_num -= NUM_OF_DOUBLE_INDIRECT_BLOCK_ADDR + NUM_OF_SINGLE_INDIRECT_BLOCK_ADDR + NUM_DIRECT_BLOCKS;

        //TODO check the correctness

        ending_block_num -= (NUM_OF_DOUBLE_INDIRECT_BLOCK_ADDR * triple_i_idx) + (NUM_OF_SINGLE_INDIRECT_BLOCK_ADDR * double_i_idx); 
        //Iterating through the triple indirect block
        for(sType i = triple_i_idx; i < NUM_OF_SINGLE_INDIRECT_BLOCK_ADDR; i++)
        {
            sType data_block_num = triple_indirect_block_arr[i];
            if(data_block_num <= 0)
            {
                return false;
            }

            sType j = (i == triple_i_idx) ? double_i_idx : 0;
            sType* double_indirect_block_arr = (sType*) read_data_block(data_block_num);

            bool should_free_double_indirect = (j == 0);
            bool should_free_single_indirect, has_reached_end = false;
            //iterating through the double indirect block
            for(;j < NUM_OF_SINGLE_INDIRECT_BLOCK_ADDR; j++)
            {
                sType single_indirect_data_block_num = double_indirect_block_arr[j];
                if(single_indirect_data_block_num <= 0)
                {
                    return false;
                }
                sType k = (j == double_i_idx) ? inner_idx : 0;
                sType* single_indirect_block_arr = (sType*) read_data_block(single_indirect_data_block_num);
                
                should_free_single_indirect = (k == 0);
                // iterating through the single indirect block
                for(; k < NUM_OF_SINGLE_INDIRECT_BLOCK_ADDR && k < ending_block_num; k++) // TODO: Check if the bounds checking for i,j,k, are right
                {
                    if (!free_data_block(single_indirect_block_arr[k]))
                    {
                        return false;
                    }
                    single_indirect_block_arr[k] = 0; 
                }

                if(!write_data_block(single_indirect_data_block_num, (char*)single_indirect_block_arr))
                {
                    return false;
                }

                free(single_indirect_block_arr);

                // free single indirect block
                if (should_free_single_indirect)
                {
                    if (!free_data_block(single_indirect_data_block_num))
                    {
                        return false;
                    }
                }

                if (k == ending_block_num)
                {
                    has_reached_end = true;
                    break;
                }

                ending_block_num -= NUM_OF_SINGLE_INDIRECT_BLOCK_ADDR;

                
            }
            free(double_indirect_block_arr);
            if (should_free_double_indirect)
            {
                if (!free_data_block(data_block_num))
                {
                    return false;
                }
            }

            if (has_reached_end)
                break;

            
        }
        
        free(triple_indirect_block_arr);
        if(logical_block_num == 0){
            if (!free_data_block(inodeObj->tripleIndirect))
            {
                return false;
            }
            inodeObj->tripleIndirect = 0;
        }
        inodeObj->blocks = starting_block_num;
        return true;
    }
    return false;
}

sType get_last_index_of_parent_path(const char* const path, sType path_length)
{
    for(sType i = path_length-1 ; i >= 0 ; i--)
    {
        if (path[i] == '/' && i != path_length-1)
            return i;
    }
    return -1;
}

bool copy_parent_path(char* const buffer, const char* const path, sType path_len)
{
    sType index = get_last_index_of_parent_path(path, path_len);
    if(index == -1)
    {
        printf("IN FALSE CASE");
        buffer[0] = '/';
        buffer[1] = '\0';
        return true;
    }

    if(index == 0)
    {
        memcpy(buffer, path, 1);
        buffer[1] = '\0';
        return true;
    }

    memcpy(buffer, path, index);
    buffer[index] = '\0';
    return true;
} 

bool copy_file_name(char* const buffer, const char* const path, sType path_len)
{
    sType start_index = get_last_index_of_parent_path(path, path_len), end_index = path_len;;
    
    if(start_index == -1)
    {
        start_index = 0; 
    }
    else
    {
        start_index++; 
    }
    
    // remove trailing /
    if(path[end_index-1]=='/'){
        end_index--;
    }
    memcpy(buffer, path+start_index, end_index-start_index);
    buffer[end_index - start_index]='\0';
    return true;
}


sType get_inode_of_File(const char* const file_path)
{
    sType file_path_len = strlen(file_path);

    if (file_path_len == 1 && file_path[0] == '/')
    {
       return ROOT_DIR_INODE_NUM;
    }

    char parent_path[file_path_len + 1];
    if (!copy_parent_path(parent_path, file_path, file_path_len))
        return -1;
    
    sType parent_inum = get_inode_of_File(parent_path);
    if (parent_inum == -1)
    {
        return -1;
    }

    inodeStruct* inodeObj = loadINodeFromDisk(parent_inum);
    if(!inodeObj){return -1;}
    char child_path[file_path_len + 1];
    if(!copy_file_name(child_path, file_path, file_path_len)){
        free(inodeObj);
        return -1;
    }


    sType offset = -1;
    char * p_block = NULL;
    sType p_plock_num=-1;

    if (!S_ISDIR((inodeObj)->i_mode))
    {
        free(inodeObj);
        return -1;
    }

    sType file_name_len = strlen(child_path);
    if(file_name_len > FILE_NAME_MAX_LENGTH){
        return -1;
    }

    sType prev_block = 0;
    for(sType l_block_num = 0; l_block_num < (inodeObj)->blocks; l_block_num++)
    {
        p_plock_num = get_datablock_from_inode(inodeObj, l_block_num, &prev_block);

        if(p_plock_num <= 0)
        {
            free(inodeObj);
            return -1;
        }

        p_block = read_data_block(p_plock_num);
        if(((sType*)p_block)[0]==0){
            free(p_block);
            continue;
        }
        sType curr_pos = sizeof(sType);
        while(curr_pos <= (BLOCK_SIZE-RECORD_FIXED_LEN))
        {
            sType record_len = ((sType*)(p_block + curr_pos))[0];
            char* curr_file_name = p_block + curr_pos + RECORD_FIXED_LEN;
            sType curr_file_name_len = ((sType)(record_len - RECORD_FIXED_LEN - 1));  // adjust for \0
            
            if (record_len == 0)
                break;

            // If the file name matches the input file name => we have found our file
            if (curr_file_name_len == strlen(child_path) && 
                strncmp(curr_file_name, child_path, curr_file_name_len) == 0) {
                offset = curr_pos;
                break;
            }
            else {
                curr_pos += record_len;
            }
        } 
        if(offset>0){
            break;
        }
        free(p_block);
    }
    if(offset==-1){
        return -1;
    }

    free(inodeObj);
    
    if(offset == -1){
        free(p_block);
        return -1;
    }

    sType inum = ((sType*) (p_block + offset + RECORD_LENGTH))[0];
    
    free(p_block);
    
    return inum;
}

//TESTING PURPOSES ONLY
bool directory_contains_entry(inodeStruct* dir_inode, const char* file_name) {
    if (!S_ISDIR(dir_inode->i_mode)) {
        return false;
    }

    sType fileNameLen = strlen(file_name);
    if (dir_inode->blocks <= 0) {
        return false;
    }

    sType prev_block = 0;
    for (sType l_block_num = 0; l_block_num < dir_inode->blocks; l_block_num++) {
        sType p_block_num = get_datablock_from_inode(dir_inode, l_block_num, &prev_block);
        if (p_block_num <= 0) {
            return false;
        }

        char* assignBlock = read_data_block(p_block_num);
        sType pos = sizeof(sType);
        sType block_size = ((sType*) assignBlock)[0];

        while (pos < block_size) {
            sType record_length = ((sType*)(assignBlock + pos))[0];
            sType entry_inum = ((sType*)(assignBlock + pos + RECORD_LENGTH))[0];
            char* entry_name = (char*)(assignBlock + pos + RECORD_FIXED_LEN);

            if (strncmp(entry_name, file_name, fileNameLen) == 0) {
                free(assignBlock);
                return true;
            }

            pos += record_length; 
        }

        free(assignBlock);
    }

    return false; 
}