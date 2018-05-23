#
.SUFFIXES: .c .o .h

CC = gcc -g
CFLAGS = -std=c11 -Wall -pedantic 

TARGETS := em-dump em-idx em-evt em-decode mwpc-dump mwpc-evt mwpc-decode #em-stats 
OBJS := em5-parser.o gzopen.o mwpc-parser.o
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

.PRECIOUS: $(TARGETS) $(OBJS)  #do not delete on error
.PHONY: all clean


all: $(TARGETS)

em-dump: em-dump.o $(OBJS)
	$(CC) $(CFLAGS)  $^ -o $@

em-idx: em-idx.o $(OBJS)
	$(CC) $(CFLAGS)  $^ -o $@

em-evt: em-evt.o $(OBJS)
	$(CC) $(CFLAGS)  $^ -o $@

em-decode: em-decode.o $(OBJS)
	$(CC) $(CFLAGS)  $^ -o $@

mwpc-dump: mwpc-dump.o $(OBJS)
	$(CC) $(CFLAGS)  $^ -o $@

mwpc-evt: mwpc-evt.o $(OBJS)
	$(CC) $(CFLAGS)  $^ -o $@

mwpc-decode: mwpc-decode.o $(OBJS)
	$(CC) $(CFLAGS)  $^ -o $@

clean:
	-rm -f *.o
	-rm -f $(TARGETS)
