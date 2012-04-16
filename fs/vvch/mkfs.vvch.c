#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <sys/stat.h>
#include <sys/types.h>

#include "vvchfs_fs.h"

static const char disk_name[] = "/dev/sda6";

int main() {
	int res;
	struct vvchfs_super_block sb;
	int fd = open (disk_name, O_RDWR);

	if (-1 == fd) {
		fprintf(stderr, "%s:open(%s):%s\n", __func__, disk_name, strerror(errno));
		exit (EXIT_FAILURE);
	}

	res = pwrite(fd, &sb, sizeof(sb), VVCHFS_DISK_OFFSET_IN_BYTES);
	if (-1 == res) {
		fprintf(stderr, "%s:pwrite(%s):%s\n", __func__, disk_name, strerror(errno));
		exit (EXIT_FAILURE);
	}

	close(fd);

	exit(EXIT_SUCCESS);
}
