# set the target platform (hardware or simulator)
platforms := hardware

# flags
k1-lflags := -lmppaipc

# build master bin
io-bin := master
master-srcs := src/main_mppa.c src/interface_mppa.c

# build image (master only)
multibin-objs := master
multibin-name := memcpy.img
mppa-bin := multibin

# must be at the end of the makefile!
include $(K1_TOOLCHAIN_DIR)/share/make/Makefile.mppaipc
