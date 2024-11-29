#include "../header/layerZero.h"
#include "../header/common_config.h"
#include "../header/data_block_operation.h"
#include "../header/inode.h"
#include "../header/linkFuseAndFS.h"
#include <time.h>
#include <iostream>
#include <unistd.h>

using namespace std;

//#include <string>
sType ROOT_DIR_INODE_NUM=0;

bool bootUpFileSytem()
{
    printf("starting loadFs\n");
   load_FS();
    printf("done with loadFs %lu\n",ROOT_DIR_INODE_NUM);
   //TODO check if this is ever return null??
   // Check for root directory
    inodeStruct* root_dir = loadINodeFromDisk(ROOT_DIR_INODE_NUM);
    
    //printf("got root dir %d %d\n",root_dir->is_allocated,root_dir->filesCount);
    if(root_dir && root_dir->is_allocated && root_dir->filesCount >= 2)
    {
        return true;
    }

    ROOT_DIR_INODE_NUM=createInode();
    root_dir = loadINodeFromDisk(ROOT_DIR_INODE_NUM);
    if(!root_dir){
        printf("cannot get root inode\n\n");
        return false;
    }
    printf("%d rootInode\n\n",ROOT_DIR_INODE_NUM);
    //not found
    time_t curr_time = time(NULL);

    root_dir->is_allocated = true;
    root_dir->linkCount=1;
    root_dir->i_mode = S_IFDIR | DEFAULT_PERMISSIONS;
    root_dir->blocks = 0;
    root_dir->fileSize = 0;
    root_dir->atime = curr_time;
    root_dir->ctime = curr_time;
    root_dir->mtime = curr_time;
    root_dir->filesCount=0;

    string dir_name = ".";
    if(!add_directory_entry(&root_dir, ROOT_DIR_INODE_NUM,(char*) dir_name.data())){
        return false;
    }
    printf("%lu %lu %lu %lu %lu :blocks number\n",root_dir->fileSize,root_dir->blocks,INODE_BLOCK_COUNT,BLOCK_COUNT,INODES_PER_BLOCK);
    dir_name = "..";
    if(!add_directory_entry(&root_dir, ROOT_DIR_INODE_NUM,(char*) dir_name.data())){
        return false;
    }

    printf("%lu %lu :blocks number\n",root_dir->fileSize,root_dir->blocks);
    if(!writeINodeToDisk(ROOT_DIR_INODE_NUM, root_dir)){
        return false;
    }
    
    free(root_dir);

    return true;
}


/*
struct stat {
    dev_t     st_dev;         ID of device containing file
    ino_t     st_ino;         Inode number
    mode_t    st_mode;        File type and mode
    nlink_t   st_nlink;       Number of hard links
    uid_t     st_uid;         User ID of owner
    gid_t     st_gid;         Group ID of owner
    dev_t     st_rdev;        Device ID (if special file)
    off_t     st_size;        Total size, in bytes
    blksize_t st_blksize;     Block size for filesystem I/O
    blkcnt_t  st_blocks;      Number of 512B blocks allocated
}
*/
static void inode_to_stat(inodeStruct** node, struct stat** st){
    (*st)->st_mode = (*node)->i_mode;
    (*st)->st_nlink = (*node)->linkCount;
    (*st)->st_uid = getgid(); 
    (*st)->st_gid = getgid(); 
    (*st)->st_rdev = 0;
    (*st)->st_size = (*node)->fileSize;
    (*st)->st_blksize = BLOCK_SIZE;
    (*st)->st_blocks = (*node)->blocks;
    (*st)->st_atime = (*node)->atime;
    (*st)->st_ctime = (*node)->ctime;
    (*st)->st_mtime = (*node)->mtime;
    printf("%d %d %lu %lu :stat\n",(*node)->i_mode,(*node)->linkCount,(*node)->fileSize,(*node)->blocks);
}


sType fs_chmod(const char* path, mode_t mode)
{
    sType inum = get_inode_of_File(path);
    if(inum == -1)
    {
        return -ENOENT;
    }

    inodeStruct* node = loadINodeFromDisk(inum);
    if(node == NULL)
    {
        return -1;
    }

    node->i_mode = mode;
    if(!writeINodeToDisk(inum, node))
    {
        return -1;
    }
    return 0;
}

