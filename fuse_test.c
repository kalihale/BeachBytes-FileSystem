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

#include "fuse_test.h"

#define FIOC_NAME	"fioc"

enum {
	FIOC_NONE,
	FIOC_ROOT,
	FIOC_FILE,
};

static struct data {
	const char *name;	
}data;

#define OPTION(t, p)                           \
    { t, offsetof(struct data, p), 1 }
static const struct fuse_opt data_spec[] = {
	OPTION("--name=%s", name),
	FUSE_OPT_END
};



static void *fioc_buf;
static size_t fioc_size;

static int fioc_resize(size_t new_size)
{
	void *new_buf;
	if (new_size == fioc_size)
		return 0;

	new_buf = realloc(fioc_buf, new_size);
	if (!new_buf && new_size)
		return -ENOMEM;

	if (new_size > fioc_size)
		memset(new_buf + fioc_size, 0, new_size - fioc_size);

	fioc_buf = new_buf;
	fioc_size = new_size;

	return 0;
}

static int fioc_expand(size_t new_size)
{
	printf("sdfasd\n\n");
	if (new_size > fioc_size)
		return fioc_resize(new_size);
	return 0;
}

static int fioc_file_type(const char *path)
{
	
	if (strcmp(path, "/") == 0)
		return FIOC_ROOT;
	
	char* newPath = malloc(strlen(data.name) + 2);
	strcpy(newPath, "/");
	strcat(newPath, data.name);
	if (strcmp(path, newPath) == 0){
		free(newPath);
		return FIOC_FILE;
	}
	free(newPath);
	return FIOC_NONE;
}

static int fioc_getattr(const char *path, struct stat *stbuf,
			struct fuse_file_info *fi)
{
	(void) fi;
	printf("sdfasd\n\n");
	stbuf->st_uid = getuid();
	stbuf->st_gid = getgid();
	stbuf->st_atime = stbuf->st_mtime = time(NULL);

	switch (fioc_file_type(path)) {
	case FIOC_ROOT:
		stbuf->st_mode = S_IFDIR | 0777;
		stbuf->st_nlink = 2;
		break;
	case FIOC_FILE:
	case FIOC_NONE:
		stbuf->st_mode = S_IFREG | 0777;
		stbuf->st_nlink = 1;
		stbuf->st_size = fioc_size;
		break;
	// case FIOC_NONE:
	// 	return -ENOENT;
	}

	return 0;
}

static int fioc_open(const char *path, struct fuse_file_info *fi)
{
	(void) fi;
	printf("fioc_open\n\n");
	// if (fioc_file_type(path) != FIOC_NONE)
	// 	return 0;
	return 0;
}

static int fuse_example_create(const char *path, mode_t mode, struct fuse_file_info *fi) {
    printf("Creating file: %s\n", path);
    // Your logic to create the file
    return 0;
}
static int fioc_do_read(char *buf, size_t size, off_t offset)
{
	if (offset >= fioc_size)
		return 0;
	printf("do_read\n\n");
	if (size > fioc_size - offset)
		size = fioc_size - offset;

	memcpy(buf, fioc_buf + offset, size);

	return size;
}

static int fioc_read(const char *path, char *buf, size_t size,
		     off_t offset, struct fuse_file_info *fi)
{
	(void) fi;
	printf("sdfasd\n\n");
	// if (fioc_file_type(path) != FIOC_FILE)
	// 	return -EINVAL;

	return fioc_do_read(buf, size, offset);
}

static int fioc_do_write(const char *buf, size_t size, off_t offset)
{
	printf("sdfasd\n\n");
	 if (fioc_expand(offset + size))
	 	return -ENOMEM;
	unsigned char *str = malloc(sizeof(char)*size);
	int i=0;
	while (buf[i] != '\0') {
        str[i] = toupper(buf[i]);
        i++;
    }
	memcpy(fioc_buf + offset,str, size);

	return size;
}

static int fioc_write(const char *path, const char *buf, size_t size,
		      off_t offset, struct fuse_file_info *fi)
{
	(void) fi;
	printf("wert\n\n");
	FILE *fp;
	int status;
	fp = popen("mail -s 'CS270 testing' rkerur@ucsb.edu", "w");
	
	size_t r1 = fwrite(buf, sizeof(buf[0]), size, fp);

	status = pclose(fp);
	if (status == -1) {
    	printf(" didnot send\n\n");
	} else {
		printf(" sent correctly\n\n");
	}
	// if (fioc_file_type(path) != FIOC_FILE)
	// 	return -EINVAL;

	//return fioc_do_write(buf, size, offset);
	return size;
}

static int fioc_truncate(const char *path, off_t size,
			 struct fuse_file_info *fi)
{
	(void) fi;
	printf("sdfasd\n\n");
	// if (fioc_file_type(path) != FIOC_FILE)
	// 	return -EINVAL;

	return fioc_resize(size);
}

static int fioc_readdir(const char *path, void *buf, fuse_fill_dir_t filler,
			off_t offset, struct fuse_file_info *fi,
			enum fuse_readdir_flags flags)
{
	(void) fi;
	(void) offset;
	(void) flags;
	printf("readdir\n\n");
	// if (fioc_file_type(path) != FIOC_ROOT)
	// 	return -ENOENT;

	filler(buf, ".", NULL, 0, FUSE_FILL_DIR_DEFAULTS);
	filler(buf, "..", NULL, 0, FUSE_FILL_DIR_DEFAULTS);
	filler(buf, data.name, NULL, 0, FUSE_FILL_DIR_DEFAULTS);

	return 0;
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

	if (flags & FUSE_IOCTL_COMPAT)
		return -ENOSYS;

	switch (cmd) {
	case FIOC_GET_SIZE:
		*(size_t *)data = fioc_size;
		return 0;

	case FIOC_SET_SIZE:
		fioc_resize(*(size_t *)data);
		return 0;
	}

	return -EINVAL;
}

static const struct fuse_operations fioc_oper = {
	.getattr	= fioc_getattr,
	.readdir	= fioc_readdir,
	.truncate	= fioc_truncate,
	.open		= fioc_open,
	.read		= fioc_read,
	.write		= fioc_write,
	.ioctl		= fioc_ioctl,
	.create = fuse_example_create,
};

int main(int argc, char *argv[])
{
	struct fuse_args args = FUSE_ARGS_INIT(argc, argv);

	data.name = strdup("DefaultFile");
	if (fuse_opt_parse(&args, &data, data_spec, NULL) == -1)
		return 1;

	printf(data.name);

	int ret = fuse_main(args.argc, args.argv, &fioc_oper, NULL);
	fuse_opt_free_args(&args);
	return ret;
}