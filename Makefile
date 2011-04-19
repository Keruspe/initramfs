all: lvm

lvm: init initramfs-lvm

mdadm: init initramfs-mdadm

both: init initramfs

init: init.c cmdline.c
	klcc init.c cmdline.c -Ofast -march=native -Wall -Wextra -Werror -std=gnu99 -o init -Wl,-Ofast -Wl,--as-needed -static

initramfs-lvm: initramfs.in
	sed '/mdadm/d' initramfs.in > initramfs

initramfs-mdadm: initramfs.in
	sed '/lvm/d' initramfs.in > initramfs

initramfs: initramfs.in
	cp initramfs.in initramfs

clean:
	rm -f init initramfs
