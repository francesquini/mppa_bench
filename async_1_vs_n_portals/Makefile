
K1_TOOLCHAIN_DIR=/usr/local/k1tools

IMAGE=async_1_vs_n_portals
BIN_MASTER=master
BIN_SLAVE=slave
INTERFACE_MPPA=interface_mppa
DEBUG=-DDEBUG
# DEBUG=


$(IMAGE).mpk: src/master.c src/slave.c src/common.c
	mkdir -p bin/
	k1-gcc -Wall -mos=rtems $(DEBUG) src/$(BIN_MASTER).c src/$(INTERFACE_MPPA).c src/common.c -o $(BIN_MASTER) -lmppaipc
	k1-gcc -Wall -mos=nodeos $(DEBUG) src/$(BIN_SLAVE).c src/$(INTERFACE_MPPA).c src/common.c -o $(BIN_SLAVE) -lmppaipc -fopenmp 
	rm -f bin/$(IMAGE).mpk
	createImage.rb --clusters $(BIN_SLAVE) --boot=$(BIN_MASTER) -T bin/$(IMAGE).mpk	
	rm -f $(BIN_MASTER) $(BIN_SLAVE)

clean:
	rm -f *~ $(BIN_MASTER) $(BIN_SLAVE) bin/$(IMAGE).mpk
