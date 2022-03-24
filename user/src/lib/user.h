#ifndef USER_H
#define USER_H
int exec(char*, char**);
int exit(int) __attribute__((noreturn));
int open(const char*, int);
int mknod(const char*, short, short);
int write(int, const void*, int);
int read(int, void*, int);
int close(int);
#endif /* USER_H */