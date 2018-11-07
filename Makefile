CC=gcc
CFLAGS=-I.
DEPS = client2.h
OBJ = client2.o

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

client: $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS)
	rm *.o

clean:
	rm client *.o
