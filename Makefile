all: init encrypt

init: init.c
	gcc init.c -Ofast -march=native -Wall -W -Wextra -Werror -std=gnu99 -o init -lcryptsetup -ldevmapper -lgcrypt -lgpg-error -luuid -ludev -lrt -Wl,-O3 -Wl,--as-needed -static

encrypt: encrypt.c
	gcc encrypt.c -Ofast -march=native -Wall -W -Wextra -Werror -std=gnu99 -o encrypt -lgcrypt -Wl,-O3 -Wl,--as-needed

clean:
	rm -f init
