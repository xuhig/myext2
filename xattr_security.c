/*
 * linux/fs/myext2/xattr_security.c
 * Handler for storing security labels as extended attributes.
 */

#include "myext2.h"
#include <linux/security.h>
#include "xattr.h"

static size_t
myext2_xattr_security_list(struct dentry *dentry, char *list, size_t list_size,
			 const char *name, size_t name_len, int type)
{
	const int prefix_len = XATTR_SECURITY_PREFIX_LEN;
	const size_t total_len = prefix_len + name_len + 1;

	if (list && total_len <= list_size) {
		memcpy(list, XATTR_SECURITY_PREFIX, prefix_len);
		memcpy(list+prefix_len, name, name_len);
		list[prefix_len + name_len] = '\0';
	}
	return total_len;
}

static int
myext2_xattr_security_get(struct dentry *dentry, const char *name,
		       void *buffer, size_t size, int type)
{
	if (strcmp(name, "") == 0)
		return -EINVAL;
	return myext2_xattr_get(dentry->d_inode, MYEXT2_XATTR_INDEX_SECURITY, name,
			      buffer, size);
}

static int
myext2_xattr_security_set(struct dentry *dentry, const char *name,
		const void *value, size_t size, int flags, int type)
{
	if (strcmp(name, "") == 0)
		return -EINVAL;
	return myext2_xattr_set(dentry->d_inode, MYEXT2_XATTR_INDEX_SECURITY, name,
			      value, size, flags);
}

int myext2_initxattrs(struct inode *inode, const struct xattr *xattr_array,
		    void *fs_info)
{
	const struct xattr *xattr;
	int err = 0;

	for (xattr = xattr_array; xattr->name != NULL; xattr++) {
		err = myext2_xattr_set(inode, MYEXT2_XATTR_INDEX_SECURITY,
				     xattr->name, xattr->value,
				     xattr->value_len, 0);
		if (err < 0)
			break;
	}
	return err;
}

int
myext2_init_security(struct inode *inode, struct inode *dir,
		   const struct qstr *qstr)
{
	return security_inode_init_security(inode, dir, qstr,
					    &myext2_initxattrs, NULL);
}

const struct xattr_handler myext2_xattr_security_handler = {
	.prefix	= XATTR_SECURITY_PREFIX,
	.list	= myext2_xattr_security_list,
	.get	= myext2_xattr_security_get,
	.set	= myext2_xattr_security_set,
};
