CXX=gcc -std=c11
CXX_WARNINGS=-Wall -Wextra -Wpedantic


lab3a: lab3a.c
	$(CXX) lab3a.c -o lab3a $(CXX_WARNINGS)

test: lab3a.c
	make lab3a
	./lab3a trivial.img

clean:
	-rm lab3a

