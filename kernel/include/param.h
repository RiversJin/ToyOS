#ifndef PARAM_H
#define PARAM_H

#define NCPU 4 // cpu数量
#define NPROC 64 // 最大进程数
#define NOFILE 16 // 每个进程最大可以打开的文件数量
#define KSTACKSIZE 4096 // 每个进程内核栈的大小


#define MAXOPBLOCKS 10 // 每次事务所允许的最大操作块数量
#define NBUF (MAXOPBLOCKS * 3) // buffer缓存大小
#define LOGSIZE (MAXOPBLOCKS * 3)

// mkfs only
#define FSSIZE 1000  // Size of file system in blocks

#endif /* PARAM_H */