bool fs_mknod(const char* path, mode_t mode, dev_t dev)
{
    dev = 0; // Silence error until used
    inodeStruct* node = NULL;

    sType inum = get_inode_of_File(path);
    if(inum != -1)
    {
        return false;
    }

    sType parent_inum;
    inum = create_new_file(path, &node, mode, &parent_inum);
    printf("INODE NUM %d\n\n", inum);

    if (inum <= -1)
    {
        free(node);
        return false;
    }

    free(node);
    return true;
}

sType create_new_file(const char* const path, inodeStruct** buff, mode_t mode, sType* parent_inum)
{
    printf("create_new_file has been called");
    // getting parent path 
    sType path_len = strlen(path);
    char parent_path[path_len+1];
    if(!copy_parent_path(parent_path, path, path_len))
    {
        return -ENOENT;
    }

    sType parent_inode_num = get_inode_of_File(parent_path);
    if(parent_inode_num == -1)
    {
        return -ENOENT;
    }

    inodeStruct* parent_inode = loadINodeFromDisk(parent_inode_num);
    // Check if parent is a directory
    if(!parent_inode || !S_ISDIR(parent_inode->i_mode))
    {
        if(parent_inode)
            free(parent_inode);
        return -ENOENT;
    }
    // Check parent write permission
    if(!(bool)(parent_inode->i_mode & S_IWUSR))
    {
        free(parent_inode);
        return -EACCES;
    }

    char child_name[path_len+1];
    if(!copy_file_name(child_name, path, path_len))
    {
        return -1;
    }

    sType child_inode_num = createInode();
    if(child_inode_num == -1)
    {
        free(parent_inode);
        return -EDQUOT;
    }
    if(!add_directory_entry(&parent_inode, child_inode_num, child_name))
    {
        delete_inode(child_inode_num);
        free(parent_inode);
        return -EDQUOT;
    }

    time_t curr_time = time(NULL);
    parent_inode->mtime = curr_time;
    parent_inode->ctime = curr_time;
    if(!writeINodeToDisk(parent_inode_num, parent_inode))
    {
        free(parent_inode);
        return -1;
    }

    *buff = loadINodeFromDisk(child_inode_num);
    if(!*buff){
        return -1;
    }
    (*buff)->linkCount = 1;
    (*buff)->i_mode = mode;
    (*buff)->blocks = 0;
    (*buff)->fileSize = 0;
    (*buff)->atime = curr_time;
    (*buff)->mtime = curr_time;
    (*buff)->ctime = curr_time;
    (*buff)->filesCount = 0;

    if(!writeINodeToDisk(child_inode_num, *buff))
    {
        free(*buff);
        return -1;
    }

    free(parent_inode);
    *parent_inum = parent_inode_num;
    return child_inode_num;
}

sType fs_getattr(const char* path, struct stat** st)
{
    memset(*st, 0, sizeof(struct stat));
    sType inum = get_inode_of_File(path);
    if(inum == -1)
    {
        printf("inum<0: %d for path inside fs_getattr\n\n",inum);
        return -ENOENT;
    }

    inodeStruct* node = loadINodeFromDisk(inum);
    if(node == NULL)
    {
        printf("empty node\n\n");
        return -EIO;
    }

    inode_to_stat(&node, st);
    (*st)->st_ino = inum;
    free(node);

    return 0;
}


bool fs_mkdir(const char* path, mode_t mode)
{
    printf("fs_mkdir called");
    inodeStruct* dir_inode = NULL;
    sType parent_inum;

    sType inum = get_inode_of_File(path);
    if(inum != -1)
    {
        return false;
    }

    sType dir_inode_num = create_new_file(path, &dir_inode, S_IFDIR|mode, &parent_inum);
    if(dir_inode_num <= -1)
    {
        free(dir_inode);
        return false;
    }
    
    string name = ".";
    // TODO error on name.data()
    if(!add_directory_entry(&dir_inode, dir_inode_num, name.data()))
    {
        free(dir_inode);
        return false;
    }

    name = "..";
    // TODO error on name.data()
    if(!add_directory_entry(&dir_inode, parent_inum, name.data()))
    {
        free(dir_inode);
        return false;
    }

    if(!writeINodeToDisk(dir_inode_num, dir_inode))
    {
        free(dir_inode);
        return false;
    }
    free(dir_inode);
    return true;
}

bool is_empty_dir(inodeStruct** dir_inode){
    if((*dir_inode)->filesCount > 2)
    {
        return false;
    }
    return true;
}

