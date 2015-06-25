PROGRAM = rollercoaster
OBJS = demo.o curve.o material.o
CC = gcc
LFLAGS = -Wall -g
CFLAGS = -Wall -g -c
LDFLAGS = -lGL -lGLU -lglut

$(PROGRAM) : $(OBJS)
	$(CC) $(LFLAGS) $(OBJS) -o $(PROGRAM) $(LDFLAGS)

demo.o : demo.h demo.c curve.h
	$(CC) $(CFLAGS) demo.c

curve.o : curve.h curve.c demo.h
	$(CC) $(CFLAGS) curve.c

clean:
	rm -f *.o $(PROGRAM)
	
tar:
	tar cfv $(PROGRAM).tar *.c   *.h
