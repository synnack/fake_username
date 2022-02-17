CFLAGS=-Wall -Wextra -std=gnu11

all: show-getpwuid libfakeuser.so

libfakeuser.so: fakeuser.c
	$(CC)  $(CFLAGS) -shared -fPIC -o libfakeuser.so fakeuser.c -ldl $(LDFLAGS)

show-getpwuid: show-getpwuid.c

clean:
	rm -f show-getpwuid libfakeuser.so