sType fs_unlink(const char* path)
{
    sType inum = get_inode_of_File(path);
    
    printf("UNLINK CALLED for path %s\n", path);
    printf("ATTEMPTING TO DELETE inode %d", inum);
    
    
    if(inum == -1)
    {
        return -ENOENT;
    }
    if(inum == ROOT_DIR_INODE_NUM)
    {
        return -EACCES;
    }

    sType path_len = strlen(path);
    char child_name[path_len + 1];
    if(!copy_file_name(child_name, path, path_len))
    {
        return -1;
    }

    char parent_path[path_len + 1];
    if(!copy_parent_path(parent_path, path, path_len))
    {
        return -EINVAL;
    }

    inodeStruct* node = loadINodeFromDisk(inum);
    if(!node)return -EINVAL;
    if(S_ISDIR(node->i_mode) && !is_empty_dir(&node))
    {
        free(node);
        return -ENOTEMPTY;
    }

    sType parent_inum = get_inode_of_File(parent_path);
    if(parent_inum == -1)
    {
        return -1;
    }
    inodeStruct* parent = loadINodeFromDisk(parent_inum);
    if(!parent)return -ENOTEMPTY;
    if(!remove_from_directory(&parent, child_name))
    {
        free(node);
        free(parent);
        return -1;
    }

    node->linkCount--;
    if(node->linkCount == 0)
    {
        delete_inode(inum);
    } else
    {
        writeINodeToDisk(inum, node);
    }
    writeINodeToDisk(parent_inum, parent);
    free(parent);
    free(node);
    printf(" unlink : Deleted file %s\n", path);
    return 0;
}


sType fs_truncate(const char* path, off_t length)
{
    sType inum = get_inode_of_File(path);
    if(inum == -1)
    {
       return -ENOENT;
    }
    inodeStruct* node = loadINodeFromDisk(inum);
    if(!node){return -EACCES; }
    if(!(bool)(node->i_mode & S_IWUSR))
    {
        return -EACCES;
    }

    if(length < 0)
    {
        return -EINVAL;
    }
    
    if(length == 0 && node->fileSize == 0)
    {
        free(node);
        return 0;
    }

    if(length > node->fileSize)
    {
        sType bytes_to_add = length - node->fileSize;
        char* zeros = (char*)calloc(1, bytes_to_add);
        sType written = fs_write(path, zeros, bytes_to_add, node->fileSize);
        free(node);
        if(written == bytes_to_add)
        {
            return 0;
        } else
        {
            return -1;
        }
    }

    sType i_block_num = (length == 0) ? -1 : (sType)((length - 1) / BLOCK_SIZE);
    if(node->blocks > i_block_num + 1)
    {
        // Remove everything from i_block_num + 1
        remove_datablocks_range_from_inode(node, i_block_num + 1);
    }

    if(i_block_num == -1)
    {
        node->fileSize = 0;
        writeINodeToDisk(inum, node);
        free(node);
        return 0;
    }

    sType prev = 0;
    sType d_block_num = get_datablock_from_inode(node, i_block_num, &prev);
    if(d_block_num <= 0)
    {
        return -1;
    }
    char* data_block = read_data_block(d_block_num);

    sType block_offset = (sType)((length - 1) % BLOCK_SIZE);
    if(block_offset != BLOCK_SIZE - 1)
    {
        memset(data_block + block_offset + 1, 0, BLOCK_SIZE - block_offset - 1);
        write_data_block(d_block_num, data_block);
    }
    free(data_block);

    node->fileSize = (sType)length;
    writeINodeToDisk(inum, node);
    free(node);
    return 0;
}

