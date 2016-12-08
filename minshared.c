#include "fs.h"
#include "inode.h"
#include "minshared.h"
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <inttypes.h>
#include <unistd.h>
#include <ctype.h>
#include <getopt.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>

void printSuperBlock(struct superblock * superBlock)
{
   uint32_t zonesize = superBlock->blocksize << superBlock->log_zone_size;
   uint32_t addr;

    printf("Superblock Contents: \nStored Fields: \n");
    printf("   ninodes %10d\n", superBlock->ninodes);
    printf("   i_blocks %9d\n", superBlock->i_blocks);
    printf("   z_blocks %9d\n", superBlock->z_blocks);
    printf("   firstdata %8d\n", superBlock->firstdata);
    printf("   log_zone_size %4d\n", superBlock->log_zone_size);
    printf("   max_file %9u\n", superBlock->max_file);
    printf("   magic %12x\n", superBlock->magic);
    printf("   zones %12d\n", superBlock->zones);
    printf("   blocksize %8d\n", superBlock->blocksize);
    printf("   subversion %7d\n", superBlock->subversion);
   printf("Computed Fields: \n");
   printf("   version: %9d\n", MINIX_VERSION);
   addr = 2 * superBlock->blocksize / zonesize;
   printf("   firstImap %8d\n", addr);
   addr += superBlock->i_blocks * superBlock->blocksize / zonesize;
   printf("   firstZmap %8d\n", addr);
   addr += superBlock->z_blocks * superBlock->blocksize / zonesize;
   printf("   firstIBlock %6d\n", addr);   
   printf("   zonesize %9d\n", zonesize);
   printf("   ptrs_per_zone %4d\n", (int)(zonesize / sizeof(uint32_t)));
   printf("   ino_per_block %4d\n", superBlock->blocksize / INODE_SIZE);
   printf("   wrongended %7d\n", 0);
   printf("   fileent_size %5d\n", DIRENT_SIZE);
   printf("   max_filename %5d\n", DIRENT_NAME_LEN);
   printf("   ent_per_zone %5d\n\n", zonesize / DIRENT_SIZE);



}

void printiNode(struct inode * inode)
{
   time_t temp;
   printf("File inode: \n");
   printf("  uint16_t mode %15x\n", inode->mode);
   printf("  uint16_t links %15d\n", inode->links);
   printf("  uint16_t uid %15d\n", inode->uid);
   printf("  uint16_t gid %15d\n", inode->gid);
   printf("  uint16_t size %15d\n", inode->size);
   temp = (time_t) inode->atime;
   printf("  uint32_t atime %15d --- %s", 
      inode->atime, ctime(&temp));
   temp = (time_t) inode->mtime;
   printf("  uint32_t mtime %15d --- %s", 
      inode->mtime, ctime(&temp));
   temp = (time_t) inode->ctime;
   printf("  uint32_t ctime %15d --- %s", 
      inode->ctime, ctime(&temp));
   printf("Direct Zones: \n");
   printf("      zone[0] = %15d\n", inode->zone[0]);
   printf("      zone[1] = %15d\n", inode->zone[1]);
   printf("      zone[2] = %15d\n", inode->zone[2]);
   printf("      zone[3] = %15d\n", inode->zone[3]);
   printf("      zone[4] = %15d\n", inode->zone[4]);
   printf("      zone[5] = %15d\n", inode->zone[5]);
   printf("      zone[6] = %15d\n", inode->zone[6]);
   printf("  uint32_t indirect %15d\n", inode->indirect);
   printf("  uint32_t double %15d\n\n", inode->two_indirect);
}

void seekZone(FILE * fImage, struct superblock * superBlock, 
   uint32_t zonenum)
{
   uint32_t zonesize = superBlock->blocksize << superBlock->log_zone_size;

   /*assume fImage is at the bottom of the disk Image*/
   fseek(fImage, zonesize * zonenum, SEEK_CUR);

}

