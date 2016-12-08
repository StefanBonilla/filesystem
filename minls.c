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

#include "fs.h"

void getInode(FILE * fImage, uint32_t nodeNum, 
   struct superblock* superBlock, struct inode * node);
void listDir(FILE * fImage, struct inode * inode, 
   struct superblock * superBlock);

struct cmdLine
{
   uint32_t vFlag;
   uint32_t pFlag;
   uint32_t pVal;
   uint32_t sFlag;
   uint32_t sVal;
   char * imageFile;
   char * pathName;
};

void printUsage()
{
   printf("usage: minls [-v] [ -p num [ -s num ] ] imagefile [ path ]\n");
   printf("Options:\n");
   printf("-p  part    --- select partition for filesystem (default: none)\n");
   printf("-s  sub     --- select subpartition for filesystem (default: none)\n");  
   printf("-h  help    --- print usage information and exit\n");
   printf("-v  verbose --- increase verbosity level\n");

}

void printSuperBlock(struct superblock * superBlock)
{
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
   printf("  uint32_t double %15d\n", inode->two_indirect);
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
   printf("     NAV ZONE: totalEntriesRead: %d, numEntriesInDir : %d\n", 
      *totalEntriesRead, numEntriesInDir);
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
         printf("***********getInode: %d\n", currDirent->inode);
         fseek(fImage, base, SEEK_SET);
         getInode(fImage, currDirent->inode, superBlock, 
            nextNode);
         printf("NAV_ZONE INODE: \n");
         printiNode(nextNode);
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

   printf("NAVIGATING to inode of specified path\n");

   nextNode = inode;
   printf("**************nextNode: \n");
   printiNode(nextNode);

   while(pathToken)
   {
      printf("    ->locating directory: %s\n", pathToken);
      tokenFlag = 0;
      /*listDir(fImage, nextNode, superBlock);*/
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
            printf("    ENTRY_FOUND\n");
            pathToken = strtok(NULL, PATH_DELIM);
            printiNode(nextNode);
            printf("next token: %s", pathToken);
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

void listDir(FILE * fImage, struct inode * basenode, 
   struct superblock * superBlock)
{
   struct dirent * dirent = malloc(sizeof(struct dirent));
   int zone0, i;
   int numEntries;
   uint32_t zonesize = superBlock->blocksize << superBlock->log_zone_size;
   int base = ftell(fImage);
   int zone = ftell(fImage);
   struct inode * tempNode = malloc(sizeof(struct inode));

   zone0 = basenode->zone[0];


   printf("LISTING directories:\n");
   printf(" *************inode->size: %d\n", basenode->size);
   numEntries = basenode->size / DIRENT_SIZE;

   printf("    numEntries: %d\n", numEntries);
   fseek(fImage, superBlock->firstdata * zonesize, SEEK_CUR);
   zone = ftell(fImage);

   for(i = 0; i < numEntries; i++)
   {
      fseek(fImage, zone, SEEK_SET);
      fread(dirent, sizeof(struct dirent), 1, fImage);
      zone = ftell(fImage);
      if(dirent->inode)
      {
         fseek(fImage, base, SEEK_SET);
         getInode(fImage, dirent->inode, superBlock, tempNode);
         printf("      node: %d,    name: %s  ", 
            dirent->inode, dirent->filename);
         printf(" size: %d\n", tempNode->size);
      }
   }
   fseek(fImage, base, SEEK_SET);

}

void printPartitionTable(FILE * fImage)
{
   int i;
   int rew = ftell(fImage);
   struct part_entry * entry = malloc(sizeof(struct part_entry));
   printf("Partition Table: \n");
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
   fseek(fImage, rew, SEEK_SET);
}

/*assumes file stream is set to beginning of partition table*/
void validatePartTable(FILE * fImage)
{
   int rew = ftell(fImage);
   uint8_t check = 0;

   printf("     VALIDATING table\n");
   fseek(fImage, sizeof(struct part_entry) * 4, SEEK_CUR);
   printf("      fImage addr: %lu\n", ftell(fImage));
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
   printf("     MINIX check: 0x%x\n", entry->sysind);
   if(entry->sysind != MINIX_PART_TYPE)
   {
      fprintf(stderr, "not Minix partition\n");
      exit(EXIT_FAILURE);
   }
}

/*sets fImage to start of specified partition
 *assume fImage is set at the biginning of the partition table*/
void seekPartition(FILE * fImage, int partition)
{
   struct part_entry * entry = malloc(sizeof(struct part_entry));
   printPartitionTable(fImage);
   validatePartTable(fImage);
   printf("   SEEKING partition %d\n", partition);

   /*get entry from table*/
   fseek(fImage, partition* sizeof(struct part_entry), SEEK_CUR);
   fread(entry, sizeof(struct part_entry), 1, fImage);
   partIsMinix(entry);
   printf("   entry->lowsec: %lu\n", (long unsigned) entry->lowsec);

   /*skip to specified partition entry*/
   fseek(fImage, ((long unsigned)entry->lowsec) * SECTOR_SIZE, SEEK_SET);
}

/*goes to the start of the filesystem*/
void findFileSystem(FILE * fImage, int partition, int subpartition)
{
   int rew;
   printf("FINDING file system\n");
   fseek(fImage, 0, SEEK_SET);
   fseek(fImage, LOC_PARTITION_TABLE, SEEK_CUR);
   seekPartition(fImage, partition);

   if(subpartition >= 0)
   {
      printf("   current fImage addr: %lu\n", ftell(fImage));
      rew = ftell(fImage);
      fseek(fImage, LOC_PARTITION_TABLE, SEEK_CUR);
      seekPartition(fImage, subpartition);
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

   printf("sizeof partition entry * 4: %d\n", sizeof(struct part_entry) * 4);

	/*fseek(fImage, 0, SEEK_SET);
	fseek(fImage, SECTOR_SIZE * 2, SEEK_CUR);*/
   if(cmdline->pFlag)
   {
      if(cmdline->sFlag)
      {
         findFileSystem(fImage, cmdline->pVal, cmdline->sVal);
      }
      else
      {
         findFileSystem(fImage, cmdline->pVal, -1);
      }
   }

   rew = ftell(fImage);

   fseek(fImage, SECTOR_SIZE * 2, SEEK_CUR);
	fread(superBlock, sizeof(struct superblock), 1, fImage);
	printSuperBlock(superBlock);

   fseek(fImage, rew, SEEK_SET);
   return superBlock;
}

void initCmdLine(struct cmdLine * cmdline)
{
   cmdline->vFlag = 0;
   cmdline->pFlag = 0;
   cmdline->pVal = 0;
   cmdline->sFlag = 0;
   cmdline->sVal = 0;
}

int parseCmdLine(int argc, char ** argv, struct cmdLine * cmdline)
{
   int opt;
   int lineArg = 1;

   printf("PARSING cmd line: \n");
   initCmdLine(cmdline);
   while((opt = getopt(argc, argv, "vp:s:")) != -1)
   {
      switch(opt)
      {
         case 'v':
            cmdline->vFlag = 1;
            lineArg++;
            printf("   vflag set\n");
            break;
         case 'p':
            cmdline->pFlag = 1;
            cmdline->pVal = atoi(optarg);
            /*TODO: handle case of no specified partition*/
            lineArg += 2;
            printf("   pFlag set: %d\n", cmdline->pVal);
            break;
         case 's':
            cmdline->sFlag = 1;
            /*TODO: handle case of no specified subpartition*/
            cmdline->sVal = atoi(optarg);
            lineArg += 2;
            printf("   sFlag set: %d\n", cmdline->sVal);
            break;
         default:
            printUsage();
            exit(EXIT_FAILURE);
      }

   }
   /*TODO: check if valid*/
   cmdline->imageFile = argv[lineArg];
   lineArg++;

   /*check if a path is specified*/
   if(lineArg >= argc - 1)
   {
      cmdline->pathName = argv[lineArg];
      printf("pathname specified: %s\n", cmdline->pathName);
   }
   return 0;

}


int main(int argc, char ** argv)
{
   FILE * fImage;
	struct superblock * superBlock;
   struct inode * rootInode = malloc(sizeof(struct inode));
   struct inode * pathInode;
   int res;
   struct cmdLine * cmdline = malloc(sizeof(struct cmdLine));


   res = parseCmdLine(argc, argv, cmdline);
   if(res)
   {
      printUsage();
   }

   fImage = fopen(cmdline->imageFile, "r");
   if(!fImage)
   {
      fprintf(stderr, "failed to open disk image\n");
      exit(EXIT_FAILURE);
   }

   superBlock = getfsSuperblock(fImage, cmdline);
   getInode(fImage, 1, superBlock, rootInode); /* get the root inode */
   printf("*******root: \n");
   printiNode(rootInode);

   listDir(fImage, rootInode, superBlock); /* ls root dir */

   /* Navigate to the dir specified by the path */
   if (cmdline->pathName) 
   {
      printf("******SEARCHING for path: %s\n", cmdline->pathName);
      printf("ROOT INODE: \n");
      printiNode(rootInode);
      pathInode = findDir(fImage, rootInode, superBlock, cmdline->pathName);
      printf("PATH INODE: \n");
      printiNode(pathInode);
      /*listDir(fImage, pathInode, superBlock);*/
   }
   
	return 0;
}