sType fs_write(const char* path, const char* buff, size_t nbytes, off_t offset)
{
    printf("Write attempt: path=%s, nbytes=%zu, offset=%lld\n", path, nbytes, (long long)offset);
    printf("WRITE CONTENT %.*s\n\n", (int)nbytes, buff);
    
    if(nbytes == 0)
    {
        return 0;
    }

    if(offset < 0)
    {
        return -EINVAL;
    }

    sType inum = get_inode_of_File(path);
    if (inum == -1)
    {
        return -ENOENT;
    }

    inodeStruct* node = loadINodeFromDisk(inum);
    if(!node){
        return -1;
    }
    printf("Current file size: %lld, Blocks: %lld\n", (long long)node->fileSize, (long long)node->blocks);
    size_t bytes_written = 0;

    sType start_block = (sType)(offset / BLOCK_SIZE);
    sType end_block = (sType)((offset + nbytes - 1) / BLOCK_SIZE);
    sType start_block_offset = (sType)(offset % BLOCK_SIZE);
    sType end_block_offset = (sType)((offset + nbytes - 1) % BLOCK_SIZE);

    sType existing_blocks = node->blocks;
    sType num_blocks_needed = end_block + 1;
    sType blocks_to_allocate = (num_blocks_needed > existing_blocks) ? (num_blocks_needed - existing_blocks) : 0;

    sType prev_block = 0;

    for(sType i = start_block; i <= end_block; i++)
    {
        sType dblock_num;
        char* buf_read = NULL;

        if(i < existing_blocks)
        {
            // Existing block
            dblock_num = get_datablock_from_inode(node, i, &prev_block);
            if(dblock_num <= 0)
            {
                free(node);
                return (bytes_written == 0) ? -1 : bytes_written;
            }
            buf_read = read_data_block(dblock_num);
            if(buf_read == NULL)
            {
                free(node);
                return (bytes_written == 0) ? -1 : bytes_written;
            }
        }
        else
        {
            // Need to allocate a new block
            dblock_num = allocate_data_block();
            if(dblock_num <= 0)
            {
                free(node);
                return (bytes_written == 0) ? -1 : bytes_written;
            }
            if(!add_datablock_to_inode(node, dblock_num))
            {
                free(node);
                return (bytes_written == 0) ? -1 : bytes_written;
            }
            buf_read = (char*)calloc(BLOCK_SIZE, sizeof(char));
            if(buf_read == NULL)
            {
                free(node);
                return (bytes_written == 0) ? -ENOMEM : bytes_written;
            }
            node->blocks++;
        }

        sType block_offset = 0;
        sType to_write = 0;

        if(i == start_block && i == end_block)
        {
            // Single block write
            block_offset = start_block_offset;
            to_write = nbytes;
        }
        else if(i == start_block)
        {
            // Start block
            block_offset = start_block_offset;
            to_write = BLOCK_SIZE - start_block_offset;
        }
        else if(i == end_block)
        {
            // End block
            block_offset = 0;
            to_write = end_block_offset + 1;
        }
        else
        {
            // Middle block
            block_offset = 0;
            to_write = BLOCK_SIZE;
        }

        // Adjust to_write if remaining bytes are less
        if(to_write > nbytes - bytes_written)
        {
            to_write = nbytes - bytes_written;
        }

        memcpy(buf_read + block_offset, buff + bytes_written, to_write);
        bytes_written += to_write;

        bool written = write_data_block(dblock_num, buf_read);
        free(buf_read);
        if(!written)
        {
            free(node);
            return (bytes_written == 0) ? -1 : bytes_written;
        }
        printf("Block %lld data: %.*s\n", (long long)i, (int)to_write, buf_read + block_offset);

    }

    sType new_file_size = offset + bytes_written;
    if(new_file_size > node->fileSize)
    {
        node->fileSize = new_file_size;
    }

    if(bytes_written > 0)
    {
        time_t curr_time= time(NULL);
        node->mtime = curr_time;
    }

    if(!writeINodeToDisk(inum, node))
    {
        free(node);
        return -1;
    }
    free(node);
    return bytes_written;
}


sType fs_readdir(const char* path, void* buff, fuse_fill_dir_t filler)
{
    sType inum = get_inode_of_File(path);
    if(inum == -1)
    {
        return -ENOENT;
    }

    inodeStruct* node = loadINodeFromDisk(inum);
    if(!node){return -ENOTDIR;}
    if(!S_ISDIR(node->i_mode))
    {
        return -ENOTDIR;
    }

    sType num_blocks = node->blocks;
    sType prev = 0;
    for(sType i_block_num = 0; i_block_num < num_blocks; i_block_num++)
    {
        sType dblock_num = get_datablock_from_inode(node, i_block_num, &prev);
        char* dblock = read_data_block(dblock_num);
        
        sType offset = sizeof(uint64_t);
        while(offset < (BLOCK_SIZE - RECORD_FIXED_LEN))
        {
            char* record = dblock + offset;
            unsigned short rec_len = ((unsigned short*)record)[0];
            sType file_inum = ((sType*)(record + RECORD_LENGTH))[0];
            unsigned short name_len = rec_len - RECORD_FIXED_LEN;
            if(rec_len == 0)
                break;

            char file_name[name_len];
            memcpy(file_name, record + RECORD_FIXED_LEN, name_len);

            inodeStruct* file_inode = loadINodeFromDisk(file_inum);
            if(!file_inode){return -ENOTDIR; }
            struct stat stbuff_data;
            memset(&stbuff_data, 0, sizeof(struct stat));
            struct stat *stbuff = &stbuff_data;
            inode_to_stat(&file_inode, &stbuff);
            stbuff->st_ino = file_inum;
            // TODO FUSE_FILL_DIR_DEFAULTS undefined
            filler(buff, file_name, stbuff, 0, FUSE_FILL_DIR_DEFAULTS);  
            free(file_inode);

            record = NULL;
            offset += rec_len;
        }
        free(dblock);
    }
    free(node);

    return 0;
}