int navZone(FILE * fImage, struct inode * nextNode,
   struct superblock * superBlock, int zoneNum, char * pathToken, 
   int * totalEntriesRead, int numEntriesInDir) 
{
   uint32_t zonesize = superBlock->blocksize << superBlock->log_zone_size;
   struct dirent * currDirent = malloc(sizeof(struct dirent));
   int entriesPerZone = zonesize / DIRENT_SIZE;
   int i;
   int base = ftell(fImage);

   seekZone(fImage, superBlock, zoneNum);
   for(i = 0; i < entriesPerZone; i++)
   {
      if(*totalEntriesRead + 1 > numEntriesInDir)
      {
         fprintf(stderr, "file/directory does not exist\n");
         exit(EXIT_FAILURE);
      }
      (*totalEntriesRead)++;
      fread(currDirent, sizeof(struct dirent), 1, fImage);

      if(!strcmp((const char *)currDirent->filename, pathToken) && 
         currDirent->inode)
      {
         /*we have a hit, get the inode*/
         fseek(fImage, base, SEEK_SET);
         getInode(fImage, currDirent->inode, superBlock, 
            nextNode);
         return ENTRY_FOUND;
      }
   }

   return ENTRY_MISSING_IN_ZONE;
}

struct inode * findDir(FILE * fImage, struct inode * inode, 
   struct superblock * superBlock, char * path)
{
   int numEntriesInDir;
   int currZone = 0;
   int numEntriesRead;
   int base = ftell(fImage);
   int res = 0;

   struct inode * nextNode;
   char * pathToken = strtok(path, PATH_DELIM);
   int tokenFlag = 0;


   nextNode = inode;

   while(pathToken)
   {
      tokenFlag = 0;
      numEntriesRead = 0;
      numEntriesInDir = nextNode->size / DIRENT_SIZE;

      /*iterate through each of the inode's zones*/
      for (currZone = 0; currZone < 7; currZone++)
      {
         /*check if valid zone number*/
         if(!nextNode->zone[currZone])
         {
            fprintf(stderr, "file/directory does not exist\n");
            exit(EXIT_FAILURE);
         }
         fseek(fImage, base, SEEK_SET);

         /*iterate through the entries of the zones*/
         res = navZone(fImage, nextNode, superBlock, 
            nextNode->zone[currZone], pathToken, 
            &numEntriesRead, numEntriesInDir);
         if(res == ENTRY_FOUND)
         {
            pathToken = strtok(NULL, PATH_DELIM);
            tokenFlag = 1;
            break;
         }
      }
      if(!tokenFlag)
      {
         fprintf(stderr, "could not find directory\n");
         exit(EXIT_FAILURE);
      }
   }
   /*we now have the inode of the last file/directory in the pathname,
     go back to beginning of partition and return inode*/
   fseek(fImage, base, SEEK_SET);
   return nextNode;
}

void printPartitionTable(FILE * fImage)
{
   int i;
   int rew = ftell(fImage);
   struct part_entry * entry = malloc(sizeof(struct part_entry));
   printf("       ----Start----       ------End------\n");
   printf("  Boot head  sec  cyl Type head  sec  cyl      First    Size\n");

   for (i = 0 ; i < NUM_PARTITIONS; i++)
   {
      fread(entry, sizeof(struct part_entry), 1, fImage);
      printf("  0x%2x %4d %4d %3d  0x%2x %4d  %3d  %3d %10lu %7lu\n", 
         entry->bootind, (int)entry->start_head,(int)entry->start_sec,
         (int) entry->start_cyl, (int)entry->sysind, (int)entry->last_head, 
         (int)entry->last_sec, (int)entry->last_cyl, 
         (long unsigned) entry->lowsec, (long unsigned) entry->size);
   }
   printf("\n");
   fseek(fImage, rew, SEEK_SET);
}

