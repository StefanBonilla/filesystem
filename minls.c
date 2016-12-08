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

int navZone(FILE * fImage, struct inode * inode,
   struct superblock * superBlock, int zoneNum, char * pathToken) 
{
   uint32_t zonesize = superBlock->blocksize << superBlock->log_zone_size;
   int entriesPerZone = zonesize / DIRENT_SIZE;
   int notFound = 0;
   int zoneDirIndex;

   for(zoneDirIndex = 0; zoneDirIndex < entriesPerZone; zoneDirIndex++)
   {
      if(numEntriesRead + 1 > numUsedZones)
      {
         notFound = 1;
         break;
      }
      numEntriesRead++;
      fread(currDirent, sizeof(struct currDirent), 1, fImage);
      if(!strcmp(currDirent->filename, pathToken) && 
         currDirent->inode)
      {
         /*we have a hit, get the inode*/
         getInode(fImage, currDirent->inode, superBlock, 
            nextNode);]


         pathToken = strtok(NULL, PATH_DELIM);
         numUsedZones = nextNode->size / zonesize;
         numEntriesRead = 0;
         currZone = 0;
         break;
      }
   }

   return notFound;
}

struct inode * findDir(FILE * fImage, struct inode * inode, 
   struct superblock * superBlock, char * path)
{
   struct dirent * currDirent = malloc(sizeof(struct dirent));
   uint32_t zonesize = superBlock->blocksize << superBlock->log_zone_size;
   int numUsedZones;
   int entriesPerZone = zonesize / DIRENT_SIZE;
   int currZone = 0;
   int numEntriesRead;
   int zoneDirIndex = 0;     /* number of entries looked at in directory*/
   int rew = ftell(fImage);
   int notFound = 0;

   struct inode * nextNode = malloc(sizeof(struct inode));
   char * pathToken = strtok(path, PATH_DELIM);

   while(pathToken)
   {
      numEntriesRead = 0;
      numUsedZones = inode->size / zonesize;

      /*iterate through each of the inode's zones*/
      for (currZone = 0; currZone < 7; currZone++)
      {
         /*iterate through the entries of the zones*/
         notFound = navZone(fImage, inode, superBlock, currZone, pathToken);
         if(notFound)
            break;

         fseek(fImage, inode->zone[currZone] * zonesize, SEEK_CUR);
      }
      fprintf(stderr, "could not find directory/file\n");
      exit(EXIT_FAILURE);
   }




}

void listDir(FILE * fImage, struct inode * inode, struct superblock * superBlock)
{
   struct dirent * dirent = malloc(sizeof(struct dirent));
   int zone0, i;
   int numEntries;
   uint32_t zonesize = superBlock->blocksize << superBlock->log_zone_size;

   zone0 = inode->zone[0];


   numEntries = inode->size / DIRENT_SIZE;
   printf("numEntries: %d\n", numEntries);
   fseek(fImage, superBlock->firstdata * zonesize, SEEK_CUR);

   for(i = 0; i < numEntries; i++)
   {
      fread(dirent, sizeof(struct dirent), 1, fImage);
      if(dirent->inode)
      {
         getInode(fImage, dirent->inode, superBlock, inode);
         printf(" node: %d,    name: %s  ", 
            dirent->inode, dirent->filename);
         printf(" size: %d\n", inode->size);
      }
   }


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
   int rewind = ftell(fImage);

   fseek(fImage, 0, SEEK_SET);
   fseek(fImage, superBlock->blocksize * 2 , SEEK_CUR);
   fseek(fImage, (superBlock->i_blocks * superBlock->blocksize) +
      (superBlock->z_blocks * superBlock->blocksize), SEEK_CUR);

   fseek(fImage, sizeof(struct inode) * (nodeNum -1) , SEEK_CUR);
   fread(node, sizeof(struct inode) , 1, fImage);

   /*rewind*/
   fseek(fImage, 0, SEEK_SET);
   fseek(fImage, rewind, SEEK_CUR);

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
   if(lineArg > argc - 1)
   {
      cmdline->pathName = argv[lineArg];
   }
   return 0;

}


int main(int argc, char ** argv)
{
   FILE * fImage;
	struct superblock * superBlock;
   struct inode * rootInode;
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
   listDir(fImage, rootInode, superBlock); /* ls root dir */

   /* Navigate to the dir specified by the path */
   if (cmdline->pathName) {
      pathInode = findDir(fImage, rootInode, superBlock, cmdline->pathName);
   }
   

	return 0;
}