/*
 * Copyright 2012 by Vitaly Chernooky, licensing governed by vvchfs/README
 */

#include <linux/time.h>
#include <linux/fs.h>
#include <linux/string.h>
#include <linux/buffer_head.h>

#include <stdarg.h>

#include "vvchfs_fs.h"

static char error_buf[1024];
static char fmt_buf[1024];








static char *is_there_vvchfs_struct(char *fmt, int *what)
{
	char *k = fmt;

	while ((k = strchr(k, '%')) != NULL) {
		if (k[1] == 'k' || k[1] == 'K' || k[1] == 'h' || k[1] == 't' ||
		    k[1] == 'z' || k[1] == 'b' || k[1] == 'y' || k[1] == 'a') {
			*what = k[1];
			break;
		}
		k++;
	}
	return k;
}

/* debugging vvchfs we used to print out a lot of different
   variables, like keys, item headers, buffer heads etc. Values of
   most fields matter. So it took a long time just to write
   appropriative printk. With this vvchfs_warning you can use format
   specification for complex structures like you used to do with
   printfs for integers, doubles and pointers. For instance, to print
   out key structure you have to write just:
   vvchfs_warning ("bad key %k", key);
   instead of
   printk ("bad key %lu %lu %lu %lu", key->k_dir_id, key->k_objectid,
           key->k_offset, key->k_uniqueness);
*/
static DEFINE_SPINLOCK(error_lock);
static void prepare_error_buf(const char *fmt, va_list args)
{
	char *fmt1 = fmt_buf;
	char *k;
	char *p = error_buf;
	int what;

	spin_lock(&error_lock);

	strcpy(fmt1, fmt);

	while ((k = is_there_vvchfs_struct(fmt1, &what)) != NULL) {
		*k = 0;

		p += vsprintf(p, fmt1, args);

		switch (what) {
		case 'k':
			//sprintf_le_key(p, va_arg(args, struct vvchfs_key *));
			break;
		case 'K':
			//sprintf_cpu_key(p, va_arg(args, struct cpu_key *));
			break;
		case 'h':
			//sprintf_item_head(p, va_arg(args, struct item_head *));
			break;
		case 't':
            /*
			sprintf_direntry(p,
					 va_arg(args,
						struct vvchfs_dir_entry *));
                        */
			break;
		case 'y':
            /*
			sprintf_disk_child(p,
					   va_arg(args, struct disk_child *));
                       */
			break;
		case 'z':
            /*
			sprintf_block_head(p,
					   va_arg(args, struct buffer_head *));
                       */
			break;
		case 'b':
            /*
			sprintf_buffer_head(p,
					    va_arg(args, struct buffer_head *));
                        */
			break;
		case 'a':
			/*
            sprintf_de_head(p,
					va_arg(args,
					       struct vvchfs_de_head *));
                           */
			break;
		}

		p += strlen(p);
		fmt1 = k + 2;
	}
	vsprintf(p, fmt1, args);
	spin_unlock(&error_lock);

}

/* in addition to usual conversion specifiers this accepts vvchfs
   specific conversion specifiers:
   %k to print little endian key,
   %K to print cpu key,
   %h to print item_head,
   %t to print directory entry
   %z to print block head (arg must be struct buffer_head *
   %b to print buffer_head
*/

#define do_vvchfs_warning(fmt)\
{\
    va_list args;\
    va_start( args, fmt );\
    prepare_error_buf( fmt, args );\
    va_end( args );\
}

void __vvchfs_warning(struct super_block *sb, const char *id,
			 const char *function, const char *fmt, ...)
{
	do_vvchfs_warning(fmt);
	if (sb)
		printk(KERN_WARNING "VVCHFS warning (device %s): %s%s%s: "
		       "%s\n", sb->s_id, id ? id : "", id ? " " : "",
		       function, error_buf);
	else
		printk(KERN_WARNING "VVCHFS warning: %s%s%s: %s\n",
		       id ? id : "", id ? " " : "", function, error_buf);
}

/* No newline.. vvchfs_info calls can be followed by printk's */
void vvchfs_info(struct super_block *sb, const char *fmt, ...)
{
	do_vvchfs_warning(fmt);
	if (sb)
		printk(KERN_NOTICE "VVCHFS (device %s): %s",
		       sb->s_id, error_buf);
	else
		printk(KERN_NOTICE "VVCHFS %s:", error_buf);
}
