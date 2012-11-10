/*
 *  linux/fs/vvch/inode.c
 *
 * Copyright (C) 2012
 * Vitaly Chernooky (vitaly.v.ch@chernooky.com)
 *
 */

#include <linux/time.h>
#include <linux/fs.h>
#include <linux/vvchfs_fs.h>
#include <linux/vvchfs_acl.h>
#include <linux/vvchfs_xattr.h>
#include <linux/exportfs.h>
#include <linux/pagemap.h>
#include <linux/highmem.h>
#include <linux/slab.h>
#include <asm/uaccess.h>
#include <asm/unaligned.h>
#include <linux/buffer_head.h>
#include <linux/mpage.h>
#include <linux/writeback.h>
#include <linux/quotaops.h>
#include <linux/swap.h>


/**
 * vvchfs_find_actor() - "find actor" vvchfs supplies to iget5_locked().
 *
 * @inode:    inode from hash table to check
 * @opaque:   "cookie" passed to iget5_locked(). This is &vvchfs_iget_args.
 *
 * This function is called by iget5_locked() to distinguish vvchfs inodes
 * having the same inode numbers. Such inodes can only exist due to some
 * error condition. One of them should be bad. Inodes with identical
 * inode numbers (objectids) are distinguished by parent directory ids.
 *
 */
int vvchfs_find_actor(struct inode *inode, void *opaque)
{
	struct vvchfs_iget_args *args;

	args = opaque;
	/* args is already in CPU order */
	return (inode->i_ino == args->objectid) &&
	    (le32_to_cpu(INODE_PKEY(inode)->k_dir_id) == args->dirid);
}

