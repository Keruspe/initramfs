all: init 

init: init.c
	gcc init.c -Ofast -march=native -Wall -W -Wextra -Werror -std=gnu99 -o init -lgpgme -lgpg-error -lassuan -lcryptsetup -ldevmapper -lgcrypt -luuid -ludev -lrt -Wl,-O3 -Wl,--as-needed -static

clean:
	rm -f init
