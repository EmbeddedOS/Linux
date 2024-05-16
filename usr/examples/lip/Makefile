CC=gcc
OBJ=signals/signals.o signals/kill.o threads/joining.o
FLAGS=-pthread

all: $(OBJ)

%.o: %.c
	$(CC) $(FLAGS) $(INCLUDES) $< -o $@

clean:
	rm -f *.o $(OBJ)