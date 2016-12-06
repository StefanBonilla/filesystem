#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <inttypes.h>


#define SECTOR_SIZE 512

struct superblock { /* Minix Version 3 Superblock
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


int main(int argc, char ** argv)
{
	char * fsName = calloc(sizeof(char), 100);
	struct superblock * superBlock = malloc(sizeof(struct superblock));
	FILE * fImage; 

	fsName = argv[1];

	fImage = fopen(fsName, "r");
	if(!fImage)
	{
		fprintf(stderr, "fukkckkkck\n");
		return 0;
	}
	fseek(fImage, 0, SEEK_SET);
	fseek(fImage, SECTOR_SIZE * 2, SEEK_CUR);

	fread(superBlock, sizeof(struct superblock), 1, fImage);

	printf("superblock: num nodes %d\n", superBlock->ninodes);
	return 0;


}