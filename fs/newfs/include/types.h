#ifndef _TYPES_H_
#define _TYPES_H_

/******************************************************************************
* SECTION: Type def
*******************************************************************************/
typedef int          boolean;
typedef uint16_t     flag16;

typedef enum nfs_file_type {
    NFS_FILE,
    NFS_DIR
} NFS_FILE_TYPE;
/******************************************************************************
* SECTION: Macro
*******************************************************************************/
#define TRUE                    1
#define FALSE                   0
#define UINT32_BITS             32
#define UINT8_BITS              8

#define NFS_MAGIC_NUM           0x52415453  
#define NFS_SUPER_OFS           0
#define NFS_ROOT_INO            0



#define NFS_ERROR_NONE          0
#define NFS_ERROR_ACCESS        EACCES
#define NFS_ERROR_SEEK          ESPIPE     
#define NFS_ERROR_ISDIR         EISDIR
#define NFS_ERROR_NOSPACE       ENOSPC
#define NFS_ERROR_EXISTS        EEXIST
#define NFS_ERROR_NOTFOUND      ENOENT
#define NFS_ERROR_UNSUPPORTED   ENXIO
#define NFS_ERROR_IO            EIO     /* Error Input/Output */
#define NFS_ERROR_INVAL         EINVAL  /* Invalid Args */

#define NFS_MAX_FILE_NAME       128
#define NFS_INODE_PER_FILE      1       /* 一个逻辑块放几个索引 */
#define NFS_DATA_PER_FILE       6       /* 一个文件多少逻辑块 */
#define NFS_DEFAULT_PERM        0777

#define NFS_IOC_MAGIC           'S'
#define NFS_IOC_SEEK            _IO(NFS_IOC_MAGIC, 0)

#define NFS_FLAG_BUF_DIRTY      0x1
#define NFS_FLAG_BUF_OCCUPY     0x2

#define NFS_SUPER_BLKS          1       /* 超级块占1个逻辑块 */
#define NFS_MAP_INODE_BLKS      1       /* 索引节点位图占1个逻辑块 */
#define NFS_MAP_DATA_BLKS       1       /* 数据块位图占1个逻辑块 */

#define NFS_INODE_BLKS          585     
#define NFS_DATA_BLKS           3508    


/******************************************************************************
* SECTION: Macro Function
*******************************************************************************/
/* 求各种基本信息 */
#define NFS_IO_SZ()                     (nfs_super.sz_io)
#define NFS_BLK_SZ()                    (nfs_super.sz_blks)
#define NFS_DISK_SZ()                   (nfs_super.sz_disk)
#define NFS_DRIVER()                    (nfs_super.fd)
#define NFS_DENTRY_PER_DATABLK()        (NFS_BLK_SZ() / sizeof(struct nfs_dentry))      //计算一个逻辑块可以储存多少dentry
#define NFS_BLKS_SZ(blks)               ((blks) * NFS_BLK_SZ())

/* 取整函数 */
#define NFS_ROUND_DOWN(value, round)    ((value) % (round) == 0 ? (value) : ((value) / (round)) * (round))
#define NFS_ROUND_UP(value, round)      ((value) % (round) == 0 ? (value) : ((value) / (round) + 1) * (round))

#define NFS_ASSIGN_FNAME(pnfs_dentry, _fname)  memcpy(pnfs_dentry->fname, _fname, strlen(_fname))


/*  求基地址*/
#define NFS_INO_OFS(ino)                (nfs_super.inode_offset + NFS_BLKS_SZ(ino))
#define NFS_DATA_OFS(dno)               (nfs_super.data_offset + NFS_BLKS_SZ(dno))


/* 判断是普通文件还是文件夹 */
#define NFS_IS_DIR(pinode)              (pinode->dentry->ftype == NFS_DIR)
#define NFS_IS_REG(pinode)              (pinode->dentry->ftype == NFS_FILE)
// #define NFS_IS_SYM_LINK(pinode)         (pinode->dentry->ftype == NFS_SYM_LINK)
/******************************************************************************
* SECTION: FS Specific Structure - In memory structure
*******************************************************************************/
// struct nfs_dentry;
// struct nfs_inode;
// struct nfs_super;

