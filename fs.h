#ifndef _FS_H
#define _FS_H

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <inttypes.h>

#define NUM_PARTITIONS           4
#define SECTOR_SIZE              512
#define DIRECT_ZONES             7
#define DIRENT_NAME_LEN          60

#define BS_BYTE_510              510

/*navigation stuff*/
#define LOC_PARTITION_TABLE      0x1BE
#define MINIX_PART_TYPE          0x81
#define BOOT_SECTOR_BYTE_510     0x55
#define BOOT_SECTOR_BYTE_511     0xAA
#define MINIX_MAGIC_NUM          0x4D5A
#define MINIX_MAGIC_NUM_FLIP     0x5A4D
#define INODE_SIZE               64
#define DIRENT_SIZE              64

/*file type constants*/
#define FILE_TYPE_MASK           0170000
#define REG_FILE_MASK            0100000
#define DIR_MASK                 0040000
#define OWNER_RD_PERM            0000400
#define OWNER_WR_PERM            0000200
#define OWNER_EX_PERM            0000100
#define GROUP_RD_PERM            0000040
#define GROUP_WR_PERM            0000020
#define GROUP_EX_PERM            0000010
#define OTHER_RD_PERM            0000004
#define OTHER_WR_PERM            0000002
#define OTHER_EX_PERM            0000001

struct dirent
{
   uint32_t inode;
   unsigned char filename[DIRENT_NAME_LEN];
};

struct superblock 
{ 
  /* Minix Version 3 Superblock
   * this structure found in fs/super.h
   * * in minix 3.1.1
   * */
   /* on disk. These fields and orientation are non–negotiable    */
   uint32_t ninodes;      /* number of inodes in this filesystem  */
   uint16_t pad1;         /* make things line up properly         */
   int16_t i_blocks;      /* # of blocks used by inode bit map    */
   int16_t z_blocks;      /* # of blocks used by zone bit map     */
   uint16_t firstdata;    /* number of first data zone            */
   int16_t log_zone_size; /* log2 of blocks per zone              */
   int16_t pad2;          /* make things line up again            */
   uint32_t max_file;     /* maximum file size                    */
   uint32_t zones;        /* number of zones on disk              */
   int16_t magic;         /* magic number                         */
   int16_t pad3;          /* make things line up again            */
   uint16_t blocksize;    /* block size in bytes                  */
   uint8_t subversion;    /* filesystem sub–version               */
};

struct part_entry 
{
   uint8_t bootind;     /* boot indicator 0/ACTIVE_FLAG  */
   uint8_t start_head;  /* head value for first sector   */
   uint8_t start_sec;   /* sector value + cyl bits for first sector */
   uint8_t start_cyl;   /* track value for first sector  */
   uint8_t sysind;      /* system indicator    */
   uint8_t last_head;   /* head value for last sector  */
   uint8_t last_sec;    /* sector value + cyl bits for last sector */
   uint8_t last_cyl;    /* track value for last sector   */
   uint32_t lowsec;      /* logical first sector    */
   uint32_t size;        /* size of partition in sectors  */
};

struct inode 
{
   uint16_t mode;                                                 
   uint16_t links;              
   uint16_t uid;              
   uint16_t gid;             
   uint32_t size;               
   int32_t atime;             
   int32_t mtime;               
   int32_t ctime;               
   uint32_t zone[DIRECT_ZONES]; 
   uint32_t indirect;           
   uint32_t two_indirect;       
   uint32_t unused;             
};

#endif /* _FS_H */
