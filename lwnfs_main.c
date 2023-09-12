// SPDX-License-Identifier: GPL-2.0

#include <linux/fs.h>
#include <linux/init.h>
#include <linux/module.h>

#define LFS_MAGIC	0xdeadbeef
#define TMPSIZE		512

static atomic_t counter;
static struct inode *lfs_make_inode(struct super_block *sb, int mode);

static ssize_t lfs_read_file(struct file *filp, char *buf,
		size_t count, loff_t *offset)
{
	int len;
	char tmp[TMPSIZE];
	atomic_t *counter = (atomic_t *) filp->private_data;
	int v = atomic_read(counter);
	atomic_inc(counter);

	len = snprintf(tmp, TMPSIZE, "%d\n", v);
	if (*offset > len)
		return 0;
	if (count > len - *offset)
		count = len - *offset;

	if (copy_to_user(buf, tmp + *offset, count))
		return -EFAULT;
	*offset += count;
	return count;
}

static ssize_t lfs_write_file(struct file *filp, const char *buf,
		size_t count, loff_t *offset)
{
	atomic_t *counter = (atomic_t *) filp->private_data;
	char tmp[TMPSIZE];

	if (*offset != 0)
		return -EINVAL;
	if (count >= TMPSIZE)
		return -EINVAL;

	memset(tmp, 0, TMPSIZE);
	if (copy_from_user(tmp, buf, count))
		return -EFAULT;
	atomic_set(counter, simple_strtol(tmp, NULL, 10));
	return count;
}

static int lfs_open(struct inode *inode, struct file *filp)
{
	filp->private_data = inode->i_private;
	return 0;
}

static struct file_operations lfs_file_ops = {
	.open	= lfs_open,
	.read 	= lfs_read_file,
	.write  = lfs_write_file,
};

const struct inode_operations lfs_file_inode_operations = {
        .setattr        = simple_setattr,
        .getattr        = simple_getattr,
};

static struct dentry *lfs_create_file (struct super_block *sb,
		struct dentry *dir, const char *name,
		atomic_t *counter)
{
	struct dentry *dentry;
	struct inode *inode;
	struct qstr qname;

	qname.name = name;
	qname.len = strlen (name);
	qname.hash = full_name_hash(dir, name, qname.len);
	dentry = d_alloc(dir, &qname);

	inode = lfs_make_inode(sb, S_IFREG | 0644);
	if (! inode)
		goto out_dput;
	inode->i_fop = &lfs_file_ops;
	inode->i_op = &lfs_file_inode_operations;
	inode->i_private = counter;

	d_add(dentry, inode);
	return dentry;
out_dput:
	return NULL;
}

static void lfs_create_files (struct super_block *sb, struct dentry *root)
{
        /* ... */
	atomic_set(&counter, 0);
	lfs_create_file(sb, root, "counter", &counter);
	/* ... */
}

static struct inode *lfs_make_inode(struct super_block *sb, int mode)
{
	struct inode *ret = new_inode(sb);

	if (ret) {
		ret->i_ino = get_next_ino();
		ret->i_mode = mode;
		// ret->i_blksize = PAGE_SIZE;
		ret->i_blocks = 0;
		ret->i_atime = ret->i_mtime = ret->i_ctime = current_time(ret);
	}
	return ret;
}

static struct super_operations lfs_s_ops = {
	.statfs		= simple_statfs,
	.drop_inode	= generic_delete_inode,
};

static int lfs_fill_super (struct super_block *sb, void *data, int silent)
{
	static const struct tree_descr debug_files[] = {{""}};
	struct inode *root;
	int err;

	err  =  simple_fill_super(sb, LFS_MAGIC, debug_files);
	if (err) {
		goto fail;
	}
	sb->s_op = &lfs_s_ops;

	// create the root dentry
	root = lfs_make_inode (sb, S_IFDIR | 0755);
	if (root) {
		root->i_op = &simple_dir_inode_operations;
		root->i_fop = &simple_dir_operations;
	}
	sb->s_root = d_make_root(root);
	if (!sb->s_root) {
		err = -ENOMEM;
		goto fail;
	}
	lfs_create_files(sb, sb->s_root);
	return 0;

fail:
	return err;
}

static struct dentry *lfs_get_super(struct file_system_type *fst,
		int flags, const char *devname, void *data)
{
	return mount_single(fst, flags, data, lfs_fill_super);
}

static struct file_system_type lfs_type = {
	.name		= "lwnfs",
	.mount		= lfs_get_super,
	// .kill_sb	= kill_litter_super,
};

static int __init lfs_init(void)
{
	return register_filesystem(&lfs_type);
}
module_init(lfs_init);

MODULE_LICENSE("GPL v2");
MODULE_AUTHOR("suxiaocheng");
