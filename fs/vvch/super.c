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
#include <linux/quotaops.h>
#include <linux/smp_lock.h>
#include <linux/vfs.h>
#include <linux/seq_file.h>
#include <linux/mount.h>
#include <linux/log2.h>
#include <linux/crc32.h>
#include <asm/uaccess.h>

#include "vvchfs_fs.h"
#include "vvchfs_fs_sb.h"

static struct file_system_type vvch_fs_type;

static const char vvchfs_sb_magic_string[] = VVCHFS_SB_SUPER_MAGIC_STRING;
static const char vvchfs_bp_magic_string[] = VVCHFS_BP_SUPER_MAGIC_STRING;

int is_vvchfs_sb(struct vvchfs_super_block *vs)
{
	return !strncmp(vs->s_magic, vvchfs_sb_magic_string,
			strlen(vvchfs_sb_magic_string));
}

int is_vvchfs_bp(struct vvchfs_super_block *vs)
{
	return !strncmp(vs->s_magic, vvchfs_bp_magic_string,
			strlen(vvchfs_bp_magic_string));
}

static int is_any_vvchfs_magic_string(struct vvchfs_super_block *vs)
{
	return (is_vvchfs_sb(vs));
}

static int vvchfs_statfs(struct dentry *dentry, struct kstatfs *buf);

static const struct super_operations vvchfs_sops = {
#if 0
	.alloc_inode = vvchfs_alloc_inode,
	.destroy_inode = vvchfs_destroy_inode,
	.write_inode = vvchfs_write_inode,
	.dirty_inode = vvchfs_dirty_inode,
	.evict_inode = vvchfs_evict_inode,
	.put_super = vvchfs_put_super,
	.write_super = vvchfs_write_super,
	.sync_fs = vvchfs_sync_fs,
	.freeze_fs = vvchfs_freeze,
	.unfreeze_fs = vvchfs_unfreeze,
#endif
	.statfs = vvchfs_statfs,
#if 0
	.remount_fs = vvchfs_remount,
	.show_options = generic_show_options,
#endif
};

static int vvchfs_parse_options(struct super_block *s, char *options,	/* string given via mount's -o */
				  unsigned long *mount_options,
				  /* after the parsing phase, contains the
				     collection of bitflags defining what
				     mount options were selected. */
				  unsigned long *blocks,	/* strtol-ed from NNN of resize=NNN */
				  char **jdev_name,
				  unsigned int *commit_max_age,
				  char **qf_names,
				  unsigned int *qfmt)
{
    /*
	int c;
	char *arg = NULL;
    */
	char *pos;
#if 0
	opt_desc_t opts[] = {
		/* Compatibility stuff, so that -o notail for old setups still work */
		{"tails",.arg_required = 't',.values = tails},
		{"notail",.clrmask =
		 (1 << VVCHFS_LARGETAIL) | (1 << VVCHFS_SMALLTAIL)},
		{"conv",.setmask = 1 << VVCHFS_CONVERT},
		{"attrs",.setmask = 1 << VVCHFS_ATTRS},
		{"noattrs",.clrmask = 1 << VVCHFS_ATTRS},
		{"expose_privroot", .setmask = 1 << VVCHFS_EXPOSE_PRIVROOT},
#ifdef CONFIG_VVCHFS_FS_XATTR
		{"user_xattr",.setmask = 1 << VVCHFS_XATTRS_USER},
		{"nouser_xattr",.clrmask = 1 << VVCHFS_XATTRS_USER},
#else
		{"user_xattr",.setmask = 1 << VVCHFS_UNSUPPORTED_OPT},
		{"nouser_xattr",.clrmask = 1 << VVCHFS_UNSUPPORTED_OPT},
#endif
#ifdef CONFIG_VVCHFS_FS_POSIX_ACL
		{"acl",.setmask = 1 << VVCHFS_POSIXACL},
		{"noacl",.clrmask = 1 << VVCHFS_POSIXACL},
#else
		{"acl",.setmask = 1 << VVCHFS_UNSUPPORTED_OPT},
		{"noacl",.clrmask = 1 << VVCHFS_UNSUPPORTED_OPT},
#endif
		{.option_name = "nolog"},
		{"replayonly",.setmask = 1 << REPLAYONLY},
		{"block-allocator",.arg_required = 'a',.values = balloc},
		{"data",.arg_required = 'd',.values = logging_mode},
		{"barrier",.arg_required = 'b',.values = barrier_mode},
		{"resize",.arg_required = 'r',.values = NULL},
		{"jdev",.arg_required = 'j',.values = NULL},
		{"nolargeio",.arg_required = 'w',.values = NULL},
		{"commit",.arg_required = 'c',.values = NULL},
		{"usrquota",.setmask = 1 << VVCHFS_QUOTA},
		{"grpquota",.setmask = 1 << VVCHFS_QUOTA},
		{"noquota",.clrmask = 1 << VVCHFS_QUOTA},
		{"errors",.arg_required = 'e',.values = error_actions},
		{"usrjquota",.arg_required =
		 'u' | (1 << VVCHFS_OPT_ALLOWEMPTY),.values = NULL},
		{"grpjquota",.arg_required =
		 'g' | (1 << VVCHFS_OPT_ALLOWEMPTY),.values = NULL},
		{"jqfmt",.arg_required = 'f',.values = NULL},
		{.option_name = NULL}
	};
#endif

	*blocks = 0;
	if (!options || !*options)
		/* use default configuration: create tails, journaling on, no
		   conversion to newest format */
		return 1;

	for (pos = options; pos;) {
#if 0
		c = vvchfs_getopt(s, &pos, opts, &arg, mount_options);
		if (c == -1)
			/* wrong option is given */
			return 0;

		if (c == 'r') {
			char *p;

			p = NULL;
			/* "resize=NNN" or "resize=auto" */

			if (!strcmp(arg, "auto")) {
				/* From JFS code, to auto-get the size. */
				*blocks =
				    s->s_bdev->bd_inode->i_size >> s->
				    s_blocksize_bits;
			} else {
				*blocks = simple_strtoul(arg, &p, 0);
				if (*p != '\0') {
					/* NNN does not look like a number */
					vvchfs_warning(s, "super-6507",
							 "bad value %s for "
							 "-oresize\n", arg);
					return 0;
				}
			}
		}

		if (c == 'c') {
			char *p = NULL;
			unsigned long val = simple_strtoul(arg, &p, 0);
			/* commit=NNN (time in seconds) */
			if (*p != '\0' || val >= (unsigned int)-1) {
				vvchfs_warning(s, "super-6508",
						 "bad value %s for -ocommit\n",
						 arg);
				return 0;
			}
			*commit_max_age = (unsigned int)val;
		}

		if (c == 'w') {
			vvchfs_warning(s, "super-6509", "nolargeio option "
					 "is no longer supported");
			return 0;
		}

		if (c == 'j') {
			if (arg && *arg && jdev_name) {
				if (*jdev_name) {	//Hm, already assigned?
					vvchfs_warning(s, "super-6510",
							 "journal device was "
							 "already specified to "
							 "be %s", *jdev_name);
					return 0;
				}
				*jdev_name = arg;
			}
		}
#endif
	}

	return 1;
}


