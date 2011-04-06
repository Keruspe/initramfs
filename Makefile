all: lvm

lvm: init initramfs-lvm

mdadm: init initramfs-mdadm

both: init initramfs

init: init.c dm_stuff.c cmdline.c run_bg.c
	gcc init.c dm_stuff.c cmdline.c run_bg.c -O3 -march=native -Wall -Wextra -Werror -pedantic -std=gnu99 -o init -Wl,-O2 -Wl,--as-needed -static

initramfs-lvm: initramfs.in
	sed '/mdadm/d' initramfs.in > initramfs

initramfs-mdadm: initramfs.in
	sed '/lvm/d' initramfs.in > initramfs

initramfs: initramfs.in
	cp initramfs.in initramfs

clean:
	rm -f init initramfs
