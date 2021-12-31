CC=gcc
CFLAGS=-g -Wall
TARGET=20171669.out
OBJS=20171669.o op_code.o assembler.o

$(TARGET): $(OBJS)
	$(CC) -o $@ $(OBJS) -lm

20171669.o : 20171669.c
	$(CC) -c -o 20171669.o 20171669.c

op_code.o : op_code.c
	$(CC) -c -o op_code.o op_code.c

assembler.o : assembler.c
	$(CC) -c -o assembler.o assembler.c

clean:
	rm -f *.o
	rm -f $(TARGET)
