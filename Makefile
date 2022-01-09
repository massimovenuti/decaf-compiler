prefixe=decaf

CC = gcc
CFLAGS = -g -Wall -Wextra -Werror
# exige 3 fichiers:
# - $(prefixe).y (fichier bison)
# - $(prefixe).lex (fichier flex)
# - main.c (programme principal)
# construit un exécutable nommé "main"

# note : le programme principal ne doit surtout pas s'appeler $(prefixe).c
# (make l'écraserait parce qu'il a une règle "%.c: %.y")

all: decaf

decaf: main.o $(prefixe).tab.o lex.yy.o quad.o table.o quad2mips.o
	$(CC) $(LDFLAGS) $^ -g -o $@ $(LDLIBS)

$(prefixe).tab.c: $(prefixe).y
	bison -t -d --debug --verbose $(prefixe).y

lex.yy.c: $(prefixe).l $(prefixe).tab.h
	flex $(prefixe).l

doc:
	doxygen doxygen.conf

clean:
	rm -f *.o $(prefixe).tab.c $(prefixe).tab.h lex.yy.c decaf \
		$(prefixe).output $(prefixe).dot $(prefixe).pdf
	rm -rf doc

