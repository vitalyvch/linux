#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/types.h>

#include <linux/fs.h>

#include "vvchfs_fs.h"

#define VVCHFS_SB_SUPER_MAGIC_STRING	"VvChFsSb"

static const char disk_name[] = "/dev/sda6";

int main() {
	int res;
	int physical_sector_size=0;
	size_t logical_sector_size=0ULL;

	struct vvchfs_super_block sb = {};
	int fd = open (disk_name, O_RDWR);

	if (-1 == fd) {
		fprintf(stderr, "%s:open(%s):%s\n", __func__, disk_name, strerror(errno));
		exit (EXIT_FAILURE);
	}

	strcpy (sb.s_magic, VVCHFS_SB_SUPER_MAGIC_STRING);

	ioctl (fd, BLKBSZGET, &logical_sector_size);
	ioctl (fd, BLKSSZGET, &physical_sector_size);
	
	fprintf(stderr, "%s: BSZ=%d; SSZ=%d\n",
			disk_name, logical_sector_size, physical_sector_size);

	sb.s_blocksize = logical_sector_size;

	res = pwrite(fd, &sb, sizeof(sb), VVCHFS_DISK_OFFSET_IN_BYTES);
	if (-1 == res) {
		fprintf(stderr, "%s:pwrite(%s):%s\n", __func__, disk_name, strerror(errno));
		exit (EXIT_FAILURE);
	}

	close(fd);

	exit(EXIT_SUCCESS);
}
