#ifndef USER_H
#define USER_H
int exec(char*, char**);
int exit(int) __attribute__((noreturn));
int getpid();
int fork();
int wait(int*);
void yield();

int mknod(const char*, short, short);
int open(const char*, int);
int close(int);
int read(int, void*, int);
int write(int, const void*, int);
int dup(int);


#endif /* USER_H */