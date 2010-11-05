init: init.c
	gcc init.c -O3 -march=native -Wall -Wextra -Werror -pedantic -std=gnu99 -o init -Wl,-O3 -Wl,--as-needed -static
clean:
	rm -f init
