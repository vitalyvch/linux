/* Copyright 1996-2000 Vitaly Chernooky, see vvchfs/README for licensing
 * and copyright details */

#ifndef _LINUX_VVCH_FS_SB
#define _LINUX_VVCH_FS_SB

#ifdef __KERNEL__
#include <linux/workqueue.h>
#include <linux/rwsem.h>
#include <linux/mutex.h>
#include <linux/sched.h>
#endif

#define sb_block_count(sbp)         (le32_to_cpu((sbp)->s_block_count))
#define set_sb_block_count(sbp,v)   ((sbp)->s_block_count = cpu_to_le32(v))
#define sb_free_blocks(sbp)         (le32_to_cpu((sbp)->s_free_blocks))
#define set_sb_free_blocks(sbp,v)   ((sbp)->s_free_blocks = cpu_to_le32(v))
#define sb_root_block(sbp)          (le32_to_cpu((sbp)->s_root_block))
#define set_sb_root_block(sbp,v)    ((sbp)->s_root_block = cpu_to_le32(v))

#define sb_blocksize(sbp)          (le16_to_cpu((sbp)->s_blocksize))
#define set_sb_blocksize(sbp,v)    ((sbp)->s_blocksize = cpu_to_le16(v))
#define sb_oid_maxsize(sbp)        (le16_to_cpu((sbp)->s_oid_maxsize))
#define set_sb_oid_maxsize(sbp,v)  ((sbp)->s_oid_maxsize = cpu_to_le16(v))
#define sb_oid_cursize(sbp)        (le16_to_cpu((sbp)->s_oid_cursize))
#define set_sb_oid_cursize(sbp,v)  ((sbp)->s_oid_cursize = cpu_to_le16(v))
#define sb_umount_state(sbp)       (le16_to_cpu((sbp)->s_umount_state))
#define set_sb_umount_state(sbp,v) ((sbp)->s_umount_state = cpu_to_le16(v))
#define sb_fs_state(sbp)           (le16_to_cpu((sbp)->s_fs_state))
#define set_sb_fs_state(sbp,v)     ((sbp)->s_fs_state = cpu_to_le16(v))
/* vvchfs union of in-core super block data */
struct vvchfs_sb_info {
	struct buffer_head *s_sbh;	/* Buffer containing the super block */
	/* both the comment and the choice of
	   name are unclear for s_vs -Hans */
	struct vvchfs_super_block *s_vs;	/* Pointer to the super block in the buffer */
	//struct vvchfs_bitmap_info *s_ap_bitmap;
	//struct vvchfs_journal *s_journal;	/* pointer to journal information */
	unsigned short s_mount_state;	/* vvchfs state (valid, invalid) */

	/* Serialize writers access, replace the old bkl */
	struct mutex lock;
	/* Owner of the lock (can be recursive) */
	struct task_struct *lock_owner;
	/* Depth of the lock, start from -1 like the bkl */
	int lock_depth;

	/* Comment? -Hans */
	void (*end_io_handler) (struct buffer_head *, int);
	/*hashf_t s_hash_function;*/	/* pointer to function which is used
					   to sort names in directory. Set on
					   mount */
	unsigned long s_mount_opt;	/* vvchfs's mount options are set
					   here (currently - NOTAIL, NOLOG,
					   REPLAYONLY) */

	struct {		/* This is a structure that describes block allocator options */
		unsigned long bits;	/* Bitfield for enable/disable kind of options */
		unsigned long large_file_size;	/* size started from which we consider file to be a large one(in blocks) */
		int border;	/* percentage of disk, border takes */
		int preallocmin;	/* Minimal file size (in blocks) starting from which we do preallocations */
		int preallocsize;	/* Number of blocks we try to prealloc when file
					   reaches preallocmin size (in blocks) or
					   prealloc_list is empty. */
	} s_alloc_options;

	/* Comment? -Hans */
	wait_queue_head_t s_wait;
	/* To be obsoleted soon by per buffer seals.. -Hans */
	atomic_t s_generation_counter;	// increased by one every time the
	// tree gets re-balanced
	unsigned long s_properties;	/* File system properties. Currently holds
					   on-disk FS format */

	/* session statistics */
	int s_disk_reads;
	int s_disk_writes;
	int s_fix_nodes;
	int s_do_balance;
	int s_unneeded_left_neighbor;
	int s_good_search_by_key_reada;
	int s_bmaps;
	int s_bmaps_without_search;
	int s_direct2indirect;
	int s_indirect2direct;
	/* set up when it's ok for vvchfs_read_inode2() to read from
	   disk inode with nlink==0. Currently this is only used during
	   finish_unfinished() processing at mount time */
	int s_is_unlinked_ok;
	//vvchfs_proc_info_data_t s_proc_info_data;
	struct proc_dir_entry *procdir;
	int reserved_blocks;	/* amount of blocks reserved for further allocations */
	spinlock_t bitmap_lock;	/* this lock on now only used to protect reserved_blocks variable */
	struct dentry *priv_root;	/* root of /.vvchfs_priv */
	struct dentry *xattr_root;	/* root of /.vvchfs_priv/xattrs */
	int j_errno;
};

#endif				/* _LINUX_VVCH_FS_SB */
