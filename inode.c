#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <inttypes.h>
#include <unistd.h>
#include <string.h>
#include "inode.h"

char * getPermissionString(struct inode * inode) {
    char * perm = (char *)malloc(sizeof(char) * PERM_STR_LEN);
    uint16_t mode = inode->mode;
    memset(perm, '-', PERM_STR_LEN); /* set string to all '-' */

    if (mode & DIR_MASK) {
        perm[FILE_TYPE] = 'd';
    }
    if (mode & OWNER_RD_PERM) {
        perm[OWNER_R] = 'r';
    }
    if (mode & OWNER_WR_PERM) {
        perm[OWNER_W] = 'w';
    }
    if (mode & OWNER_EX_PERM) {
        perm[OWNER_X] = 'x';
    }
    if (mode & GROUP_RD_PERM) {
        perm[GROUP_R] = 'r';
    }
    if (mode & GROUP_WR_PERM) {
        perm[GROUP_W] = 'w';
    }
    if (mode & GROUP_EX_PERM) {
        perm[GROUP_X] = 'x';
    }
    if (mode & OTHER_RD_PERM) {
        perm[OTHER_R] = 'r';
    }
    if (mode & OTHER_WR_PERM) {
        perm[OTHER_W] = 'w';
    }
    if (mode & OTHER_EX_PERM) {
        perm[OTHER_X] = 'x';
    }

    return perm;
}

int isDir(struct inode * inode) {
    return inode->mode & DIR_MASK;
}
