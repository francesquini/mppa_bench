
K1_TOOLCHAIN_DIR=/usr/local/k1tools

IMAGE=noc-latency
BIN_MASTER=master
BIN_SLAVE=slave
INTERFACE_MPPA=interface_mppa
# DEBUG=-DDEBUG
DEBUG=

ifndef TYPE
$(warning WARNING: Communication type (TYPE variable) was not set, so TYPE=USE_PORTAL will be used by default. You can choose the communicaiton type by running either "TYPE=USE_PORTAL make" OR "TYPE=USE_CHANNEL make")
TYPE=USE_PORTAL
endif

$(IMAGE).mpk: src/master.c src/slave.c src/common.c
	mkdir -p bin/
	k1-gcc -Wall -D$(TYPE) -mos=rtems $(DEBUG) src/$(BIN_MASTER).c src/$(INTERFACE_MPPA).c src/common.c -o $(BIN_MASTER) -lmppaipc
	k1-gcc -Wall -D$(TYPE) -mos=nodeos $(DEBUG) src/$(BIN_SLAVE).c src/$(INTERFACE_MPPA).c src/common.c -o $(BIN_SLAVE) -lmppaipc -fopenmp 
	rm -f bin/$(IMAGE).mpk
	createImage.rb --clusters $(BIN_SLAVE) --boot=$(BIN_MASTER) -T bin/$(IMAGE).mpk	
	rm -f $(BIN_MASTER) $(BIN_SLAVE)

clean:
	rm -f *~ $(BIN_MASTER) $(BIN_SLAVE) bin/$(IMAGE).mpk
