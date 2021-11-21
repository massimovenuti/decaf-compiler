CC = gcc -g
CFLAGS = -Wall -Wextra -Werror

all: test_table

test_table: test_table.o ../table.o

test_table.o: test_table.c ../table.h
	$(CC) $(CFLAGS) -c $< -I../

clean:
	rm -f *.o ../*.o test_table
