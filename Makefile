# MT, 2016aug22
#
# - Original source: http://mrbook.org/blog/tutorials/make/
#
# - Install: apt-get install libgtk-3-dev

CC=gcc

# For GuiSingleton_cairo usage:
#
##CFLAGS=-c -O3 -std=gnu11 -Wall `pkg-config gtk+-3.0 --cflags` -DNDEBUG
#CFLAGS=-c -g -std=gnu11 -Wall `pkg-config gtk+-3.0 --cflags`
#LDFLAGS=-lm `pkg-config gtk+-3.0 --libs`
#SOURCES=main.c Mt3dSingleton.c GuiSingleton_cairo.c Loop.c LoopSingleton.c SinSingleton.c Mt3d.c Mt3dInput.c Map.c MapSample.c Sys.c Calc.c Bmp.c Dim.c

# For GuiSingleton_sdl usage:
#
#CFLAGS=-c -O3 -std=gnu11 -Wall `sdl2-config --cflags` -DNDEBUG
CFLAGS=-c -g -std=gnu11 -Wall `sdl2-config --cflags`
LDFLAGS=-lm `sdl2-config --libs`
SOURCES=main.c Mt3dSingleton.c GuiSingleton_sdl.c Loop.c LoopSingleton.c SinSingleton.c Mt3d.c Mt3dInput.c Map.c MapSample.c Sys.c Calc.c Bmp.c Dim.c

OBJECTS=$(SOURCES:.c=.o)
EXECUTABLE=mt3d

all: $(SOURCES) $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS)
	$(CC) $(LDFLAGS) $(OBJECTS) -o $@

.c.o:
	$(CC) $(CFLAGS) $< -o $@

clean:
	rm $(OBJECTS) $(EXECUTABLE)
