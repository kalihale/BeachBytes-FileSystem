#define FUSE_USE_VERSION 35

#include <fuse.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <errno.h>
#include <ctype.h>
#include <stddef.h>
#include <dirent.h>
#include <assert.h>

#include "../header/fuse_interface.h"

#include "../header/data_block_operation.h"
#include "../header/layerZero.h"
#include "../header/inode.h"
#include "../header/linkFuseAndFS.h"
#define FIOC_NAME	"fioc"

enum {
	FIOC_NONE,
	FIOC_ROOT,
	FIOC_FILE,
};

static void *fioc_buf;
static size_t fioc_size;
bool debug = true;

// static int fioc_resize(size_t new_size)
// {
// 	void *new_buf;
// 	if (new_size == fioc_size)
// 		return 0;

// 	new_buf = (void *)realloc(fioc_buf, new_size);
// 	if (!new_buf && new_size)
// 		return -ENOMEM;

// 	if (new_size > fioc_size)
// 		memset(new_buf + fioc_size, 0, new_size - fioc_size);

// 	fioc_buf = new_buf;
// 	fioc_size = new_size;

// 	return 0;
// }

static int fioc_expand(size_t new_size)
{
	// if (new_size > fioc_size)
	// 	return fioc_resize(new_size);
	return 0;
}


static int fioc_getattr(const char *path, struct stat *stbuf,
			struct fuse_file_info *fi)
{
    if (debug)
	    printf("Getting attribute for path %s\n", path);

	// stbuf->st_uid = getuid();
	// stbuf->st_gid = getgid();
	// stbuf->st_atime = stbuf->st_mtime = time(NULL);
	// stbuf->st_ctime = time(NULL);
     int h= fs_getattr(path, stbuf);
    printf("done with fs_get %d : returned\n\n",h);
    printf("stat VALUES %ld, %ld\n\n", stbuf->st_atime, stbuf->st_mtime);
    return h;
}

static int fioc_readlink(const char *path, char *buf, size_t size) {
    (void) path;
    (void) buf;
    (void) size;
    return -ENOSYS;
}

static int fioc_mknod(const char *path, mode_t mode, dev_t rdev) {
    (void) path;
    (void) mode;
    (void) rdev;
    bool status = fs_mknod(path, mode, rdev);
    if(!status)
    {
        return -1;
    }
    return 0;
}

static int fioc_mkdir(const char *path, mode_t mode) {
    (void) path;
    (void) mode;
    bool status = fs_mkdir(path, mode);
    if(!status)
    {
        return -1;
    }
    return 0;
}

static int fioc_unlink(const char *path) {
    (void) path;
    return fs_unlink(path);
}

static int fioc_rmdir(const char *path) {
    (void) path;
    return fs_unlink(path);
}

static int fioc_symlink(const char *target, const char *linkpath) {
    (void) target;
    (void) linkpath;
    return -ENOSYS;
}

static int fioc_rename(const char *oldpath, const char *newpath, unsigned int flags) {
    (void) oldpath;
    (void) newpath;
    return fs_rename(oldpath, newpath);
}

static int fioc_link(const char *oldpath, const char *newpath) {
    (void) oldpath;
    (void) newpath;
    return -ENOSYS;
}

static int fioc_chmod(const char *path, mode_t mode, struct fuse_file_info *fi) {
    (void) path;
    (void) mode;
    return fs_chmod(path, mode);
}

//TODO chown changes the owner of the file
static int fioc_chown(const char *path, uid_t uid, gid_t gid, struct fuse_file_info *fi) {
    // Stub for chown
    (void) path;
    (void) uid;
    (void) gid;
    return 0;
}

static int fioc_truncate(const char *path, off_t size,
			 struct fuse_file_info *fi)
{
	(void) fi;
	if (debug)
	    printf("CALLING FIOC_TRUNCATE\n\n");
	// if (fioc_file_type(path) != FIOC_FILE)
	// 	return -EINVAL;
    return fs_truncate(path, size);
	//return fioc_resize(size);
}

static int fioc_open(const char *path, struct fuse_file_info *fi)
{
	
	if (debug)
	    printf("PATH PASSED IN %s\n", path);
	ssize_t inum = fs_openFile(path, fi->flags);
    fi->direct_io = 1;
    if(inum <= -1)
    {
        return inum;
    }
    return 0;
}

static int fioc_read(const char *path, char *buf, size_t size,
		     off_t offset, struct fuse_file_info *fi)
{
	(void) fi;
	// if (fioc_file_type(path) != FIOC_FILE)
	// 	return -EINVAL;
    if (debug)
	    printf("CALLING FIOC_READ \n\n");
    printf("reading file : %s\n", path);
	sType nbytes = fs_read(path, buf, size, offset);
    printf("Final read buffer: %.*s\n", (int)nbytes, buf);
    return nbytes;
}



static int fioc_write(const char *path,const char *buf, size_t size, off_t offset, struct fuse_file_info *fi)
{
	if (debug)
	    printf("CALLING FIOC_WRITE \n");
    return fs_write(path, buf, size, offset);
}

