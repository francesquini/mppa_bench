# build master bin
cluster-bin := master
master-srcs := src/master.c src/interface_mppa.c src/common.c

# build slave bin
io-bin := slave
slave-srcs := src/slave.c src/interface_mppa.c src/common.c

# build image (master + slave)
# the first binary will be the one booted on the IO!
multibin-objs := master slave
multibin-name := noc-latency.img
mppa-bin := multibin

# flags
k1-lflags := -lmppaipc

# must be at the end of the makefile!
include $(K1_TOOLCHAIN_DIR)/share/make/Makefile.kalray
