all: init encrypt decrypt

init: init.c crypt-init.c crypt-deinit.c decrypt-util.c
	gcc init.c -Ofast -march=native -Wall -W -Wextra -Werror -std=gnu99 -o init -lcryptsetup -ldevmapper -lgcrypt -lgpg-error -luuid -ludev -lrt -Wl,-O3 -Wl,--as-needed -static

encrypt: encrypt.c crypt-init.c crypt-deinit.c decrypt-util.c
	gcc encrypt.c -Ofast -march=native -Wall -W -Wextra -Werror -std=gnu99 -o encrypt -lgcrypt -Wl,-O3 -Wl,--as-needed

decrypt: decrypt.c crypt-init.c crypt-deinit.c
	gcc decrypt.c -Ofast -march=native -Wall -W -Wextra -Werror -std=gnu99 -o decrypt -lgcrypt -Wl,-O3 -Wl,--as-needed

clean:
	rm -f init encrypt decrypt
