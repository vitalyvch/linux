/*
 *  linux/fs/vvch/inode.c
 *
 * Copyright (C) 2012
 * Vitaly Chernooky (vitaly.v.ch@chernooky.com)
 *
 */

#include <linux/time.h>
#include <linux/fs.h>
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

#include "vvchfs_fs.h"

//
// initially this function was derived from minix or ext2's analog and
// evolved as the prototype did
//

int vvchfs_init_locked_inode(struct inode *inode, void *p)
{
	struct vvchfs_iget_args *args = (struct vvchfs_iget_args *)p;
	inode->i_ino = args->objectid;
	INODE_PKEY(inode)->k_dir_id = cpu_to_le32(args->dirid);
	return 0;
}

/* looks for stat data in the tree, and fills up the fields of in-core
   inode stat data fields */
void vvchfs_read_locked_inode(struct inode *inode,
				struct vvchfs_iget_args *args)
{
#if 0
	INITIALIZE_PATH(path_to_sd);
	struct cpu_key key;
	unsigned long dirino;
	int retval;

	dirino = args->dirid;

	/* set version 1, version 2 could be used too, because stat data
	   key is the same in both versions */
	key.version = KEY_FORMAT_3_5;
	key.on_disk_key.k_dir_id = dirino;
	key.on_disk_key.k_objectid = inode->i_ino;
	key.on_disk_key.k_offset = 0;
	key.on_disk_key.k_type = 0;

	/* look for the object's stat data */
	retval = search_item(inode->i_sb, &key, &path_to_sd);
	if (retval == IO_ERROR) {
		vvchfs_error(inode->i_sb, "vs-13070",
			       "i/o failure occurred trying to find "
			       "stat data of %K", &key);
		vvchfs_make_bad_inode(inode);
		return;
	}
	if (retval != ITEM_FOUND) {
		/* a stale NFS handle can trigger this without it being an error */
		pathrelse(&path_to_sd);
		vvchfs_make_bad_inode(inode);
		inode->i_nlink = 0;
		return;
	}

	init_inode(inode, &path_to_sd);

	/* It is possible that knfsd is trying to access inode of a file
	   that is being removed from the disk by some other thread. As we
	   update sd on unlink all that is required is to check for nlink
	   here. This bug was first found by Sizif when debugging
	   SquidNG/Butterfly, forgotten, and found again after Philippe
	   Gramoulle <philippe.gramoulle@mmania.com> reproduced it.

	   More logical fix would require changes in fs/inode.c:iput() to
	   remove inode from hash-table _after_ fs cleaned disk stuff up and
	   in iget() to return NULL if I_FREEING inode is found in
	   hash-table. */
	/* Currently there is one place where it's ok to meet inode with
	   nlink==0: processing of open-unlinked and half-truncated files
	   during mount (fs/vvchfs/super.c:finish_unfinished()). */
	if ((inode->i_nlink == 0) &&
	    !REISERFS_SB(inode->i_sb)->s_is_unlinked_ok) {
		vvchfs_warning(inode->i_sb, "vs-13075",
				 "dead inode read from disk %K. "
				 "This is likely to be race with knfsd. Ignore",
				 &key);
		vvchfs_make_bad_inode(inode);
	}

	vvchfs_check_path(&path_to_sd);	/* init inode should be relsing */
#endif
}

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

