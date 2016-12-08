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

void printUsage()
{
   printf("usage: minls [-v] [ -p num [ -s num ] ] imagefile [ path ]\n");
   printf("Options:\n");
   printf("-p  part    --- select partition for filesystem (default: none)\n");
   printf("-s  sub     --- select subpartition");
   printf(" for filesystem (default: none)\n");  
   printf("-h  help    --- print usage information and exit\n");
   printf("-v  verbose --- increase verbosity level\n");

}

void printEntry(struct inode * inode, char * pathname)
{
   char * perms = getPermissionString(inode);
   printf("%s %9d %s\n", perms, inode->size, pathname);
}

void listDir(FILE * fImage, struct inode * basenode, 
   struct superblock * superBlock)
{
   struct dirent * dirent = malloc(sizeof(struct dirent));
   struct inode * tempNode = malloc(sizeof(struct inode));
   int  i;
   int numEntries;
   uint32_t zonesize = superBlock->blocksize << superBlock->log_zone_size;
   int base = ftell(fImage);
   int zone;


   numEntries = basenode->size / DIRENT_SIZE;

   fseek(fImage, basenode->zone[0] * zonesize, SEEK_CUR);

   /*set zone to the start of the partition data zones*/
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
         printEntry(tempNode, (char *) dirent->filename);
         /*printf("      node: %d,    name: %s  ", 
            dirent->inode, dirent->filename);
         printf(" size: %d\n", tempNode->size);*/
      }
   }
   fseek(fImage, base, SEEK_SET);
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

   initCmdLine(cmdline);
   while((opt = getopt(argc, argv, "vp:s:")) != -1)
   {
      switch(opt)
      {
         case 'v':
            cmdline->vFlag = 1;
            lineArg++;
            break;
         case 'p':
            cmdline->pFlag = 1;
            cmdline->pVal = atoi(optarg);
            /*TODO: handle case of no specified partition*/
            lineArg += 2;
            break;
         case 's':
            cmdline->sFlag = 1;
            /*TODO: handle case of no specified subpartition*/
            cmdline->sVal = atoi(optarg);
            lineArg += 2;
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
   char * pathName = calloc(sizeof(char) ,  1000);
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
   /*printf("*******root: \n");
   printiNode(rootInode);*/

   /*listDir(fImage, rootInode, superBlock); *//* ls root dir */

   /* Navigate to the dir specified by the path */
   if (cmdline->pathName) 
   {
      strcpy(pathName, cmdline->pathName);

      /*printf("******SEARCHING for path: %s\n", cmdline->pathName);
      printf("ROOT INODE: \n");
      printiNode(rootInode);*/
      pathInode = findDir(fImage, rootInode, superBlock, cmdline->pathName);
      if(cmdline->vFlag)
         printiNode(pathInode);
      /*printf("PATH INODE: \n");
      printiNode(pathInode);*/

      if(isDir(pathInode))
      {
         printf("%s:\n", pathName);
         listDir(fImage, pathInode, superBlock);
      }
      else
         printEntry(pathInode, pathName);

   }
   else
   {
      printf("/:\n");
      listDir(fImage, rootInode, superBlock);
   }
   
	return 0;
}
