/* @author Denis Deryugin
 * @date 17 Mar 2015
 *
 * Dumb VFS
 */

#ifndef _DVFS_H_
#define _DVFS_H_

#include <drivers/block_dev.h>
#include <framework/mod/options.h>
#include <fs/idesc.h>
#include <fs/file_desc.h>
#include <fs/file_system.h>
#include <util/dlist.h>
#include <fs/file_operation.h>
#include <fs/inode.h>

/*****************
 New VFS prototype
 *****************/

#include <config/embox/fs/dvfs/core.h>
#define DENTRY_NAME_LEN   OPTION_MODULE_GET(embox__fs__dvfs__core, NUMBER, dentry_name_len)
#define DVFS_MAX_PATH_LEN OPTION_MODULE_GET(embox__fs__dvfs__core, NUMBER, max_path_len)
#define FS_NAME_LEN       16

#define DVFS_PATH_FULL     0x001
#define DVFS_PATH_FS       0x002
#define DVFS_NAME          0x004
#define DVFS_DIR_VIRTUAL   0x01000000
#define DVFS_CHILD_VIRTUAL 0x02000000
#define DVFS_MOUNT_POINT   0x04000000
#define DVFS_NO_LSEEK      0x08000000
#define DVFS_DISCONNECTED  0x10000000

#define FILE_TYPE(flags, ftype) ((((flags) & S_IFMT) == (ftype)) ? (ftype) : 0)

struct dentry;
struct dir_ctx;
struct dumb_fs_driver;
struct file_desc;
struct inode;
struct super_block;
struct lookup;

struct super_block {
	const struct dumb_fs_driver *fs_drv; /* Assume that all FS have single driver */
	struct block_dev *bdev;
	struct file_desc      *bdev_file;
	struct dentry     *root;
	struct dlist_head *inode_list;

	struct super_block_operations *sb_ops;
	struct inode_operations	      *sb_iops;
	struct dentry_operations      *sb_dops;
	struct file_operations        *sb_fops;

	void *sb_data;
};

struct super_block_operations {
	struct inode *(*alloc_inode)(struct super_block *sb);
	int           (*destroy_inode)(struct inode *inode);
	int           (*write_inode)(struct inode *inode);
	int           (*umount_begin)(struct super_block *sb);
	struct idesc *(*open_idesc)(struct lookup *l, int __oflag);
};

struct inode_operations {
	int           (*create)(struct inode *i_new, struct inode *i_dir, int mode);
	struct inode *(*lookup)(char const *name, struct dentry const *dir);
	int           (*remove)(struct inode *inode);
	int           (*mkdir)(struct dentry *d_new, struct dentry *d_parent);
	int           (*rmdir)(struct dentry *dir);
	int           (*truncate)(struct inode *inode, size_t len);
	int           (*pathname)(struct inode *inode, char *buf, int flags);
	int           (*iterate)(struct inode *next, struct inode *parent, struct dir_ctx *ctx);
	int           (*rename)(struct inode *node, struct inode *new_parent, const char *new_name);
	int           (*getxattr)(struct inode *node, const char *name, char *value, size_t size);
	int           (*setxattr)(struct inode *node, const char *name, const char *value, size_t size, int flags);
};

struct dentry {
	char name[DENTRY_NAME_LEN];

	int flags;
	int usage_count;

	struct inode *d_inode;
	struct super_block *d_sb;

	struct dentry     *parent;
	struct dlist_head children; /* Subelements of directory */
	struct dlist_head children_lnk;

	struct dlist_head d_lnk;   /* List for all dentries in system */

	struct dentry_operations *d_ops;
};

struct dentry_operations {

};

struct dumb_fs_driver {
	const char name[FS_NAME_LEN];
	int (*format)(struct block_dev *dev, void *priv);
	int (*fill_sb)(struct super_block *sb, struct file_desc *dev);
	int (*mount_end)(struct super_block *sb);
	int (*clean_sb)(struct super_block *sb);
};

struct auto_mount {
	char mount_path[DVFS_MAX_PATH_LEN];
	struct dumb_fs_driver *fs_driver;
};

struct dvfsmnt {
	struct dvfsmnt *mnt_parent;
	struct dentry  *mnt_mountpoint;
	struct dentry  *mnt_root;
	struct super_block *mnt_sb;

	struct dlist_head children;
	struct dlist_head children_lnk;

	int flags;
};

struct dir_ctx {
	int   flags;
	void *fs_ctx;
};

#define DFS_CREAT 0x0001
extern struct inode  *dvfs_alloc_inode(struct super_block *sb);
extern int            dvfs_destroy_inode(struct inode *inode);

extern struct dentry *dvfs_alloc_dentry(void);
extern int            dvfs_destroy_dentry(struct dentry *dentry);
extern int            dvfs_fs_dentry_try_free(struct super_block *sb);

extern struct dvfsmnt *dvfs_alloc_mnt(void);
extern int             dvfs_destroy_mnt(struct dvfsmnt *mnt);

struct lookup {
	struct dentry *item;
	struct dentry *parent;
};

extern struct dentry *dvfs_root(void);
extern int dvfs_lookup(const char *path, struct lookup *lookup);
extern int dvfs_remove(const char *path);
struct idesc *dvfs_file_open_idesc(struct lookup *lookup, int __oflag);
extern int dvfs_close(struct file_desc *desc);
extern int dvfs_write(struct file_desc *desc, char *buf, int count);
extern int dvfs_read(struct file_desc *desc, char *buf, int count);
extern int dvfs_fstat(struct file_desc *desc, struct stat *sb);
extern int dvfs_iterate(struct lookup *lookup, struct dir_ctx *ctx);
extern int dvfs_pathname(struct inode *inode, char *buf, int flags);
extern int dvfs_create_new(const char *name, struct lookup *lookup, int flags);
extern int dvfs_rename(struct dentry *from, struct dentry *to);

/* dcache-related stuff */
extern struct dentry *dvfs_cache_lookup(const char *path, struct dentry *base);
extern struct dentry *dvfs_cache_get(char *path, struct lookup *lookup);
extern int dvfs_cache_del(struct dentry *dentry);
extern int dvfs_cache_add(struct dentry *dentry);

extern struct super_block *dvfs_alloc_sb(const struct dumb_fs_driver *drv, struct file_desc *bdev_file);
extern int dvfs_destroy_sb(struct super_block *sb);
extern const struct dumb_fs_driver *dumb_fs_driver_find(const char *name);
struct super_block *dumb_fs_fill_sb(struct super_block *sb, struct file_desc *bdev);

extern int dvfs_mount(const char *dev, const char *dest, const char *fstype, int flags);
extern int dvfs_umount(struct dentry *d);

extern int dentry_fill(struct super_block *, struct inode *,
                       struct dentry *d, struct dentry *parent);
extern void dentry_upd_flags(struct dentry *dentry);
extern int dentry_full_path(struct dentry *dentry, char *buf);
extern int dentry_ref_inc(struct dentry *dentry);
extern int dentry_ref_dec(struct dentry *dentry);
extern int dentry_disconnect(struct dentry *dentry);
extern int dentry_reconnect(struct dentry *parent, const char *name);

extern int dvfs_bdev_read(
		struct file_desc *bdev_file,
		char *buff,
		size_t count,
		int blkno);

extern int dvfs_bdev_write(
		struct file_desc *bdev_file,
		char *buff,
		size_t count,
		int blkno);
#endif
