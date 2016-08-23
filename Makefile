# MT, 2016aug22
#
# Original source: http://mrbook.org/blog/tutorials/make/

CC=gcc
#CFLAGS=-c -O3 -std=gnu11 -Wall -DNDEBUG
CFLAGS=-c -g -std=gnu11 -Wall
#LDFLAGS=
LDFLAGS=-g -lm
SOURCES=main.c Mt3d.c Map.c MapSample.c Bmp.c Sys.c
OBJECTS=$(SOURCES:.c=.o)
EXECUTABLE=mt3d

all: $(SOURCES) $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS) 
	$(CC) $(LDFLAGS) $(OBJECTS) -o $@

.c.o:
	$(CC) $(CFLAGS) $< -o $@

clean:
	rm $(OBJECTS) $(EXECUTABLE)
