
lib	:= libkconfig.o
CFLAGS := -Wall

conf: $(lib) conf.o
	$(CC) $^ -o $@

.PHONY: clean
clean:
	rm -rf *.o conf
