CC := arm-none-linux-gnueabi-gcc
LD := arm-none-linux-gnueabi-gcc
CFLAGS := -g -Wall -Werror -static
LDFLAGS := -static

OBJECTS := prinfo.o stress.o

all: prinfo stress


: $(OBJECTS)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $(OBJECTS)

%.o: %.c
	$(CC) $(CFLAGS) -c $^

clean:
	rm -f prinfo
	rm -f prinfo.o
	rm -f stress
	rm -f stress.o

.PHONY: clean
