PROGRAM = bsim
CC = gcc
CFLAGS = 
rdwrbin = wrbin rdbin
SOURCES = define.c base.c print.c label.c execute.c main.c

$(PROGRAM): $(SOURCES)
	$(CC) $(CFLAGS) -o $@ $(SOURCES)
test: $(SOURCES)
	gcc -g -O0 -o $@ $^
wrbin: bin_writer.c
	gcc -o $@ $^
rdbin: bin_reader.c
	gcc -o $@ $^
clean:
	$(RM) $(PROGRAM)
	find -name "*_out" | xargs $(RM)
clean_bin:
	$(RM) $(rdwrbin)
move_test:
	mv ../assembler/example ./
	mv ../assembler/example_label ./ 
