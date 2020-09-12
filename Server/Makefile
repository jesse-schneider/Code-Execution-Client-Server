OBJECTS = server.o funcs.o
all: $(OBJECTS)
	$(CC) $(OBJECTS) -o server
server.o: server.c
	$(CC) -c server.c
funcs.o: funcs.c
	$(CC) -c funcs.c