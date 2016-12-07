#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <inttypes.h>
#include <unistd.h>
#include <ctype.h>
#include <getopt.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "fs.h"

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
   printf("File inode: \n");
   printf("  uint16_t mode %15x\n", inode->mode);
   printf("  uint16_t links %15d\n", inode->links);
   printf("  uint16_t uid %15d\n", inode->uid);
   printf("  uint16_t gid %15d\n", inode->gid);
   printf("  uint16_t size %15d\n", inode->size);
   printf("  zone[0] = %15d\n", inode->zone[0]);
}

struct superblock * getfsSuperblock(struct cmdLine * cmdline )
{

	struct superblock * superBlock = malloc(sizeof(struct superblock));
	FILE * fImage;
   struct inode * inode = malloc(sizeof(struct inode));
	fImage = fopen(cmdline->imageFile, "r");
	if(!fImage)
	{
		fprintf(stderr, "failed to open disk image\n");
		exit(EXIT_FAILURE);
	}
	fseek(fImage, 0, SEEK_SET);
	fseek(fImage, SECTOR_SIZE * 2, SEEK_CUR);

	fread(superBlock, sizeof(struct superblock), 1, fImage);
	printSuperBlock(superBlock);

   /*seek the inode of the root directory*/
   printf("SECTOR_SIZE = %d\n", SECTOR_SIZE);
   printf("blocksize = %d\n", superBlock->blocksize);
   fseek(fImage, superBlock->blocksize * 2 , SEEK_SET);
   fseek(fImage, (superBlock->i_blocks * superBlock->blocksize) +
      (superBlock->z_blocks * superBlock->blocksize), SEEK_CUR);
   /*fseek(fImage, sizeof(struct inode), SEEK_CUR);*/
   fread(inode, sizeof(struct inode), 1, fImage);
   printiNode(inode);


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
	struct superblock * superBlock;
   int res;
   struct cmdLine * cmdline = malloc(sizeof(struct cmdLine));


   res = parseCmdLine(argc, argv, cmdline);
   if(res)
   {
      printUsage();
   }

   superBlock = getfsSuperblock(cmdline);

	return 0;


}