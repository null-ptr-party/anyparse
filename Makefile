CC = gcc
CFLAGS  ?=  -W -Wall -Wextra -pedantic -fPIC -shared -o

SOURCES = parser.c \
reader.c \
converters.c \
msgcfg.c

OUTNAME = anyparse.dll
	
all: build

build:
	$(CC) $(SOURCES) $(CFLAGS) $(OUTNAME)	# compile all object files 

clean:
	echo "Removing .bin file..."
	rm -f *.dll