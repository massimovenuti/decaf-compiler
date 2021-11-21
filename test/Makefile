CC = gcc -g
CFLAGS = -Wall -Wextra -Werror

all: test_quad test_table

test_quad: test_quad.o ../quad.o

test_quad.o: test_quad.c ../quad.h
	cc -c $< -I../

test_table: test_table.o ../table.o

test_table.o: test_table.c ../table.h
	$(CC) $(CFLAGS) -c $< -I../

clean:
	rm -f *.o ../*.o test_quad test_table
