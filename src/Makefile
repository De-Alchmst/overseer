object_files := process.o intercept.o

CC = gcc
CFLAGS = -Wall -pedantic -std=c99

overseer: $(object_files)
	$(CC) $(CFLAGS) $^ -o overseer overseer.c

# Define a pattern rule that compiles every .c file into a .o file
%.o : %.c
		$(CC) -c $(CFLAGS) $(CPPFLAGS) $< -o $@
	
clean:
	rm -fr overseer *.o
