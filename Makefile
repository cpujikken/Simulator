TARGET= bsim
CC = gcc
SOURCES = base.c print.c execute.c main.c

$(TARGET): $(SOURCES)
	$(CC) -o $@ $^
test: $(SOURCES)
	gcc -g -O0 -o $@ $^
wrbin: bin_writer.c
	gcc -o $@ $^
rdbin: bin_reader.c
	gcc -o $@ $^



clean:
	rm $(TARGET)
