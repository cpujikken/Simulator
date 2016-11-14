TARGET= bsim
CC = gcc
SOURCES = base.c print.c execute.c main.c

$(TARGET): $(SOURCES)
	$(CC) -o $@ $^

clean:
	rm $(TARGET)
