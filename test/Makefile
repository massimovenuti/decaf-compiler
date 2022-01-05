CC = gcc
CFLAGS = -Wall -Wextra -Werror
LDFLAGS = -fprofile-arcs -ftest-coverage
all: test_quad test_table

test_quad: test_quad.o quad.o table.o

quad.o: ../quad.c
	$(CC) $(CFLAGS) -fprofile-arcs -ftest-coverage -c $< -I../

test_quad.o: test_quad.c ../quad.h
	$(CC) $(CFLAGS) -c $< -I../

test_table: test_table.o table.o

table.o: ../table.c
	$(CC) $(CFLAGS) -fprofile-arcs -ftest-coverage -c $< -I../

test_table.o: test_table.c ../table.h
	$(CC) $(CFLAGS) -c $< -I../

clean:
	rm -f *.o *.gcda *.gcno test_quad test_table