static int fioc_statfs(const char *path, struct statvfs *stbuf) {
    if (debug)
	    printf("CALLING FIOC_statfs \n");
    (void) path;
    (void) stbuf;
    return -ENOSYS;
}

static int fioc_flush(const char *path, struct fuse_file_info *fi) {
    if (debug)
	    printf("CALLING FIOC_flush \n");
	    
    (void) path;
    (void) fi;
    return -ENOSYS;
}

static int fioc_release(const char *path, struct fuse_file_info *fi) {
    if (debug)
	    printf("CALLING FIOC_release \n");
    (void) path;
    (void) fi;
    return -ENOSYS;
}

static int fioc_fsync(const char *path, int isdatasync, struct fuse_file_info *fi) {
    (void) path;
    (void) isdatasync;
    (void) fi;
    return -ENOSYS;
}

static int fioc_setxattr(const char *path, const char *name, const char *value, size_t size, int flags) {
    (void) path;
    (void) name;
    (void) value;
    (void) size;
    (void) flags;
    return -ENOSYS;
}

static int fioc_getxattr(const char *path, const char *name, char *value, size_t size) {
    (void) path;
    (void) name;
    (void) value;
    (void) size;
    return -ENOSYS;
}

static int fioc_listxattr(const char *path, char *list, size_t size) {
    (void) path;
    (void) list;
    (void) size;
    return -ENOSYS;
}

static int fioc_removexattr(const char *path, const char *name) {
    (void) path;
    (void) name;
    return -ENOSYS;
}

static int fioc_opendir(const char *path, struct fuse_file_info *fi) {
    if (debug)
	    printf("CALLING FIOC_OPENDIR \n");
    (void) path;
    (void) fi;
    return -ENOSYS;
}

static int fioc_readdir(const char *path, void *buf, fuse_fill_dir_t filler,
			off_t offset, struct fuse_file_info *fi,
			enum fuse_readdir_flags flags)
{
    if (debug)
	    printf("CALLING FIOC_READDIR \n\n");
	(void) fi;
	(void) offset;
	(void) flags;
	// if (fioc_file_type(path) != FIOC_ROOT)
	// 	return -ENOENT;

	// filler(buf, ".", NULL, 0, FUSE_FILL_DIR_DEFAULTS);
	// filler(buf, "..", NULL, 0, FUSE_FILL_DIR_DEFAULTS);
    return fs_readdir(path, buf, filler);
	//return -ENOSYS;
}

static int fioc_releasedir(const char *path, struct fuse_file_info *fi) {
    (void) path;
    (void) fi;
    return -ENOSYS;
}

static int fioc_fsyncdir(const char *path, int isdatasync, struct fuse_file_info *fi) {
    (void) path;
    (void) isdatasync;
    (void) fi;
    return -ENOSYS;
}

static void *fioc_init(struct fuse_conn_info *conn, struct fuse_config *config) {
    (void) conn;
    return NULL;
}

static void fioc_destroy(void *private_data) {
    (void) private_data;
    fs_destroy();
    return;
}

static int fioc_access(const char *path, int mask) {
    (void) path;
    (void) mask;
    return fs_access(path);
}

static int fioc_create(const char *path, mode_t mode, struct fuse_file_info *fi) {
    if (debug)
	    printf("CALLING FIOC_CREATE WITH PATH %s \n", path);
    bool status = false;
    if(fi->flags & O_CREAT)
    {
        status = fs_mknod(path, S_IFREG|mode, -1);
    }
    else
    {
        status = fs_mknod(path, S_IFREG|0775, -1);
    }
    
    if(!status)
    {
        return -1;
    }
    return 0;
}

static int fioc_lock(const char *path, struct fuse_file_info *fi, int cmd, struct flock *lock) {
    (void) path;
    (void) fi;
    (void) cmd;
    (void) lock;
    return -ENOSYS;
}

static int fioc_utimens(const char *path, const struct timespec tv[2], struct fuse_file_info *fi) {
    printf("UTIMENS CALLED FOR PATH %s\n\n", path);
    sType inum = get_inode_of_File(path);
    if (inum == -1) {
        return -ENOENT;
    }

    inodeStruct* inode = loadINodeFromDisk(inum);
    if (inode == NULL) {
        return -EIO; 
    }

    struct timespec current_time;
    if (clock_gettime(CLOCK_REALTIME, &current_time) == -1) {
        free(inode);
        return -EIO;
    }

    if (tv[0].tv_nsec == UTIME_NOW) {
        inode->atime = current_time.tv_sec;
    } else if (tv[0].tv_nsec == UTIME_OMIT) {
    } else {
        inode->atime = tv[0].tv_sec;
    }

    if (tv[1].tv_nsec == UTIME_NOW) {
        inode->mtime = current_time.tv_sec;
    } else if (tv[1].tv_nsec == UTIME_OMIT) {
        // Do not change mtime
    } else {
        inode->mtime = tv[1].tv_sec;
    }

    inode->ctime = current_time.tv_sec;

    printf("Updated inode times:\n");
    printf("  atime: %ld\n", (long)inode->atime);
    printf("  mtime: %ld\n", (long)inode->mtime);
    printf("  ctime: %ld\n", (long)inode->ctime);


    if (!writeINodeToDisk(inum, inode)) {
        free(inode);
        return -EIO;
    }

    free(inode);
    return 0;
}

