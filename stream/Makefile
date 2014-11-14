# set the target platform (hardware or simulator)
platforms := hardware

# flags
cflags := -O
k1-lflags := -lmppaipc

# build two master bins
# 1) master: master that spawns the stream benchmark to a cluster
# 2) master_single: master that runs stream benchmark locally (IO-node)
io-bin := master master_single
master-srcs := master.c 
master_single-srcs := stream_master.c

# build slave bin
cluster-bin := stream_slave
stream_slave-srcs := stream_slave.c
stream_slave-cflags := -fopenmp
stream_slave-lflags := -fopenmp -lgomp

# build image (master + slave)
# the first binary will be the one booted on the IO!
multibin_master_slave-objs := master stream_slave
multibin_master_slave-name := stream_master_slave.img

multibin_master_single-objs := master_single
multibin_master_single-name := stream_master_single.img

# create the following multibinaries
mppa-bin := multibin_master_slave multibin_master_single

# must be at the end of the makefile!
include $(K1_TOOLCHAIN_DIR)/share/make/Makefile.mppaipc