struct custom_options {
	const char*        device;
};

struct nfs_inode
{
    int                ino;                             /* 在inode位图中的下标 */
    int                size;                            /* 文件已占用空间 */
    int                link;                            /* 链接数 */
    NFS_FILE_TYPE      ftype;                           /* 文件类型 */
    int                block_pointer[NFS_DATA_PER_FILE];/* 数据块块号（可固定分配）*/
    struct nfs_dentry* dentry;                          /* 指向该inode的父dentry */
    struct nfs_dentry* dentrys;                         /* 指向该inode的所有子dentry */
    uint8_t*           data[NFS_DATA_PER_FILE];         /* 指向数据块的指针 */
    uint8_t*           data1;         
    int                dir_cnt;                         /* 如果是目录类型文件，下面有几个目录项 */
    int                block_allocted;                  /* 已分配数据块数量 */
};  

struct nfs_dentry
{
    char               fname[NFS_MAX_FILE_NAME];
    struct nfs_dentry* parent;                        /* 父亲Inode的dentry */
    struct nfs_dentry* brother;                       /* 兄弟 */
    int                ino;
    struct nfs_inode*  inode;                         /* 指向inode */
    NFS_FILE_TYPE      ftype;
};

struct nfs_super
{
    uint32_t           magic_num;
    int                fd;
    
    /* 总体磁盘情况*/
    int                sz_io;
    int                sz_blks;
    int                sz_disk;
    int                sz_usage;
    
    /* 索引节点及索引位图相关情况*/
    int                max_ino;
    uint8_t*           map_inode;
    int                map_inode_blks;
    int                map_inode_offset;   /* inode位图的起始地址*/
    int                inode_offset;
    
    /* 数据位图及数据块的相关情况 */
    int                max_dno;         
    uint8_t*           map_data;         
    int                map_data_offset;  /* data位图的起始地址*/
    int                map_data_blks;    
    int                data_offset;

    boolean            is_mounted;
    
    /* 根目录 */
    struct nfs_dentry* root_dentry;
};

static inline struct nfs_dentry* new_dentry(char * fname, NFS_FILE_TYPE ftype) {
    struct nfs_dentry * dentry = (struct nfs_dentry *)malloc(sizeof(struct nfs_dentry));
    memset(dentry, 0, sizeof(struct nfs_dentry));
    NFS_ASSIGN_FNAME(dentry, fname);
    dentry->ftype   = ftype;
    dentry->ino     = -1;
    dentry->inode   = NULL;
    dentry->parent  = NULL;
    dentry->brother = NULL;   
    return dentry;                                         
}
/******************************************************************************
* SECTION: FS Specific Structure - Disk structure
*******************************************************************************/
struct nfs_super_d
{
    uint32_t           magic_num;
    int                sz_usage;

    /* 索引节点及其位图情况*/
    int                max_ino;
    int                map_inode_blks;
    int                map_inode_offset;
    int                inode_offset;

    /* 数据块及其位图情况*/
    int                max_dno;
    int                map_data_blks;
    int                map_data_offset;
    int                data_offset;
    
};

struct nfs_inode_d
{
    uint32_t           ino;                           /* 在inode位图中的下标 */
    uint32_t           size;                          /* 文件已占用空间 */
    int                link;
    int                block_pointer[NFS_DATA_PER_FILE];
    int                dir_cnt;
    NFS_FILE_TYPE      ftype;   
    int                block_allocted;                  /* 已分配数据块数量 */

};  

struct nfs_dentry_d
{
    char               fname[NFS_MAX_FILE_NAME];
    NFS_FILE_TYPE      ftype;
    uint32_t           ino;                           /* 指向的ino号 */
};  


#endif /* _TYPES_H_ */