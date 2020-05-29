all: iCompiler

OBJECTS = lexer.o interpret.o generator.o ifunc.o

COMPILERFLAGS = -c -DDEBUG

CC = cc
CCFLAGS = $(COMPILERFLAGS)
LIBRARIES = -lm



lexer.o: lexer.c lexer.h
	$(CC) $(CCFLAGS) -o lexer.o lexer.c

generator.o: generator.c generator.h
	$(CC) $(CCFLAGS) -o generator.o generator.c

interpret.o: interpret.c
	$(CC) $(CCFLAGS) -o interpret.o interpret.c 

ifunc.o: ifunc.c ifunc.h
	$(CC) $(CCFLAGS) -o ifunc.o ifunc.c 

iCompiler: $(OBJECTS)
	$(CC) -o iCompiler $(OBJECTS) $(LIBDIR) $(LIBRARIES)

clean:; rm -f *.o
run:; iCompiler $(args)