static int fioc_bmap(const char *path, size_t blocksize, uint64_t *idx) {
    (void) path;
    (void) blocksize;
    (void) idx;
    return -ENOSYS;
}

static int fioc_ioctl(const char *path, unsigned int cmd, void *arg,
		      struct fuse_file_info *fi, unsigned int flags, void *data)
{
	(void) arg;
	(void) fi;
	(void) flags;
	printf("sdfasd\n\n");
	// if (fioc_file_type(path) != FIOC_FILE)
	// 	return -EINVAL;

	// if (flags & FUSE_IOCTL_COMPAT)
	// 	return -ENOSYS;

	// switch (cmd) {
	// case FIOC_GET_SIZE:
	// 	*(size_t *)data = fioc_size;
	// 	return 0;

	// case FIOC_SET_SIZE:
	// 	fioc_resize(*(size_t *)data);
	// 	return 0;
	// }

	return 0;
}

static int fioc_poll(const char *path, struct fuse_file_info *fi, struct fuse_pollhandle *ph, unsigned *reventsp) {
    (void) path;
    (void) fi;
    (void) ph;
    (void) reventsp;
    return -ENOSYS;
}

static int fioc_write_buf(const char *path, struct fuse_bufvec *buf, off_t off, struct fuse_file_info *fi) {
    (void) path;
    (void) buf;
    (void) off;
    (void) fi;
    return -ENOSYS;
}

static int fioc_read_buf(const char *path, struct fuse_bufvec **bufp, size_t size, off_t off, struct fuse_file_info *fi) {
    (void) path;
    (void) bufp;
    (void) size;
    (void) off;
    (void) fi;
    return -ENOSYS;
}

static int fioc_flock(const char *path, struct fuse_file_info *fi, int op) {
    (void) path;
    (void) fi;
    (void) op;
    return -ENOSYS;
}

static int fioc_fallocate(const char *path, int mode, off_t offset, off_t length, struct fuse_file_info *fi) {
    (void) path;
    (void) mode;
    (void) offset;
    (void) length;
    (void) fi;
    return -ENOSYS;
}

static ssize_t fioc_copy_file_range(const char *path_in, struct fuse_file_info *fi_in, off_t off_in,
                                    const char *path_out, struct fuse_file_info *fi_out, off_t off_out, size_t len, int flags) {
    (void) path_in;
    (void) fi_in;
    (void) off_in;
    (void) path_out;
    (void) fi_out;
    (void) off_out;
    (void) len;
    (void) flags;
    return -ENOSYS;
}

static off_t fioc_lseek(const char *path, off_t off, int whence, struct fuse_file_info *fi) {
    (void) path;
    (void) off;
    (void) whence;
    (void) fi;
    return -ENOSYS;
}

static const struct fuse_operations fioc_oper = {
	.getattr	= fioc_getattr,
	//.readlink = fioc_readlink,
	.mknod = fioc_mknod,
	.mkdir = fioc_mkdir,
	.unlink = fioc_unlink,
	.rmdir = fioc_rmdir,
	//.symlink = fioc_symlink,
	.rename = fioc_rename,
	//.link = fioc_link,
	.chmod = fioc_chmod, 
	.chown = fioc_chown,
	.truncate	= fioc_truncate,
	.open		= fioc_open,
	.read		= fioc_read,
	.write		= fioc_write,
	//.statfs = fioc_statfs,
	//.flush = fioc_flush,
	//.release = fioc_release,
	// .fsync = fioc_fsync, 
	// .setxattr = fioc_setxattr,
	//.getxattr = fioc_getxattr,
	// .listxattr = fioc_listxattr,
	// .removexattr = fioc_removexattr,
	//.opendir = fioc_opendir,
	.readdir	= fioc_readdir,
	//.releasedir = fioc_releasedir,
	//.fsyncdir = fioc_fsyncdir,
	//.init = fioc_init,
	.destroy = fioc_destroy,
	.access = fioc_access,
	.create = fioc_create,
	//.lock = fioc_lock,
	.utimens = fioc_utimens,
	//.bmap = fioc_bmap,
	//.ioctl		= fioc_ioctl,
	// .poll = fioc_poll,
	// .write_buf = fioc_write_buf,
	// .read_buf = fioc_read_buf,
	// .flock = fioc_flock,
	// .fallocate = fioc_fallocate,
	// .copy_file_range = fioc_copy_file_range,
	//.lseek = fioc_lseek,
};

int main(int argc, char *argv[])
{
    if(!bootUpFileSytem()){
        printf("FS failed!\n");
        return 0;
    }
	return fuse_main(argc, argv, &fioc_oper, NULL);

}  