sType fs_openFile(const char* path, sType oflag)
{
    sType inum = get_inode_of_File(path);
    bool created = false;

    // Create new file if required
    if(inum == -1 && (oflag & O_CREAT))
    {
        inodeStruct* node = NULL;
        sType parent_inum;
        inum = create_new_file(path, &node, S_IFREG|DEFAULT_PERMISSIONS, &parent_inum);
        if(inum <= -1)
        {
            return inum;
        }
        // writeINodeToDisk(inum, node);
        free(node);
        created = true;
    }
    else if(inum == -1)
    {
        return -ENOENT;
    }

    // Permissions check
    inodeStruct* node = loadINodeFromDisk(inum);
    if(!node){return -EACCES;}
    if((oflag & O_RDONLY) || (oflag & O_RDWR)) // Needs read permission
    {
        if(!(bool)(node->i_mode & S_IRUSR))
        {
            return -EACCES;
        }
    }
    if((oflag & O_WRONLY) || (oflag & O_RDWR)) // Needs write permission
    {
        if(!(bool)(node->i_mode & S_IWUSR))
        {
            return -EACCES;
        }
    }

    // Truncate if required
    if((bool)(oflag & O_TRUNC) && (bool)(node->i_mode & S_IWUSR))
    {
        if(fs_truncate(path, 0) == -1)
        {
            return -1;
        }
        // If existing file, update times.
        if(!created)
        {
            time_t curr_time = time(NULL);
            node->ctime = curr_time;
            node->mtime = curr_time;
            writeINodeToDisk(inum, node);
        }
    }
    free(node);
    return inum;
}


sType fs_read(const char* path, char* buff, size_t nbytes, off_t offset)
{
    printf("Read attempt: path=%s, nbytes=%zu, offset=%lld\n", path, nbytes, (long long)offset);

    if (nbytes == 0)
    {
        return 0;
    }
    if (offset < 0)
    {
        return -EINVAL;
    }

    sType inum = get_inode_of_File(path);
    if (inum == -1)
    {
        return -ENOENT;
    }

    inodeStruct* node = loadINodeFromDisk(inum);
    if (!node)
    {
        return -ENOENT;
    }
    printf("Current file size: %lld, Blocks: %lld\n", (long long)node->fileSize, (long long)node->blocks);

    if (node->fileSize == 0)
    {
        free(node);
        return 0;
    }
    if (offset >= node->fileSize)
    {
        free(node);
        return 0; // Return 0 to indicate EOF
    }

    if (offset + nbytes > node->fileSize)
    {
        nbytes = node->fileSize - offset;
    }

    sType start_i_block = offset / BLOCK_SIZE;          // First logical block to read from
    sType start_block_offset = offset % BLOCK_SIZE;     // Starting offset in first block
    sType end_i_block = (offset + nbytes - 1) / BLOCK_SIZE; // Last logical block to read from
    sType end_block_offset = (offset + nbytes - 1) % BLOCK_SIZE; // Ending offset in last block

    sType blocks_to_read = end_i_block - start_i_block + 1; // Number of blocks to read

    size_t bytes_read = 0;
    sType prev_block = 0;

    for (sType i = 0; i < blocks_to_read; i++)
    {
        sType current_block = start_i_block + i;
        sType dblock_num = get_datablock_from_inode(node, current_block, &prev_block);
        if (dblock_num <= 0)
        {
            free(node);
            return -EIO; 
        }

        char* buf_read = read_data_block(dblock_num);
        if (buf_read == NULL)
        {
            free(node);
            return -EIO; 
        }

        sType bytes_to_read;
        sType block_offset;
        if (i == 0)
        {
            block_offset = start_block_offset;
            bytes_to_read = BLOCK_SIZE - start_block_offset;
        }
        else if (i == blocks_to_read - 1)
        {
            block_offset = 0;
            bytes_to_read = end_block_offset + 1;
        }
        else
        {
            block_offset = 0;
            bytes_to_read = BLOCK_SIZE;
        }

        if (bytes_to_read > nbytes - bytes_read)
        {
            bytes_to_read = nbytes - bytes_read;
        }

        printf("Reading from block %lld: dblock_num=%lld, block_offset=%lld, bytes_to_read=%lld\n",
               (long long)current_block, (long long)dblock_num, (long long)block_offset, (long long)bytes_to_read);

        memcpy(buff + bytes_read, buf_read + block_offset, bytes_to_read);
        bytes_read += bytes_to_read;

        printf("Data read from block %lld: %.*s\n", (long long)current_block, (int)bytes_to_read, buf_read + block_offset);

        free(buf_read);

        if (bytes_read >= nbytes)
        {
            break;
        }
    }

    node->atime = time(NULL);
    if (!writeINodeToDisk(inum, node))
    {
        return -1;
    }
    free(node);
    printf("Final read buffer: %.*s\n", (int)bytes_read, buff);

    return bytes_read;
}

