init: init.c
	gcc init.c $(shell pkg-config --cflags lvm2app devmapper devmapper-event) -O3 -march=native -Wall -Wextra -Werror -pedantic -std=gnu99 -o init $(shell pkg-config --libs lvm2app devmapper devmapper-event) -Wl,-O3 -Wl,--as-needed -static
clean:
	rm -f init
