object_files := launch.o process.o intercept.o

CC = gcc
CFLAGS = -Wall -pedantic

overseer: $(object_files) overseer.c
	$(CC) $(CFLAGS) $^ -o overseer

# Define a pattern rule that compiles every .c file into a .o file
%.o : %.c
		$(CC) -c $(CFLAGS) $(CPPFLAGS) $< -o $@
	
clean:
	rm -fr overseer *.o
