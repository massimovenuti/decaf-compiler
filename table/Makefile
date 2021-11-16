CC = gcc -g
CFLAGS = -Wall

SRC = $(wildcard *.c)
OBJ = $(patsubst %.c, %.o, $(SRC))

main: $(OBJ)
	$(CC) $(CFLAGS) -o $@ $(OBJ)

%.o:%.c %.h
	$(CC) $(CFLAGS) -c $<	

clean:
	@echo "Suppresion des .o et de l'éxécutable"
	rm *.o
	rm main
