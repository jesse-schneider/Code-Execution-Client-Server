OBJECTS = client.o funcs.o
all: $(OBJECTS)
	$(CC) $(OBJECTS) -o client
client.o: client.c
	$(CC) -c client.c
funcs.o: funcs.c
	$(CC) -c funcs.c