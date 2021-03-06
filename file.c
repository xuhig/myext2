/*
 *  linux/fs/myext2/file.c
 *
 * Copyright (C) 1992, 1993, 1994, 1995
 * Remy Card (card@masi.ibp.fr)
 * Laboratoire MASI - Institut Blaise Pascal
 * Universite Pierre et Marie Curie (Paris VI)
 *
 *  from
 *
 *  linux/fs/minix/file.c
 *
 *  Copyright (C) 1991, 1992  Linus Torvalds
 *
 *  myext2 fs regular file handling primitives
 *
 *  64-bit file support on 64-bit platforms by Jakub Jelinek
 * 	(jj@sunsite.ms.mff.cuni.cz)
 */

#include <linux/time.h>
#include <linux/pagemap.h>
#include <linux/quotaops.h>
#include "myext2.h"
#include "xattr.h"
#include "acl.h"

//add

ssize_t new_sync_write_crypt(struct file *filp, const char __user *buf, size_t len, loff_t *ppos)
{
		char* mybuf = buf;
        //在此处添加对长度为len的buf数据进行加密（简单移位密码，将每个字符值+25）
	int i=0;
for(i=0;i<len;i++)
{
	*(mybuf+i)=*(mybuf+i)+25;
//printk("haha encrypt %ld\n", *buf);
}


        printk("haha encrypt %ld\n", len);
		return do_sync_write(filp, mybuf, len, ppos);
}

ssize_t new_sync_read_crypt(struct file *filp, char __user *buf, size_t len, loff_t *ppos)
{
        //int i;
	//	//先调用默认的读函数读取文件数据
        ssize_t ret = do_sync_read(filp, buf, len, ppos);
	//	//此处添加对文件的解密（简单移位解密，将每个字符值-25）
	int i=0;
for(i=0;i<len;i++)
{
	*(buf+i)=*(buf+i)-25;
//printk("haha encrypt %ld\n", *buf);
}

		
        printk("haha encrypt %ld\n", len);
        return ret;
}


//

/*
 * Called when filp is released. This happens when all file descriptors
 * for a single struct file are closed. Note that different open() calls
 * for the same file yield different struct file structures.
 */
static int myext2_release_file (struct inode * inode, struct file * filp)
{
	if (filp->f_mode & FMODE_WRITE) {
		mutex_lock(&MYEXT2_I(inode)->truncate_mutex);
		myext2_discard_reservation(inode);
		mutex_unlock(&MYEXT2_I(inode)->truncate_mutex);
	}
	return 0;
}

int myext2_fsync(struct file *file, loff_t start, loff_t end, int datasync)
{
	int ret;
	struct super_block *sb = file->f_mapping->host->i_sb;
	struct address_space *mapping = sb->s_bdev->bd_inode->i_mapping;

	ret = generic_file_fsync(file, start, end, datasync);
	if (ret == -EIO || test_and_clear_bit(AS_EIO, &mapping->flags)) {
		/* We don't really know where the IO error happened... */
		myext2_error(sb, __func__,
			   "detected IO error when writing metadata buffers");
		ret = -EIO;
	}
	return ret;
}

/*
 * We have mostly NULL's here: the current defaults are ok for
 * the myext2 filesystem.
 */
const struct file_operations myext2_file_operations = {
	.llseek		= generic_file_llseek,
	.read		= new_sync_read_crypt,//do_sync_read,
	.write		= new_sync_write_crypt,//do_sync_write,
	.aio_read	= generic_file_aio_read,
	.aio_write	= generic_file_aio_write,
	.unlocked_ioctl = myext2_ioctl,
#ifdef CONFIG_COMPAT
	.compat_ioctl	= myext2_compat_ioctl,
#endif
	.mmap		= generic_file_mmap,
	.open		= dquot_file_open,
	.release	= myext2_release_file,
	.fsync		= myext2_fsync,
	.splice_read	= generic_file_splice_read,
	.splice_write	= generic_file_splice_write,
};

#ifdef CONFIG_MYEXT2_FS_XIP
const struct file_operations myext2_xip_file_operations = {
	.llseek		= generic_file_llseek,
	.read		= xip_file_read,
	.write		= xip_file_write,
	.unlocked_ioctl = myext2_ioctl,
#ifdef CONFIG_COMPAT
	.compat_ioctl	= myext2_compat_ioctl,
#endif
	.mmap		= xip_file_mmap,
	.open		= dquot_file_open,
	.release	= myext2_release_file,
	.fsync		= myext2_fsync,
};
#endif

const struct inode_operations myext2_file_inode_operations = {
#ifdef CONFIG_MYEXT2_FS_XATTR
	.setxattr	= generic_setxattr,
	.getxattr	= generic_getxattr,
	.listxattr	= myext2_listxattr,
	.removexattr	= generic_removexattr,
#endif
	.setattr	= myext2_setattr,
	.get_acl	= myext2_get_acl,
	.fiemap		= myext2_fiemap,
};
