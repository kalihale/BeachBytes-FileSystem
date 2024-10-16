/*
  FUSE: Filesystem in Userspace
  Copyright (C) 2001-2007  Miklos Szeredi <miklos@szeredi.hu>

  This program can be distributed under the terms of the GNU GPLv2.
  See the file COPYING.
*/

/** @file
 *
 * minimal example filesystem using high-level API
 *
 * Compile with:
 *
 *     gcc -Wall hello.c `pkg-config fuse3 --cflags --libs` -o hello
 *
 * ## Source code ##
 * \include hello.c
 */


#define FUSE_USE_VERSION 35

#include <fuse.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <stddef.h>
#include <assert.h>
#include "fuse_test.h"
#include <unistd.h>
#include <time.h>
#include <ctype.h>
#include <stdlib.h>
/*
 * Command line options
 *
 * We can't set default values for the char* fields here because
 * fuse_opt_parse would attempt to free() them when the user specifies
 * different values on the command line.
 */
static struct options {
	const char *filename;
	const char *contents;
	int show_help;
} options;

#define OPTION(t, p)                           \
    { t, offsetof(struct options, p), 1 }
static const struct fuse_opt option_spec[] = {
	OPTION("--name=%s", filename),
	OPTION("--contents=%s", contents),
	OPTION("-h", show_help),
	OPTION("--help", show_help),
	FUSE_OPT_END
};

static void *hello_buf;
static size_t hello_size;

static int hello_resize(size_t new_size)
{
	void *new_buf;
	printf("sdfasd\n\n");
	if (new_size == hello_size)
		return 0;

	new_buf = (void*)realloc(hello_buf, new_size);
	if (!new_buf && new_size)
		return -ENOMEM;

	if (new_size > hello_size)
		memset(new_buf + hello_size, 0, new_size - hello_size);

	hello_buf = new_buf;
	hello_size = new_size;

	return 0;
}

static void *hello_init(struct fuse_conn_info *conn,
			struct fuse_config *cfg)
{
	(void) conn;
	cfg->kernel_cache = 1;
	return NULL;
}

static int hello_getattr(const char *path, struct stat *stbuf,
			 struct fuse_file_info *fi)
{
	(void) fi;
	int res = 0;

	memset(stbuf, 0, sizeof(struct stat));
	if (strcmp(path, "/") == 0) {
		stbuf->st_mode = S_IFDIR | 0777;
		stbuf->st_nlink = 2;
	} else{//else if (strcmp(path+1, options.filename) == 0) {
		stbuf->st_mode = S_IFREG | 0777;
		stbuf->st_nlink = 1;
		stbuf->st_size = 100;//strlen(options.contents);
		if(strcmp(options.filename, "")==0)
		//options.filename = path+1;
		options.filename = strdup(path+1);
	}//} else
	//	res = -ENOENT;

	return res;
}

static int hello_readdir(const char *path, void *buf, fuse_fill_dir_t filler,
			 off_t offset, struct fuse_file_info *fi,
			 enum fuse_readdir_flags flags)
{
	(void) offset;
	(void) fi;
	(void) flags;

	if (strcmp(path, "/") != 0)
		return -ENOENT;

	filler(buf, ".", NULL, 0, 0);
	filler(buf, "..", NULL, 0, 0);
	if(strcmp(options.filename, "")==0){
	options.filename = strdup(path+1);
	}
	if(strcmp(options.filename, "")!=0){
		filler(buf, options.filename, NULL, 0, 0);
	}

	return 0;
}

static int hello_open(const char *path, struct fuse_file_info *fi)
{
	if (strcmp(path+1, options.filename) != 0)
		return -ENOENT;

	//if ((fi->flags & O_ACCMODE) != O_RDONLY)
	//	return -EACCES;

	//char fullpath[1024];
    //snprintf(fullpath, sizeof(char)*(strlen(path)+4), "/tes%s", path);

    // int res = open(path+1, fi->flags);
    // if (res == -1)
    //     return -errno;

    // close(res);

	return 0;
}

// static int hello_file_type(const char *path)
// {
// 	printf("sdfasd\n\n");
// 	if (strcmp(path, "/") == 0)
// 		return FIOC_ROOT;
// 	if (strcmp(path, "/" FIOC_NAME) == 0)
// 		return FIOC_FILE;
// 	return FIOC_NONE;
// }

