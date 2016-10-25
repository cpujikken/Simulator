bsim: assoc.h assoc.c opcode.h binarysim.c
	gcc -o bsim assoc.c binarysim.c
sim: assoc.h assoc.c compilorsim.c
	gcc -o sim assoc.c compilorsim.c
clean:
	rm sim bsim