static int read_super_block(struct super_block *s, int offset)
{
	struct buffer_head *bh;
	struct vvchfs_super_block *vs;
	int fs_blocksize;

	bh = sb_bread(s, offset / s->s_blocksize);
	if (!bh) {
		vvchfs_warning(s, "sh-2006",
				 "bread failed (dev %s, block %lu, size %lu)",
				 vvchfs_bdevname(s), offset / s->s_blocksize,
				 s->s_blocksize);
		return 1;
	}

	vs = (struct vvchfs_super_block *)bh->b_data;
	if (!is_any_vvchfs_magic_string(vs)) {
		brelse(bh);
		return 1;
	}
	//
	// ok, vvchfs signature (old or new) found in at the given offset
	//
	fs_blocksize = sb_blocksize(vs);
	brelse(bh);
	sb_set_blocksize(s, fs_blocksize);

	bh = sb_bread(s, offset / s->s_blocksize);
	if (!bh) {
		vvchfs_warning(s, "sh-2007",
				 "bread failed (dev %s, block %lu, size %lu)",
				 vvchfs_bdevname(s), offset / s->s_blocksize,
				 s->s_blocksize);
		return 1;
	}

	vs = (struct vvchfs_super_block *)bh->b_data;
	if (sb_blocksize(vs) != s->s_blocksize) {
		vvchfs_warning(s, "sh-2011", "can't find a vvchfs "
				 "filesystem on (dev %s, block %Lu, size %lu)",
				 vvchfs_bdevname(s),
				 (unsigned long long)bh->b_blocknr,
				 s->s_blocksize);
		brelse(bh);
		return 1;
	}

	if (vs->s_root_block == cpu_to_le32(-1)) {
		brelse(bh);
		vvchfs_warning(s, "super-6519", "Unfinished vvchfsck "
				 "--rebuild-tree run detected. Please run\n"
				 "vvchfsck --rebuild-tree and wait for a "
				 "completion. If that fails\n"
				 "get newer vvchfsprogs package");
		return 1;
	}

	SB_BUFFER_WITH_SB(s) = bh;
	SB_DISK_SUPER_BLOCK(s) = vs;

		/* s_version of standard format may contain incorrect information,
		   so we just look at the magic string */
		vvchfs_info(s,
			      "found vvchfs format \"%s\" with standard journal\n",
			      "0.0");

	s->s_op = &vvchfs_sops;
#if 0
	s->s_export_op = &vvchfs_export_ops;
#endif

	/* new format is limited by the 32 bit wide i_blocks field, want to
	 ** be one full block below that.
	 */
	s->s_maxbytes = (512LL << 32) - s->s_blocksize;
	return 0;
}

#define SWARN(silent, s, id, ...)			\
	if (!(silent))				\
		vvchfs_warning(s, id, __VA_ARGS__)

