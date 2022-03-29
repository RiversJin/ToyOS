#ifndef USER_H
#define USER_H

typedef signed char int8_t;
typedef unsigned char uint8_t;

typedef signed short int16_t;
typedef unsigned short uint16_t;

typedef signed int int32_t;
typedef unsigned int uint32_t; 

typedef signed long int int64_t;
typedef unsigned long int uint64_t;

typedef unsigned long size_t;

typedef unsigned int   uint;
typedef unsigned short ushort;
typedef unsigned char  uchar;

typedef unsigned char uint8;
typedef unsigned short uint16;
typedef unsigned int  uint32;
typedef unsigned long uint64;

#define ARRAY_SIZE(a) (sizeof(a) / sizeof(a[0]))

#define DIRSIZ 14

struct dirent {
    uint16_t inum;
    char name[DIRSIZ];
};

#undef NULL
#if defined(__cplusplus)
#define NULL 0
#else
#define NULL ((void *)0)
#endif

#define O_RDONLY  0x000
#define O_WRONLY  0x001
#define O_RDWR    0x002
#define O_CREATE  0x200
#define O_TRUNC   0x400

#define T_DIR     1   // Directory
#define T_FILE    2   // File
#define T_DEVICE  3   // Device

#define CONSOLE 1

struct stat {
  int dev;     // File system's disk device
  uint32_t ino;    // Inode number
  short type;  // Type of file
  short nlink; // Number of links to file
  uint64_t size; // Size of file in bytes
};

int exec(char*, char**);
int exit(int) __attribute__((noreturn));
int getpid();
int fork();
int wait(int*);
int pipe(int (*)[2]);
void yield();
int chdir(const char*);
int kill(int pid);
char* sbrk(int);
int sleep(int);
int uptime(void);

int fstat(int fd, struct stat * stat);
int mknod(const char*, short, short);
int mkdir(const char*);
int open(const char*, int);
int close(int);
int read(int, void*, int);
int write(int, const void*, int);
int dup(int);
int link(const char*, const char*);
int unlink(const char*);


// ulib.c
int stat(const char*, struct stat*);
char* strcpy(char*, const char*);
void *memmove(void*, const void*, int);
char* strchr(const char*, char c);
int strcmp(const char*, const char*);
void fprintf(int, const char*, ...);
void printf(const char*, ...);
char* gets(char*, int max);
uint strlen(const char*);
void* memset(void*, int, uint);
void* malloc(uint);
void free(void*);
int atoi(const char*);
int memcmp(const void *, const void *, uint);
void *memcpy(void *, const void *, uint);


#endif /* USER_H */