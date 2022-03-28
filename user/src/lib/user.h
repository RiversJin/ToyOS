#ifndef USER_H
#define USER_H
int exec(char*, char**);
int exit(int) __attribute__((noreturn));
int getpid();
int fork();
int wait(int*);
void yield();
int chdir(const char*);
int kill(int pid);
char* sbrk(int);

int fstat(int fd, struct stat*);
int mknod(const char*, short, short);
int mkdir(const char*);
int open(const char*, int);
int close(int);
int read(int, void*, int);
int write(int, const void*, int);
int dup(int);
int link(const char*, const char*);
int unlink(const char*);


#endif /* USER_H */