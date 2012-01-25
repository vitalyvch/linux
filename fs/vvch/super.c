/*
 *  linux/fs/vvch/super.c
 *
 * Copyright (C) 2011
 * Vitaly Chernooky (vitaly.v.ch@chernooky.com)
 *
 */

#include <linux/module.h>
#include <linux/string.h>
#include <linux/fs.h>
#include <linux/slab.h>
#include <linux/init.h>
#include <linux/blkdev.h>
#include <linux/parser.h>
#include <linux/random.h>
#include <linux/buffer_head.h>
#include <linux/exportfs.h>
#include <linux/smp_lock.h>
#include <linux/vfs.h>
#include <linux/seq_file.h>
#include <linux/mount.h>
#include <linux/log2.h>
#include <linux/quotaops.h>
#include <asm/uaccess.h>


static int vvch_fill_super(struct super_block *s, void *data, int silent)
{
	long ret = -EINVAL;

    printk ("VVCH-fs: Hellow World\n");

	return ret;
}

static int vvch_get_sb(struct file_system_type *fs_type,
	int flags, const char *dev_name, void *data, struct vfsmount *mnt)
{
	return get_sb_bdev(fs_type, flags, dev_name, data, vvch_fill_super, mnt);
}

static struct file_system_type vvch_fs_type = {
	.owner		= THIS_MODULE,
	.name		= "vvch",
	.get_sb		= vvch_get_sb,
	.kill_sb	= kill_block_super,
	.fs_flags	= FS_REQUIRES_DEV,
};

static int __init init_vvch_fs(void)
{
	return register_filesystem(&vvch_fs_type);
}

static void __exit exit_vvch_fs(void)
{
	unregister_filesystem(&vvch_fs_type);
}

module_init(init_vvch_fs)
module_exit(exit_vvch_fs)