sType fs_access(const char* path)
{
    sType inum = get_inode_of_File(path);
    if(inum == -1)
    {
        return -ENOENT;
    }
    // TODO: Check Permissions
    return 0;
}


sType fs_rename(const char *from, const char *to)
{
    if(fs_access(from) != 0)
    {
        return -ENOENT;
    }
    // check if to exists
    if(fs_access(to) == 0)
    {
        // to exists, try to unlink it
        sType unlink_res = fs_unlink(to);
        if(unlink_res != 0)
        {
            return unlink_res;
        }
    }
    
    sType to_path_len = strlen(to);
    char to_parent_path[to_path_len + 1];
    if(!copy_parent_path(to_parent_path, to, to_path_len))
    {
        return -1;
    }

    // Check for to's parent sanity.
    sType to_parent_inode_num = get_inode_of_File(to_parent_path);
    if(to_parent_inode_num == -1)
    {
        return -1;
    }
    inodeStruct* to_parent_inode = loadINodeFromDisk(to_parent_inode_num);
    if(!to_parent_inode){return -1;}
    if(!S_ISDIR(to_parent_inode->i_mode))
    {
        free(to_parent_inode);
        return -1;
    }
    
    char to_child_name[to_path_len + 1];
    if(!copy_file_name(to_child_name, to, to_path_len))
    {
        return -1;
    }

    sType inum = get_inode_of_File(from);
    if(!add_directory_entry(&to_parent_inode, inum, to_child_name))
    {
        free(to_parent_inode);
        return -EDQUOT;
    }
    time_t curr_time = time(NULL);
    to_parent_inode->mtime = curr_time;
    to_parent_inode->ctime = curr_time;
    if(!writeINodeToDisk(to_parent_inode_num, to_parent_inode))
    {
        free(to_parent_inode);
        return -1;
    }
    free(to_parent_inode);

    sType from_path_len = strlen(from);
    char from_parent_path[from_path_len + 1];
    if(!copy_parent_path(from_parent_path, from, from_path_len))
    {
        return -1;
    }

    sType from_parent_inode_num = get_inode_of_File(from_parent_path);
    if(from_parent_inode_num == -1)
    {
        return -1;
    }
    inodeStruct* from_parent_inode = loadINodeFromDisk(from_parent_inode_num);
    if(!from_parent_inode){return -1;}
    char from_child_name[from_path_len + 1];
    if(!copy_file_name(from_child_name, from, from_path_len))
    {
        return -1;
    }

    if(!remove_from_directory(&from_parent_inode, from_child_name))
    {
        free(from_parent_inode);
        return -1;
    }
    from_parent_inode->mtime = curr_time;
    from_parent_inode->ctime = curr_time;
    if(!writeINodeToDisk(from_parent_inode_num, from_parent_inode))
    {
        free(from_parent_inode);
        return -1;
    }
    free(from_parent_inode);

    return 0;
}

void fs_destroy()
{
    restartDisk();
}
