all: init 

init: init.c cmdline.c
	gcc init.c cmdline.c -Ofast -march=native -Wall -W -Wextra -Werror -std=gnu99 -o init -lgpgme -lgpg-error -lassuan -Wl,-O3 -Wl,--as-needed -static

clean:
	rm -f init
