/*
 *  linux/fs/myext2/xip.h
 *
 * Copyright (C) 2005 IBM Corporation
 * Author: Carsten Otte (cotte@de.ibm.com)
 */

#ifdef CONFIG_MYEXT2_FS_XIP
extern void myext2_xip_verify_sb (struct super_block *);
extern int myext2_clear_xip_target (struct inode *, sector_t);

static inline int myext2_use_xip (struct super_block *sb)
{
	struct myext2_sb_info *sbi = MYEXT2_SB(sb);
	return (sbi->s_mount_opt & MYEXT2_MOUNT_XIP);
}
int myext2_get_xip_mem(struct address_space *, pgoff_t, int,
				void **, unsigned long *);
#define mapping_is_xip(map) unlikely(map->a_ops->get_xip_mem)
#else
#define mapping_is_xip(map)			0
#define myext2_xip_verify_sb(sb)			do { } while (0)
#define myext2_use_xip(sb)			0
#define myext2_clear_xip_target(inode, chain)	0
#define myext2_get_xip_mem			NULL
#endif
