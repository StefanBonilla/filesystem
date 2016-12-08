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
#include "inode.h"
#include "minshared.h"

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
