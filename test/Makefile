
CC = gcc

TARGET	:= test

CFLAGS:=  -lcgreen -I../include

$(TARGET): ../core/fdt.o fdt_test.o fdt_stub.o
	$(CC) -o $@ $^ $(CFLAGS)

fdt_stub.o: fdt_stub.c
	$(CC) -c $< -o $@ -I ../include

.PHONY: clean
clean:
	rm -rf $(TARGET) $(TESTS) *.o
