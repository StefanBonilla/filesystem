CC       = gcc
CCFLAGS  = -std=c89 -pedantic -Wall -Werror
INCLUDES = fs.h inode.h minshared.h
SOURCES  = inode.c minshared.c

all: minls minget

minls: minls.c inode.c minshared.c $(INCLUDES)
	$(CC) $(CCFLAGS) -o minls $(SOURCES) minls.c

minget: minget.c inode.c minshared.c $(INCLUDES) 
	$(CC) $(CCFLAGS) -o minget $(SOURCES) minget.c

clean:
	rm -f minls minget
