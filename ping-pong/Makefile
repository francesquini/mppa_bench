# set the target platform (hardware or simulator)
platforms := hardware

# flags
k1-lflags := -lmppaipc

# build master bin
io-bin := master
master-srcs := src/master.c src/interface_mppa.c src/common.c

# build slave bin
cluster-bin := slave
slave-srcs := src/slave.c src/interface_mppa.c src/common.c

# build image (master + slave)
# the first binary will be the one booted on the IO!
multibin-objs := master slave
multibin-name := ping-pong.img
mppa-bin := multibin

# must be at the end of the makefile!
include $(K1_TOOLCHAIN_DIR)/share/make/Makefile.mppaipc
