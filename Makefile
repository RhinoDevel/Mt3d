# MT, 2016aug22
#
# Original source: http://mrbook.org/blog/tutorials/make/

CC=gcc
#CFLAGS=-c -O3 -std=gnu11 -Wall `pkg-config gtk+-3.0 --cflags` -std=gnu11 -DNDEBUG 
CFLAGS=-c -g -std=gnu11 -Wall `pkg-config gtk+-3.0 --cflags` -std=gnu11
LDFLAGS=-g -lm `pkg-config gtk+-3.0 --libs`
SOURCES=main.c Mt3dSingleton.c GuiSingleton_cairo.c Mt3d.c Map.c MapSample.c Sys.c Calc.c
OBJECTS=$(SOURCES:.c=.o)
EXECUTABLE=mt3d

all: $(SOURCES) $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS) 
	$(CC) $(LDFLAGS) $(OBJECTS) -o $@

.c.o:
	$(CC) $(CFLAGS) $< -o $@

clean:
	rm $(OBJECTS) $(EXECUTABLE)
