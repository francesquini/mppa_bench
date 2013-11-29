
K1_TOOLCHAIN_DIR=/usr/local/k1tools

IMAGE_MASTER_SLAVE=stream_master_slave
IMAGE_MASTER=stream_master
BIN_MASTER=master
BIN_SLAVE=stream
DEBUG=-DDEBUG

all: $(IMAGE_MASTER_SLAVE).mpk $(IMAGE_MASTER).mpk

$(IMAGE_MASTER_SLAVE).mpk: master.c stream_slave.c
	k1-gcc -O -mos=rtems $(DEBUG) $(BIN_MASTER).c -o $(BIN_MASTER) -lmppaipc
	k1-gcc -O -mos=nodeos -DOMP $(DEBUG) $(BIN_SLAVE)_slave.c -o $(BIN_SLAVE) -lmppaipc -fopenmp 
	rm -f $(IMAGE_MASTER_SLAVE).mpk
	createImage.rb --clusters $(BIN_SLAVE) --boot=$(BIN_MASTER) -T $(IMAGE_MASTER_SLAVE).mpk	
	rm -f $(BIN_MASTER) $(BIN_SLAVE)

$(IMAGE_MASTER).mpk: stream_master.c
	k1-gcc -O -mos=rtems $(DEBUG) $(BIN_SLAVE)_master.c -o $(BIN_SLAVE) -lmppaipc
	rm -f $(IMAGE_MASTER).mpk
	createImage.rb --boot=$(BIN_SLAVE) -T $(IMAGE_MASTER).mpk	
	rm -f $(BIN_SLAVE)

clean:
	rm -f *~ $(BIN_MASTER) $(BIN_SLAVE) $(IMAGE_MASTER_SLAVE).mpk $(IMAGE_MASTER).mpk
