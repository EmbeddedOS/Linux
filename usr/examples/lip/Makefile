CC=gcc
OBJ=signals/signals.o \
	signals/kill.o \
	threads/joining.o \
	threads/detach.o \
	threads/detached_attr.o \
	thread-sync/mutex-static.o

FLAGS=-pthread

all: $(OBJ)

%.o: %.c
	$(CC) $(FLAGS) $(INCLUDES) $< -o $@

clean:
	rm -f *.o $(OBJ)