static int fuse_example_utimens(const char *path, const struct timespec ts[2], struct fuse_file_info *fi) {
    printf("Setting times for: %s\n", path);

    char fullpath[1024];
    snprintf(fullpath, sizeof(fullpath), "/tes%s", path);

    // Use utimensat to set the times
    int res = utimensat(0, fullpath, ts, 0);
    if (res == -1)
        return -errno;

    return 0;
}


static int fuse_example_mknod(const char *path, mode_t mode, dev_t rdev) {
    printf("Creating file: %s\n", path);

    // Get the full path to the file
    char fullpath[1024];
    snprintf(fullpath, sizeof(fullpath), "/tes%s", path);

    // Create the file
    int res;
    if (S_ISREG(mode)) {
        res = open(fullpath, O_CREAT | O_EXCL | O_WRONLY, mode);
        if (res >= 0)
            res = close(res);
    } else if (S_ISFIFO(mode)) {
        res = mkfifo(fullpath, mode);
    } else {
        res = mknod(fullpath, mode, rdev);
    }

    if (res == -1)
        return -errno;

    return 0;
}


static int hello_expand(size_t new_size)
{
	printf("sdfasd\n\n");
	if (new_size > hello_size)
		return hello_resize(new_size);
	return 0;
}

static int hello_do_write(const char *buf, size_t size, off_t offset)
{
	printf("sdfasd\n\n");
	if (hello_expand(offset + size))
		return -ENOMEM;
	unsigned char *str = malloc(sizeof(char)*size);
	int i=0;
	while (buf[i] != '\0') {
        str[i] = toupper(buf[i]);
        i++;
    }
	printf("%s \n\n",str);
	memcpy(hello_buf + offset,str, size);

	return size;
}

static int hello_write(const char *path, const char *buf, size_t size,
		      off_t offset, struct fuse_file_info *fi)
{
	(void) fi;
	printf("wert\n\n");
	//if (fioc_file_type(path) != FIOC_FILE)
	//	return -EINVAL;

	return hello_do_write(buf, size, offset);
}

static int hello_read(const char *path, char *buf, size_t size, off_t offset,
		      struct fuse_file_info *fi)
{
	size_t len;
	(void) fi;
	if(strcmp(path+1, options.filename) != 0)
		return -ENOENT;
    printf("sdsdsds\n");
	len = strlen(options.contents);
	if (offset < len) {
		if (offset + size > len)
			size = len - offset;
		memcpy(buf, options.contents + offset, size);
	} else
		size = 0;

	return size;
}
static int fuse_example_create(const char *path, mode_t mode, struct fuse_file_info *fi) {
    printf("Creating file: %s\n", path);
    // Your logic to create the file
    return 0;
}

static int hello_ioctl(const char *path, unsigned int cmd, void *arg,
		      struct fuse_file_info *fi, unsigned int flags, void *data)
{
	(void) arg;
	(void) fi;
	(void) flags;
	printf("sdfasd\n\n");
	// if (hello_file_type(path) != FIOC_FILE)
	// 	return -EINVAL;

	if (flags & FUSE_IOCTL_COMPAT)
		return -ENOSYS;

	switch (cmd) {
	case FIOC_GET_SIZE:
		*(size_t *)data = hello_size;
		return 0;

	case FIOC_SET_SIZE:
		hello_resize(*(size_t *)data);
		return 0;
	}

	return -EINVAL;
}

static int hello_truncate(const char *path, off_t size,
			 struct fuse_file_info *fi)
{
	(void) fi;
	printf("sdfasd\n\n");
	//if (hello_file_type(path) != FIOC_FILE)
	//	return -EINVAL;

	return hello_resize(size);
}

static const struct fuse_operations hello_oper = {
	.init           = hello_init,
	.getattr	= hello_getattr,
	.readdir	= hello_readdir,
	.open		= hello_open,
	.read		= hello_read,
	.ioctl		= hello_ioctl,
	.truncate	= hello_truncate,
 	.ioctl		= hello_ioctl,
	.mknod = fuse_example_mknod,
	.utimens = fuse_example_utimens, 

};

static void show_help(const char *progname)
{
	printf("usage: %s [options] <mountpoint>\n\n", progname);
	printf("File-system specific options:\n"
	       "    --name=<s>          Name of the \"hello\" file\n"
	       "                        (default: \"hello\")\n"
	       "    --contents=<s>      Contents \"hello\" file\n"
	       "                        (default \"Hello, World!\\n\")\n"
	       "\n");
}

