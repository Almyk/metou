CC=g++
CFLAGS=-I. -lncurses
DEPS = client.h
OBJ = client.o

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

client: $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS)
	rm *.o

clean:
	rm client *.o
