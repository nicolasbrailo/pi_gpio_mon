.PHONY: clean

# XCOMPILE=--sysroot /
XCOMPILE=\
  -target arm-linux-gnueabihf \
  -mcpu=arm1176jzf-s \
  --sysroot ~/src/xcomp-rpiz-env/mnt/

CFLAGS=\
  $(XCOMPILE)\
  -ggdb \
  -Wall -Werror -Wextra \
  -Wundef \
  -Wmissing-include-dirs \
  -Wpointer-arith \
  -Winit-self \
  -Wfloat-equal \
  -Wredundant-decls \
  -Wimplicit-fallthrough \
  -Wendif-labels \
  -Wstrict-aliasing=2 \
  -Woverflow \
  -Wformat=2


gpiomon: gpiomon.o
	clang $(CFLAGS) gpiomon.o -o gpiomon -v

gpiomon.o: main.c
	clang $(CFLAGS) main.c -c -o gpiomon.o


clean:
	rm -f gpiomon gpiomon.o

.PHONY: xcompile-start xcompile-end install_sysroot_deps

xcompile-start:
	./rpiz-xcompile/mount_rpy_root.sh ~/src/xcomp-rpiz-env

xcompile-end:
	./rpiz-xcompile/umount_rpy_root.sh ~/src/xcomp-rpiz-env

install_sysroot_deps:
	true

