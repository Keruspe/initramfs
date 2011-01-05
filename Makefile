init: init.c
	gcc init.c dm_stuff.c cmdline.c utils.c -O2 -march=native -Wall -Wextra -Werror -pedantic -std=gnu99 -o init -Wl,-O2 -Wl,--as-needed -static
clean:
	rm -f init
