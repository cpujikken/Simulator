PROGRAM = bsim
CC = gcc
CFLAGS = 
rdwrbin = wrbin rdbin
SOURCES = define.c base.c print.c execute.c main.c

$(PROGRAM): $(SOURCES)
	$(CC) $(CFLAGS) -o $@ $(SOURCES)
test: $(SOURCES)
	gcc -g -O0 -o $@ $^
wrbin: bin_writer.c
	gcc -o $@ $^
rdbin: bin_reader.c
	gcc -o $@ $^



clean:
	$(RM) $(PROGRAM) $(rdwrbin)
	find -name "*_out" | xargs $(RM)
