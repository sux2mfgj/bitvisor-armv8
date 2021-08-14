
CC = gcc

TARGET	:= test

CFLAGS:=  -Wall -lcgreen -I../include -DCGREEN

$(TARGET): ../core/fdt.o fdt_stub.o fdt_test.o
	$(CC) -o $@ $^ $(CFLAGS)

.o.c:
	$(CC) $(CFLAGS) -c $< -o $@

.PHONY: clean
clean:
	rm -rf $(TARGET) $(TESTS) *.o
