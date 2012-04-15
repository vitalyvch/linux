/*
 *  vvch_fs.h
 *
 *  VVCh File System constants and structures
 *
 *  Copyright 2012 Vitaly Chernooky, see vvchfs/README for licensing and copyright details
 */


#ifndef _LINUX_VVCH_FS_H
#define _LINUX_VVCH_FS_H

#include <linux/types.h>
#include <linux/magic.h>

#ifdef __KERNEL__
#include <linux/slab.h>
#include <linux/interrupt.h>
#include <linux/sched.h>
#include <linux/workqueue.h>
#include <asm/unaligned.h>
#include <linux/bitops.h>
#include <linux/proc_fs.h>
#include <linux/buffer_head.h>
#include "vvchfs_fs_i.h"
#include "vvchfs_fs_sb.h"
#endif

void __vvchfs_warning(struct super_block *s, const char *id,
			 const char *func, const char *fmt, ...);
#define vvchfs_warning(s, id, fmt, args...) \
	 __vvchfs_warning(s, id, __func__, fmt, ##args)
/*
 * Disk Data Structures
 */

/***************************************************************************/
/*                             SUPER BLOCK                                 */
/***************************************************************************/

/*
 * Structure of super block on disk, a version of which in RAM is often accessed as VVCHFS_SB(s)->s_vs
 * the version in RAM is part of a larger structure containing fields never written to disk.
 */


/* this is the on disk super block */
struct vvchfs_super_block {
	__le32 s_block_count;	/* blocks count         */
	__le32 s_free_blocks;	/* free blocks count    */
	__le32 s_root_block;	/* root block number    */
	//struct journal_params s_journal;
	__le16 s_blocksize;	/* block size */
	__le16 s_oid_maxsize;	/* max size of object id array, see
				 * get_objectid() commentary  */
	__le16 s_oid_cursize;	/* current size of object id array */
	__le16 s_umount_state;	/* this is set to 1 when filesystem was
				 * umounted, to 2 - when not */
	char s_magic[10];	/* vvchfs magic string indicates that
				 * file system is vvchfs:
				 * "VvChFs" or "VvCh2Fs" or "VvCh3Fs" */
	__le16 s_fs_state;	/* it is set to used by fsck to mark which
				 * phase of rebuilding is done */
	__le32 s_hash_function_code;	/* indicate, what hash function is being use
					 * to sort names in a directory*/
	__le16 s_tree_height;	/* height of disk tree */
	__le16 s_bmap_nr;	/* amount of bitmap blocks needed to address
				 * each block of file system */
	__le16 s_version;	/* this field is only reliable on filesystem
				 * with non-standard journal */
	__le16 s_reserved_for_journal;	/* size in blocks of journal area on main
					 * device, we need to keep after
					 * making fs with non-standard journal */
	__le32 s_inode_generation;
	__le32 s_flags;		/* Right now used only by inode-attributes, if enabled */
	unsigned char s_uuid[16];	/* filesystem unique identifier */
	unsigned char s_label[16];	/* filesystem volume label */
	__le16 s_mnt_count;		/* Count of mounts since last fsck */
	__le16 s_max_mnt_count;		/* Maximum mounts before check */
	__le32 s_lastcheck;		/* Timestamp of last fsck */
	__le32 s_check_interval;	/* Interval between checks */
	char s_unused[76];	/* zero filled by mkvvchfs and
				 * vvchfs_convert_objectid_map_v1()
				 * so any additions must be updated
				 * there as well. */
} __attribute__ ((__packed__));

#define SB_SIZE (sizeof(struct vvchfs_super_block))


// on-disk super block fields converted to cpu form
#define SB_DISK_SUPER_BLOCK(s) (VVCHFS_SB(s)->s_vs)
#define SB_BLOCKSIZE(s) \
        le32_to_cpu ((SB_DISK_SUPER_BLOCK(s)->s_blocksize))
#define SB_BLOCK_COUNT(s) \
        le32_to_cpu ((SB_DISK_SUPER_BLOCK(s)->s_block_count))
#define SB_FREE_BLOCKS(s) \
        le32_to_cpu ((SB_DISK_SUPER_BLOCK(s)->s_free_blocks))
#define SB_VVCHFS_MAGIC(s) \
        (SB_DISK_SUPER_BLOCK(s)->s_magic)
#define SB_ROOT_BLOCK(s) \
        le32_to_cpu ((SB_DISK_SUPER_BLOCK(s)->s_root_block))
#define SB_TREE_HEIGHT(s) \
        le16_to_cpu ((SB_DISK_SUPER_BLOCK(s)->s_tree_height))
#define SB_VVCHFS_STATE(s) \
        le16_to_cpu ((SB_DISK_SUPER_BLOCK(s)->s_umount_state))
#define SB_VERSION(s) le16_to_cpu ((SB_DISK_SUPER_BLOCK(s)->s_version))
#define SB_BMAP_NR(s) le16_to_cpu ((SB_DISK_SUPER_BLOCK(s)->s_bmap_nr))

/* vvchfs leaves the first 64k unused, so that partition labels have
   enough space.  If someone wants to write a fancy bootloader that
   needs more than 64k, let us know, and this will be increased in size.
   This number must be larger than than the largest block size on any
   platform, or code will break.  -Hans */
#define VVCHFS_DISK_OFFSET_IN_BYTES (64 * 1024)
#define VVCHFS_FIRST_BLOCK unused_define
#define VVCHFS_JOURNAL_OFFSET_IN_BYTES VVCHFS_DISK_OFFSET_IN_BYTES

/* the spot for the super in versions 3.5 - 3.5.10 (inclusive) */
#define VVCHFS_OLD_DISK_OFFSET_IN_BYTES (8 * 1024)

static inline struct vvchfs_sb_info *VVCHFS_SB(const struct super_block *sb)
{
	return sb->s_fs_info;
}


/*
 * values for s_umount_state field
 */
#define VVCHFS_VALID_FS    1
#define VVCHFS_ERROR_FS    2

// two entries per block (at least)
#define VVCHFS_MAX_NAME(block_size) 255

void vvchfs_info(struct super_block *s, const char *fmt, ...);

#endif				/* _LINUX_VVCH_FS_H */
