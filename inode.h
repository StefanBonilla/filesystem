#ifndef _INODE_H
#define _INODE_H

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <inttypes.h>
#include "fs.h"

/* permission string constants and indices */
#define PERM_STR_LEN             10
#define FILE_TYPE                0
#define OWNER_R                  1
#define OWNER_W                  2
#define OWNER_X                  3
#define GROUP_R                  4
#define GROUP_W                  5
#define GROUP_X                  6
#define OTHER_R                  7
#define OTHER_W                  8
#define OTHER_X                  9

extern char * getPermissionString(struct inode *);

#endif /* _INODE_H */
