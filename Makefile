TARGET   = minls
CC       = gcc
CCFLAGS  = -std=c89 -pedantic -Wall -Werror
SOURCES  = $(wildcard *.c)
INCLUDES = $(wildcard *.h)
OBJECTS  = $(SOURCES:.c=.o)

all:$(TARGET)

$(TARGET):$(OBJECTS)
	$(CC) -o $(TARGET)  $(OBJECTS)

$(OBJECTS):$(SOURCES) 
	$(CC) -c $(CCFLAGS) $(SOURCES)

clean:
	rm -f $(TARGET) $(OBJECTS)
