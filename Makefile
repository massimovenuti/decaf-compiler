
prefixe=decaf

# exige 3 fichiers:
# - $(prefixe).y (fichier bison)
# - $(prefixe).lex (fichier flex)
# - main.c (programme principal)
# construit un exécutable nommé "main"

# note : le programme principal ne doit surtout pas s'appeler $(prefixe).c
# (make l'écraserait parce qu'il a une règle "%.c: %.y")

all: main

main: main.o $(prefixe).tab.o lex.yy.o table.o
	$(CC) $(LDFLAGS) $^ -o $@ $(LDLIBS)

$(prefixe).tab.c: $(prefixe).y
	bison -t -d --debug --verbose $(prefixe).y

lex.yy.c: $(prefixe).l $(prefixe).tab.h
	flex $(prefixe).l

test: main
	./main < test.txt

doc:
	bison --report=all --report-file=$(prefixe).output \
		--graph=$(prefixe).dot --output=/dev/null \
		$(prefixe).y
	dot -Tpdf < $(prefixe).dot > $(prefixe).pdf

clean:
	rm -f *.o $(prefixe).tab.c $(prefixe).tab.h lex.yy.c main \
		$(prefixe).output $(prefixe).dot $(prefixe).pdf