static int vvch_fill_super(struct super_block *s, void *data, int silent)
{
	unsigned long blocks;
	unsigned int commit_max_age = 0;
	struct vvchfs_super_block *vs;
	char *jdev_name;
	struct vvchfs_sb_info *sbi;
	int errval = -EINVAL;
	char *qf_names[MAXQUOTAS] = {};
	unsigned int qfmt = 0;

    printk ("VVCH-fs: Hellow World\n");

	save_mount_options(s, data);

	sbi = kzalloc(sizeof(struct vvchfs_sb_info), GFP_KERNEL);
	if (!sbi) {
		errval = -ENOMEM;
		goto error;
	}
	/// @value
	///   -1 - Unknown sb offset
	///   -2 - There are no superblock on disk.
	///        Superblock recived via mount options
	sbi->vvchfs_disk_offset_in_bytes = -1;
	s->s_fs_info = sbi;


	jdev_name = NULL;
	if (vvchfs_parse_options
	    (s, (char *)data, &(sbi->s_mount_opt), &blocks, &jdev_name,
	     &commit_max_age, qf_names, &qfmt) == 0) {
		goto error;
	}

	if (blocks) {
		SWARN(silent, s, "jmacd-7", "resize option for remount only");
		goto error;
	}

	/// if superblocl already setted via mount options
	if (-2 == sbi->vvchfs_disk_offset_in_bytes)
		goto skip_reading_superblock;

	/// @todo Loop over powers of 2
	if (!read_super_block(s, 0))
		sbi->vvchfs_disk_offset_in_bytes = 0;
	else {
		for (sbi->vvchfs_disk_offset_in_bytes = s->s_blocksize;
			 sbi->vvchfs_disk_offset_in_bytes <= MAX_VVCHFS_DISK_OFFSET_IN_BYTES;
			 sbi->vvchfs_disk_offset_in_bytes *= 2)
		{
			if (!read_super_block(s, sbi->vvchfs_disk_offset_in_bytes))
				break;
		}
	}
	   
	   
	if (sbi->vvchfs_disk_offset_in_bytes > MAX_VVCHFS_DISK_OFFSET_IN_BYTES) {
		SWARN(silent, s, "sh-2021", "can not find vvchfs on %s",
			vvchfs_bdevname(s));
		goto error;
	}

skip_reading_superblock:
	vs = SB_DISK_SUPER_BLOCK(s);
	/* Let's do basic sanity check to verify that underlying device is not
	   smaller than the filesystem. If the check fails then abort and scream,
	   because bad stuff will happen otherwise. */
	if (s->s_bdev && s->s_bdev->bd_inode
	    && i_size_read(s->s_bdev->bd_inode) <
	    sb_block_count(vs) * sb_blocksize(vs)) {
		SWARN(silent, s, "", "Filesystem cannot be "
		      "mounted because it is bigger than the device");
		SWARN(silent, s, "", "You may need to run fsck "
		      "or increase size of your LVM partition");
		SWARN(silent, s, "", "Or may be you forgot to "
		      "reboot after fdisk when it told you to");
		goto error;
	}

	sbi->s_mount_state = SB_VVCHFS_STATE(s);
	sbi->s_mount_state = VVCHFS_VALID_FS;
    sbi->s_vs = vs;



error:
	kfree(sbi);

	s->s_fs_info = NULL;
	return errval;
}

static int vvchfs_statfs(struct dentry *dentry, struct kstatfs *buf)
{
	struct vvchfs_super_block *vs = SB_DISK_SUPER_BLOCK(dentry->d_sb);

	buf->f_namelen = (VVCHFS_MAX_NAME(s->s_blocksize));
	buf->f_bfree = sb_free_blocks(vs);
	buf->f_bavail = buf->f_bfree;
	buf->f_blocks = sb_block_count(vs);
	buf->f_bsize = dentry->d_sb->s_blocksize;
	/* changed to accommodate gcc folks. */
	buf->f_type = VVCHFS_SUPER_MAGIC;
	buf->f_fsid.val[0] = (u32)crc32_le(0, vs->s_uuid, sizeof(vs->s_uuid)/2);
	buf->f_fsid.val[1] = (u32)crc32_le(0, vs->s_uuid + sizeof(vs->s_uuid)/2,
				sizeof(vs->s_uuid)/2);

	return 0;
}

static int vvch_get_sb(struct file_system_type *fs_type,
	int flags, const char *dev_name, void *data, struct vfsmount *mnt)
{
	return get_sb_bdev(fs_type, flags, dev_name, data, vvch_fill_super, mnt);
}


static int __init init_vvch_fs(void)
{
	return register_filesystem(&vvch_fs_type);
}

static void __exit exit_vvch_fs(void)
{
	unregister_filesystem(&vvch_fs_type);
}

static struct file_system_type vvch_fs_type = {
	.owner		= THIS_MODULE,
	.name		= "vvch",
	.get_sb		= vvch_get_sb,
	.kill_sb	= kill_block_super,
	.fs_flags	= FS_REQUIRES_DEV,
};

MODULE_DESCRIPTION("vvchfs journaled filesystem");
MODULE_AUTHOR("Vitaly Chernooky <vv@chernooky.com>");
MODULE_LICENSE("GPL");

module_init(init_vvch_fs);
module_exit(exit_vvch_fs);
