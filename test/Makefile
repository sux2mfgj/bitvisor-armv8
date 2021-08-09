

TESTS	:= first_test.o
TARGET	:= test

CFLAGS	:= -lcgreen

$(TARGET): $(TESTS)
	$(CC) $(CFLAGS) -o $@ $^

.PHONY: clean
clean:
	rm $(TARGET) $(TESTS)