int main(int argc, char *argv[])
{
	int ret;
	struct fuse_args args = FUSE_ARGS_INIT(argc, argv);

	/* Set defaults -- we have to use strdup so that
	   fuse_opt_parse can free the defaults if other
	   values are specified */
	options.filename = strdup("");
	options.contents = strdup("Hello World!\n");

	/* Parse options */
	if (fuse_opt_parse(&args, &options, option_spec, NULL) == -1)
		return 1;

	/* When --help is specified, first print our own file-system
	   specific help text, then signal fuse_main to show
	   additional help (by adding `--help` to the options again)
	   without usage: line (by setting argv[0] to the empty
	   string) */
	if (options.show_help) {
		show_help(argv[0]);
		assert(fuse_opt_add_arg(&args, "--help") == 0);
		args.argv[0][0] = '\0';
	}
	printf("didnot go to parse\n");
	ret = fuse_main(args.argc, args.argv, &hello_oper, NULL);
	fuse_opt_free_args(&args);
	return ret;
}


/*
  FUSE fioc: FUSE ioctl example
  Copyright (C) 2008       SUSE Linux Products GmbH
  Copyright (C) 2008       Tejun Heo <teheo@suse.de>

  This program can be distributed under the terms of the GNU GPLv2.
  See the file COPYING.
*/

/** @file
 * @tableofcontents
 *
 * This example illustrates how to write a FUSE file system that can
 * process (a restricted set of) ioctls. It can be tested with the
 * ioctl_client.c program.
 *
 * Compile with:
 *
 *     gcc -Wall ioctl.c `pkg-config fuse3 --cflags --libs` -o ioctl
 *
 * ## Source code ##
 * \include ioctl.c
 */

// #define FUSE_USE_VERSION 35

// #include <fuse.h>
// #include <stdlib.h>
// #include <stdio.h>
// #include <string.h>
// #include <unistd.h>
// #include <time.h>
// #include <errno.h>
// #include <ctype.h>

// #include "fuse_test.h"

// #define FIOC_NAME	"fioc"

// enum {
// 	FIOC_NONE,
// 	FIOC_ROOT,
// 	FIOC_FILE,
// };

// static void *fioc_buf;
// static size_t fioc_size;

// static int fioc_resize(size_t new_size)
// {
// 	void *new_buf;
// 	printf("sdfasd\n\n");
// 	if (new_size == fioc_size)
// 		return 0;

// 	new_buf = realloc(fioc_buf, new_size);
// 	if (!new_buf && new_size)
// 		return -ENOMEM;

// 	if (new_size > fioc_size)
// 		memset(new_buf + fioc_size, 0, new_size - fioc_size);

// 	fioc_buf = new_buf;
// 	fioc_size = new_size;

// 	return 0;
// }

// static int fioc_expand(size_t new_size)
// {
// 	printf("sdfasd\n\n");
// 	if (new_size > fioc_size)
// 		return fioc_resize(new_size);
// 	return 0;
// }

// static int fioc_file_type(const char *path)
// {
// 	printf("sdfasd\n\n");
// 	if (strcmp(path, "/") == 0)
// 		return FIOC_ROOT;
// 	if (strcmp(path, "/" FIOC_NAME) == 0)
// 		return FIOC_FILE;
// 	return FIOC_NONE;
// }

// static int fioc_getattr(const char *path, struct stat *stbuf,
// 			struct fuse_file_info *fi)
// {
// 	(void) fi;
// 	printf("sdfasd\n\n");
// 	stbuf->st_uid = getuid();
// 	stbuf->st_gid = getgid();
// 	stbuf->st_atime = stbuf->st_mtime = time(NULL);

// 	switch (fioc_file_type(path)) {
// 	case FIOC_ROOT:
// 		stbuf->st_mode = S_IFDIR | 0755;
// 		stbuf->st_nlink = 2;
// 		break;
// 	case FIOC_FILE:
// 		stbuf->st_mode = S_IFREG | 0644;
// 		stbuf->st_nlink = 1;
// 		stbuf->st_size = fioc_size;
// 		break;
// 	case FIOC_NONE:
// 		return -ENOENT;
// 	}

// 	return 0;
// }

