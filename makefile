CC = gcc
CCFLAGS = -Wall -std=c11 
LIBRARIES = -pthread
EXECUTABLES = MatMult

all: $(EXECUTABLES) 
	
.c:
	$(CC) $(CCFLAGS) $(INCLUDES) $(LIBRARIES)  -o $@ $@.c

# Command for creating an executable file
.o:
	$(CC) -o $@ $@.o $(LIBRARIES)

# Rule for generating .o file from .c file
%.o: %.c
	$(CC) $(CCFLAGS) $(INCLUDES) -c $^ 
 
	
clean: 
	rm -f $(EXECUTABLES)  *.o