/*assumes file stream is set to beginning of partition table*/
void validatePartTable(FILE * fImage)
{
   int rew = ftell(fImage);
   uint8_t check = 0;

   fseek(fImage, sizeof(struct part_entry) * 4, SEEK_CUR);
   fread(&check, sizeof(uint8_t), 1, fImage);
   if(check != BOOT_SECTOR_BYTE_510)
   {
      fprintf(stderr, "invalid partition table\n");
      exit(EXIT_FAILURE);
   }
   fread(&check, sizeof(uint8_t), 1, fImage);
   if(check != BOOT_SECTOR_BYTE_511)
   {
      fprintf(stderr, "invalid partition table\n");
      exit(EXIT_FAILURE);
   }
   /*set back to beginning of partition table*/
   fseek(fImage, rew, SEEK_SET);
}

void partIsMinix(struct part_entry * entry)
{
   if(entry->sysind != MINIX_PART_TYPE)
   {
      fprintf(stderr, "not Minix partition\n");
      exit(EXIT_FAILURE);
   }
}

/*sets fImage to start of specified partition
 *assume fImage is set at the biginning of the partition table*/
void seekPartition(FILE * fImage, int partition, int verbose)
{
   struct part_entry * entry = malloc(sizeof(struct part_entry));
   if(verbose)
      printPartitionTable(fImage);
   validatePartTable(fImage);

   /*get entry from table*/
   fseek(fImage, partition* sizeof(struct part_entry), SEEK_CUR);
   fread(entry, sizeof(struct part_entry), 1, fImage);
   partIsMinix(entry);

   /*skip to specified partition entry*/
   fseek(fImage, ((long unsigned)entry->lowsec) * SECTOR_SIZE, SEEK_SET);
}

/*goes to the start of the filesystem*/
void findFileSystem(FILE * fImage, int partition, int subpartition, 
   int verbose)
{
   int rew;
   fseek(fImage, 0, SEEK_SET);
   fseek(fImage, LOC_PARTITION_TABLE, SEEK_CUR);
   if(verbose)
      printf("Partition Table: \n");
   seekPartition(fImage, partition, verbose);

   if(subpartition >= 0)
   {
      rew = ftell(fImage);
      fseek(fImage, LOC_PARTITION_TABLE, SEEK_CUR);
      if(verbose)
         printf("Subpartition Table: \n");
      seekPartition(fImage, subpartition, verbose);
   }
}

void getInode(FILE * fImage, uint32_t nodeNum, 
   struct superblock* superBlock, struct inode * node)
{
   int base = ftell(fImage);

   fseek(fImage, superBlock->blocksize * 2 , SEEK_CUR);
   fseek(fImage, (superBlock->i_blocks * superBlock->blocksize) +
      (superBlock->z_blocks * superBlock->blocksize), SEEK_CUR);

   fseek(fImage, sizeof(struct inode) * (nodeNum -1) , SEEK_CUR);
   fread(node, sizeof(struct inode) , 1, fImage);

   /*rewind*/
   fseek(fImage, base, SEEK_SET);

}


struct superblock * getfsSuperblock(FILE * fImage, struct cmdLine * cmdline)
{
    struct superblock * superBlock = malloc(sizeof(struct superblock));
   int rew;


    /*fseek(fImage, 0, SEEK_SET);
    fseek(fImage, SECTOR_SIZE * 2, SEEK_CUR);*/
   if(cmdline->pFlag)
   {
      if(cmdline->sFlag)
      {
         findFileSystem(fImage, cmdline->pVal, cmdline->sVal, cmdline->vFlag);
      }
      else
      {
         findFileSystem(fImage, cmdline->pVal, -1, cmdline->vFlag);
      }
   }

   rew = ftell(fImage);

   fseek(fImage, SECTOR_SIZE * 2, SEEK_CUR);
    fread(superBlock, sizeof(struct superblock), 1, fImage);
   if(cmdline->vFlag)
      printSuperBlock(superBlock);

   fseek(fImage, rew, SEEK_SET);
   return superBlock;
}
