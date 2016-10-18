sim: assoc.h assoc.c compilorsim.c
	gcc -o sim assoc.c compilorsim.c
clean:
	rm sim
