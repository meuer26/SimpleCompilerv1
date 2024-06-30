default: build

debug: build debug

build:
	flex lex.l
	yacc -d -v parser.y
	clang -c parserstack.c
	clang -DYYDEBUG lex.yy.c y.tab.c parserstack.o -o compiler
	./compiler prog.c
	nasm -felf64 prog.s
	ld -static prog.o -o prog

debug:
	gnome-terminal -- bash -c "gdb ./prog -x gdb_script.txt"

clean:
	rm -f y.tab.c
	rm -f y.tab.h
	rm -f parserstack.o
	rm -f lex.yy.c
	rm -f y.output
	rm -f prog.s
	rm -f compiler
	rm -f prog
	rm -f prog.o