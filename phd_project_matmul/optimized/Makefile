INCLUDE_DIR = -I/usr/include/libdrm -I/usr/include/omap
LIBRARIES = -lpthread -ldrm_omap
CFLAGS = -Wall -O2 #-mfpu=neon -ftree-vectorize
SOURCES = timer.c barrier.c util.c simpletest.c test.c 
OBJECTS=$(SOURCES:.c=.o)
EXECUTABLE=app

all : $(SOURCES) $(EXECUTABLE)

%.o : %.c
	$(CC) $(INCLUDE_DIR) $(CFLAGS) -c -o $@ $<

$(EXECUTABLE) : $(OBJECTS)
	$(CC) $(OBJECTS) -o $@ $(LIBRARIES)

clean :
	rm -f $(EXECUTABLE) $(OBJECTS)


