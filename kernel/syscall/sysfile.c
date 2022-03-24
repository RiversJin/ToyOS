#include "arg.h"
#include "../file/fcntl.h"
#include "../include/stdint.h"
#include "../include/param.h"
#include "../file/file.h"
#include "../fs/fs.h"
#include "../fs/log.h"
#include "../proc/proc.h"
#include "../printf.h"
#include "../lib/string.h"

static int fdalloc(struct file *f){
  int fd;
  struct proc *p = myproc();
  for(fd = 0; fd < NOFILE; fd++){
    if(p->ofile[fd] == 0){
      p->ofile[fd] = f;
      return fd;
    }
  }
  return -1;
}
static int argfd(int n, int64_t *pfd, struct file **pf){
  int64_t fd;
  struct file *f;

  if(argint(n, (uint64_t*)&fd) < 0)
    return -1;
  if(fd < 0 || fd >= NOFILE || (f=myproc()->ofile[fd]) == 0)
    return -1;
  if(pfd)
    *pfd = fd;
  if(pf)
    *pf = f;
  return 0;
}

static struct inode* create(char *path, int16_t type, int16_t major, int16_t minor){
    char name[DIRSIZ];
    struct inode *dp = nameiparent(path,name);
    // 若没找到对应路径
    if(dp == NULL){
        return 0;
    }
    ilock(dp);
    struct inode *ip = dirlookup(dp,name,0);
    // 如果path上的对应节点已经存在
    if(ip != NULL){
        iunlockandput(dp);
        ilock(ip);
        if(type == T_FILE && (ip->type == T_FILE || ip->type == T_DEVICE))
            return ip;
        iunlockandput(dp);
        return 0;
    }
    if((ip = ialloc(dp->dev, type)) == NULL){
        panic("create: ialloc failed. \n");
    }
    ilock(ip);
    ip->major = major;
    ip->minor = minor;
    ip->nlink = 1;
    iupdate(ip);
    // 创建 . 和 ..
    if(type == T_DIR){
        dp->nlink ++;
        iupdate(dp);
        if(dirlink(ip,".",ip->inum) < 0 || dirlink(ip,"..",dp->inum) < 0){
            panic("create: create . and .. link failed.\n");
        }
    }
    // 将其连接到父目录
    if(dirlink(dp, name, ip->inum) < 0){
        panic("create: dir link failed.\n");
    }
    iunlockandput(dp);
    return ip;
}

// int mknod(path,major,minor)
int64_t sys_mknod(){
    char *path = NULL;
    int64_t major, minor;
    struct inode *ip;
    if(argstr(0,&path) < 0){
        return -1;
    }
    begin_op();
    if( argint(1,(uint64_t*)&major) < 0 || 
        argint(2,(uint64_t*)&minor) < 0 ||
        (ip = create(path, T_DEVICE, major, minor)) == NULL){
            end_op();
            return -1;
    }
    iunlockandput(ip);
    end_op();
    return 0;
}
// int open(const char* path, int mode);
int64_t sys_open(){
    char *path;
    int64_t fd;
    int64_t mode;
    int path_size;
    if((path_size = argstr(0, &path)) < 0 || argint(1,(uint64_t*)&mode) < 0){
        return -1;
    }
    struct inode *ip;
    struct file *file;
    begin_op();
    if(mode == O_CREATE){
        ip = create(path, T_FILE, 0, 0);
        if(ip == NULL){
            end_op();
            return -1;
        }
    } else {
        // 若非创建文件 下面是打开的逻辑
        if((ip = namei(path)) == NULL){
            end_op();
            return -1;
        }
        // 已获得当前文件的inode 即 ip
        ilock(ip);
        // 如果打开的是目录的inode, 是不许直接修改的
        if(ip->type == T_DIR && mode != O_RDONLY){
            iunlockandput(ip);
            end_op();
            return -1;
        }
    }
    // 当前的ip是有效的了
    // 如果打开的是设备文件 但是主设备号有问题
    if(ip->type == T_DEVICE && (ip->major < 0 || ip->major >= NDEV)){
        iunlockandput(ip);
        end_op();
        return -1;
    }
    // 分配file结构体以及描述符
    if((file = filealloc()) == NULL || (fd = fdalloc(file)) < 0){
        if(file != NULL){
            fileclose(file);
        }
        iunlockandput(ip);
        end_op();
        return -1;
    }
    if(ip->type == T_DEVICE){
        file->type = FD_DEVICE;
        file->major = ip->major;
    }else{
        file->type = FD_INODE;
        file->off = 0;
    }
    file->ip = ip;
    file->readable = !(mode & O_WRONLY); // 如果不是只写 那就是可读
    file->writable = (mode & O_WRONLY) || (mode & O_RDWR); // 如果设置为只写或读写 那么就是可写的
    if((mode & O_TRUNC) && ip->type == T_FILE){
        itrunc(ip); // 根据需要截断文件内容
    }
    iunlock(ip);
    end_op();
    return fd;
}
// int close(int fd);
int64_t sys_close(){
    int64_t fd;
    struct file *file;
    if(argfd(0, &fd, &file) < 0){
        return -1;
    }
    myproc()->ofile[fd] = 0;
    fileclose(file);
    return 0;
}
// int read(int, void*, int);
int64_t sys_read(){
    struct file *file;
    int64_t n;
    char *p;
    if(argfd(0,0,&file) < 0 || argint(2,(uint64_t*)&n) < 0 || argptr(1,&p,n) < 0){
        return -1;
    }
    return fileread(file,p,n);
}

int64_t sys_write(){
    struct file *file;
    int64_t n;
    char *p;
    if(argfd(0,0,&file) < 0 || argint(2,(uint64_t*)&n) < 0 || argptr(1,&p,n) < 0){
        return -1;
    }
    return filewrite(file,p,n);
}

extern int exec(char *path, char **argv);
// int exec(char* path, char**); 
int64_t sys_exec(){
    char *path;
    char *argv[MAXARG];
    uint64_t uargv;
    int64_t uarg = 0;
    if(argstr(0,&path) < 0 || argint(1,(uint64_t*)&uargv) < 0){
        cprintf("sys_exec: invalid arguments\n");
        return -1;
    }
    
    memset(argv,0,sizeof(argv));
    for(int i=0; ; ++i){
        if(i >= ARRAY_SIZE(argv)){
            cprintf("sys_exec: too many arguments.\n");
            return -1;
        }
        if (fetchint64ataddr(uargv + sizeof(uint64_t) * i, (uint64_t*)&uarg) < 0) {
            cprintf("sys_exec: failed to fetch uarg.\n");
            return -1;
        }
        if(uarg == 0){
            argv[i] = 0;
            break;
        }
        if(fetchstr(uarg, &argv[i]) < 0) {
            cprintf("sys_exec: failed to fetch argument.\n");
            return -1;
        }
        cprintf("sys_exec: argv[%d] = '%s'\n", i, argv[i]);
    }
    return exec(path,argv);
}