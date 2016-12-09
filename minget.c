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
   printf("usage: minget [-v] [ -p num [ -s num ] ] ");
   printf("imagefile srcpath [ dstpath ]\n");
   printf("Options:\n");
   printf("-p  part    --- select partition for filesystem (default: none)\n");
   printf("-s  sub     --- select subpartition");
   printf(" for filesystem (default: none)\n");  
   printf("-h  help    --- print usage information and exit\n");
   printf("-v  verbose --- increase verbosity level\n");

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

   /*mandatory src path*/
   if(lineArg >= argc) {
      printUsage();
      exit(EXIT_FAILURE);
   }
   cmdline->pathName = argv[lineArg];
   lineArg++;
   /*check if a dest path is specified*/
   if(lineArg < argc)
   {
      cmdline->destPath = argv[lineArg];
   }
   return 0;
}

char * getFileContents(FILE * fImage, struct superblock * superBlock, 
   struct inode * inode, char * path) 
{
   uint32_t zonesize = superBlock->blocksize << superBlock->log_zone_size;
   struct inode * srcPathInode = findDir(fImage, inode, superBlock, path);
   char * fileBuffer = (char *)calloc(inode->size, sizeof(char));

   int numZones = inode->size / zonesize;
   int bytesRead = 0;
   int i;
   int base = ftell(fImage);

   if (numZones > 7)
   {
      numZones = 7;
   }

   if (isReg(srcPathInode))
   {
      for(i = 0; i < numZones; i++)
      {
         if(srcPathInode->zone[i])
         {
            fseek(fImage, base, SEEK_SET);
            if(inode->size - bytesRead <= 0)
            {
               fprintf(stderr, "fuck\n");
               exit(EXIT_FAILURE);
            }
            if(inode->size - bytesRead < zonesize)
            {
               seekZone(fImage, superBlock, srcPathInode->zone[i]);
               fread(fileBuffer + bytesRead, inode->size - bytesRead, 
                  1, fImage);
               bytesRead += inode->size - bytesRead;
            }
            else
            {
               seekZone(fImage, superBlock, srcPathInode->zone[i]);
               fread(fileBuffer + bytesRead, zonesize, 1, fImage);
               bytesRead += zonesize;
            }
         }
      }
   } 
   else 
   {
      /* file is not a regular file */
      exit(EXIT_FAILURE);
   }
   fseek(fImage, base, SEEK_SET);
   return fileBuffer;
}

void writeFileContents(char * fileBuffer, FILE * destFile) {
   fwrite(fileBuffer, strlen(fileBuffer), 1, destFile);
}

int main(int argc, char ** argv) 
{
   FILE * fImage;
   FILE * destFile;
   struct superblock * superBlock;
   struct inode * rootInode = malloc(sizeof(struct inode));
   char * fileContents;
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

   /* set destination file */
   if (cmdline->destPath) 
   {
      destFile = fopen(cmdline->destPath, "w");
      if (!destFile)
      {
         perror("Error on opening file for writing");
         return EXIT_FAILURE;
      }
   } 
   else 
   {
      destFile = stdout;
   }

   fileContents = getFileContents(fImage, superBlock, rootInode, 
      cmdline->pathName);
   writeFileContents(fileContents, destFile);

   return 0;
}
