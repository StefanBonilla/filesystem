#ifndef _MINSHARED_H
#define _MINSHARED_H
#include "fs.h"
#include "inode.h"

extern void getInode(FILE *, uint32_t, struct superblock *, struct inode *);
extern void printSuperBlock(struct superblock *);
extern void printiNode(struct inode *);
extern void seekZone(FILE *, struct superblock *, uint32_t);
extern int navZone(FILE *, struct inode *, struct superblock *, int, char *, 
   int *, int);
extern struct inode * findDir(FILE *, struct inode *, struct superblock *, 
   char *);
extern void printPartitionTable(FILE *);
extern void validatePartTable(FILE *);
extern void partIsMinix(struct part_entry *);
extern void seekPartition(FILE *, int, int);
extern void findFileSystem(FILE *, int, int, int);
extern struct superblock * getfsSuperblock(FILE *, struct cmdLine *);

#endif /* _MINSHARED_H */
