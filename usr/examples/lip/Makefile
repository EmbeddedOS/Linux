CC=gcc
OBJ=signals.o kill.o threads.o
FLAGS=-pthread

all: $(OBJ)

%.o: %.c
	$(CC) $(FLAGS) $(INCLUDES) $< -o $@

clean:
	rm *.o