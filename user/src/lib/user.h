#ifndef USER_H
#define USER_H

int exit(int) __attribute__((noreturn));
int open(const char*, int);
int mknod(const char*, short, short);
#endif /* USER_H */