#ifndef STAT_H
#define STAT_H
#include "stdint.h"

#define T_DIR     1   // Directory
#define T_FILE    2   // File
#define T_DEVICE  3   // Device

struct stat {
  int dev;     // File system's disk device
  uint32_t ino;    // Inode number
  short type;  // Type of file
  short nlink; // Number of links to file
  uint64_t size; // Size of file in bytes
};


#endif /* STAT_H */