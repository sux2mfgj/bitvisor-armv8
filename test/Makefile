
CC = gcc

TARGET	:= test

CFLAGS:=  -lcgreen -I../include -DCGREEN

$(TARGET): ../core/fdt.o fdt_test.o fdt_stub.o
	$(CC) -o $@ $^ $(CFLAGS)

.o.c:
	$(CC) $(CFLAGS) -c $< -o $@

.PHONY: clean
clean:
	rm -rf $(TARGET) $(TESTS) *.o
