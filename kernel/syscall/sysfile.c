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
#include "../pipe/pipe.h"

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
    if(mode & O_CREATE){
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
        //cprintf("sys_exec: argv[%d] = '%s'\n", i, argv[i]);
    }
    return exec(path,argv);
}

// int dup(int);
int64_t sys_dup(){
    struct file *file;
    int64_t fd;
    if(argfd(0,NULL,&file) < 0) {
        cprintf("sys_dup: failed to fetch fd.\n");
        return -1;
    }
    if((fd = fdalloc(file)) < 0){
        cprintf("sys_dup: failed to allocate file.\n");
        return -1;
    }
    filedup(file);
    return fd;
}

// int chdir(const char*);
int64_t sys_chdir(){
    char *path;
    struct proc *p = myproc();
    if(argstr(0,&path) < 0) {
        return -1;
    }
    begin_op();
    struct inode *ip = namei(path);
    if(ip == NULL){
        end_op();
        return -1;
    }
    ilock(ip);
    if(ip->type != T_DIR){
        iunlockandput(ip);
        end_op();
        return -1;
    }
    iunlock(ip);
    iput(p->cwd);
    end_op();
    p->cwd = ip;
    return 0;
}

int64_t sys_fstat(){
    struct file *f;
    struct stat *st;
    if(argfd(0,0,&f) < 0 || argptr(1,(char**)&st,sizeof(struct stat)) < 0){
        return -1;
    }
    return filestat(f,st);
}
// int mkdir(path);
int64_t sys_mkdir(){
    char *path;
    if(argstr(0, &path) < 0) return -1;
    begin_op();
    struct inode *ip = create(path,T_DIR,0,0);
    if(ip == NULL){
        end_op();
        return -1;
    }
    iunlockandput(ip);
    end_op();
    return 0;
}
// int link(const char*, const char*);
int64_t sys_link(){
    char *old;
    char *new;
    if(argstr(0,&old) < 0 || argstr(1,&new) < 0){
        return -1;
    }
    struct inode *dp, *ip;
    begin_op();
    if((ip = namei(old)) == NULL){
        end_op();
        return -1;
    }
    ilock(ip);
    if(ip->type == T_DIR){
        iunlockandput(ip); // 不可以link目录
        end_op();
        return -1;
    }
    ip->nlink ++;
    iupdate(ip);
    iunlock(ip);
    char name[DIRSIZ];
    if((dp = nameiparent(new,name)) == NULL)
        goto bad;
    ilock(dp);
    if(dp->dev != ip->dev || dirlink(dp,name,ip->inum) < 0){
        iunlockandput(dp);
        goto bad;
    }
    iunlockandput(dp);
    iput(ip);
    end_op();
    return 0;

    bad:
    ilock(ip);
    ip->nlink--;
    iupdate(ip);
    iunlockandput(ip);
    end_op();
    return -1;
}
// 如果此目录只包含.和.. 视为空
static int is_dir_empty(struct inode *dp){
    int off;
    struct dirent de;
    // 跳过前两个 因为是 .和..
    for(off = 2*sizeof(de); off < dp->size; off += sizeof(de)){
        if(readi(dp,(char*)&de,off,sizeof(de)) != sizeof(de)){
            panic("is_dir_empty: readi failed.\n");
        }
        if(de.inum != 0)
            return 0;
    }
    return 1;
}
// int unlink(const char*);
int64_t sys_unlink(void){
    char *path;
    if(argstr(0,&path) < 0){
        return -1;
    }
    struct inode *dp, *ip;
    char name[DIRSIZ];
    begin_op();
    if((dp = nameiparent(path,name)) == NULL){
        end_op();
        return -1;
    }
    ilock(dp);
    // . 和 .. 是不行的
    if(namecmp(name,".") == 0 || namecmp(name,".."))
        goto bad;
    struct dirent de;
    uint32_t off;
    if((ip = dirlookup(dp, name, &off)) == NULL)
        goto bad;
    ilock(ip);

    if(ip->nlink < 1){
        panic("unlink: nlink < 1\n");
    }
    if(ip->type == T_DIR && !is_dir_empty(ip)){
        // 此文件夹非空是不可以的
        iunlockandput(ip);
        goto bad;
    }
    memset(&de,0,sizeof(de));
    if(writei(dp,(char*)&de,off,sizeof(de)) != sizeof(de)){
        panic("unlink: writei failed.\n");
    }
    if(ip->type == T_DIR){
        dp->nlink --;
        iupdate(dp);
    }
    iunlockandput(dp);

    ip->nlink -- ;
    iupdate(ip);
    iunlockandput(ip);
    end_op();
    return 0;

    bad:
        iunlockandput(dp);
        end_op();
        return -1;
}

int64_t sys_pipe(void){
    int (*fdarray)[2];
    if(argptr(0,(char**)&fdarray,sizeof(fdarray)) < 0){
        return -1;
    }
    struct file *rf, *wf;
    if(pipealloc(&rf,&wf) < 0){
        // 第一个读 第二个写
        return -1;
    }
    struct proc *p = myproc();
    int fd0 = -1, fd1 = -1;
    if((fd0 = fdalloc(rf)) < 0 || (fd1 = fdalloc(wf)) < 0){
        if(fd0 >= 0){
            p->ofile[fd0] = NULL;
        }
        fileclose(rf);
        fileclose(wf);
        return -1;
    }
    (*fdarray)[0] = fd0;
    (*fdarray)[1] = fd1;
    return 0;
}