// static int fioc_open(const char *path, struct fuse_file_info *fi)
// {
// 	(void) fi;
// 	printf("fioc_open\n\n");
// 	if (fioc_file_type(path) != FIOC_NONE)
// 		return 0;
// 	return -ENOENT;
// }

// static int fuse_example_create(const char *path, mode_t mode, struct fuse_file_info *fi) {
//     printf("Creating file: %s\n", path);
//     // Your logic to create the file
//     return 0;
// }
// static int fioc_do_read(char *buf, size_t size, off_t offset)
// {
// 	if (offset >= fioc_size)
// 		return 0;
// 	printf("do_read\n\n");
// 	if (size > fioc_size - offset)
// 		size = fioc_size - offset;

// 	memcpy(buf, fioc_buf + offset, size);

// 	return size;
// }

// static int fioc_read(const char *path, char *buf, size_t size,
// 		     off_t offset, struct fuse_file_info *fi)
// {
// 	(void) fi;
// 	printf("sdfasd\n\n");
// 	if (fioc_file_type(path) != FIOC_FILE)
// 		return -EINVAL;

// 	return fioc_do_read(buf, size, offset);
// }

// static int fioc_do_write(const char *buf, size_t size, off_t offset)
// {
// 	printf("sdfasd\n\n");
// 	if (fioc_expand(offset + size))
// 		return -ENOMEM;
// 	unsigned char *str = malloc(sizeof(char)*size);
// 	int i=0;
// 	while (buf[i] != '\0') {
//         str[i] = toupper(buf[i]);
//         i++;
//     }
// 	memcpy(fioc_buf + offset,str, size);

// 	return size;
// }

// static int fioc_write(const char *path, const char *buf, size_t size,
// 		      off_t offset, struct fuse_file_info *fi)
// {
// 	(void) fi;
// 	printf("wert\n\n");
// 	if (fioc_file_type(path) != FIOC_FILE)
// 		return -EINVAL;

// 	return fioc_do_write(buf, size, offset);
// }

// static int fioc_truncate(const char *path, off_t size,
// 			 struct fuse_file_info *fi)
// {
// 	(void) fi;
// 	printf("sdfasd\n\n");
// 	if (fioc_file_type(path) != FIOC_FILE)
// 		return -EINVAL;

// 	return fioc_resize(size);
// }

// static int fioc_readdir(const char *path, void *buf, fuse_fill_dir_t filler,
// 			off_t offset, struct fuse_file_info *fi,
// 			enum fuse_readdir_flags flags)
// {
// 	(void) fi;
// 	(void) offset;
// 	(void) flags;
// 	printf("readdir\n\n");
// 	if (fioc_file_type(path) != FIOC_ROOT)
// 		return -ENOENT;

// 	filler(buf, ".", NULL, 0, FUSE_FILL_DIR_DEFAULTS);
// 	filler(buf, "..", NULL, 0, FUSE_FILL_DIR_DEFAULTS);
// 	filler(buf, FIOC_NAME, NULL, 0, FUSE_FILL_DIR_DEFAULTS);

// 	return 0;
// }

// static int fioc_ioctl(const char *path, unsigned int cmd, void *arg,
// 		      struct fuse_file_info *fi, unsigned int flags, void *data)
// {
// 	(void) arg;
// 	(void) fi;
// 	(void) flags;
// 	printf("sdfasd\n\n");
// 	if (fioc_file_type(path) != FIOC_FILE)
// 		return -EINVAL;

// 	if (flags & FUSE_IOCTL_COMPAT)
// 		return -ENOSYS;

// 	switch (cmd) {
// 	case FIOC_GET_SIZE:
// 		*(size_t *)data = fioc_size;
// 		return 0;

// 	case FIOC_SET_SIZE:
// 		fioc_resize(*(size_t *)data);
// 		return 0;
// 	}

// 	return -EINVAL;
// }

// static const struct fuse_operations fioc_oper = {
// 	.getattr	= fioc_getattr,
// 	.readdir	= fioc_readdir,
// 	.truncate	= fioc_truncate,
// 	.open		= fioc_open,
// 	.read		= fioc_read,
// 	.write		= fioc_write,
// 	.ioctl		= fioc_ioctl,
// 	.create = fuse_example_create,
// };

// int main(int argc, char *argv[])
// {
// 	printf("indid main func\n\n");
// 	return fuse_main(argc, argv, &fioc_oper, NULL);
// }