all: test_quad

test_quad: test_quad.o ../quad.o

test_quad.o: test_quad.c ../quad.h
	cc -c $< -I../

clean:
	rm -f *.o ../*.o test_quad