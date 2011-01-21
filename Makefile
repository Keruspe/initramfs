all: lvm

lvm: init initramfs-lvm

mdadm: init initramfs-mdadm

both: init initramfs

init: init.c
	gcc init.c dm_stuff.c cmdline.c utils.c run_bg.c -O2 -march=native -Wall -Wextra -Werror -pedantic -std=gnu99 -o init -Wl,-O2 -Wl,--as-needed -static

initramfs-lvm:
	sed '/mdadm/d' initramfs.in > initramfs

initramfs-mdadm:
	sed '/lvm/d' initramfs.in > initramfs

initramfs:
	cp initramfs.in initramfs

clean:
	rm -f init initramfs
