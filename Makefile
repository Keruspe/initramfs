init: init.c
	gcc init.c -O2 -march=native -Wall -Wextra -Werror -pedantic -std=gnu99 -o init -Wl,-O2 -Wl,--as-needed -static
clean:
	rm -f init
