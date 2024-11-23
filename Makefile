XCOMPILE=--sysroot /
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

gpiomon: main.c
	clang $(CFLAGS) main.c -o gpiomon

