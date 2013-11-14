
K1_TOOLCHAIN_DIR=/usr/local/k1tools

IMAGE=noc-latency
BIN_MASTER=master
BIN_SLAVE=slave
INTERFACE_MPPA=interface_mppa
DEBUG=-DDEBUG

$(IMAGE).mpk: src/master.c src/slave.c
	k1-gcc -Wall -mos=rtems $(DEBUG) src/$(BIN_MASTER).c src/$(INTERFACE_MPPA).c -o $(BIN_MASTER) -lmppaipc
	k1-gcc -Wall -mos=nodeos -DOMP $(DEBUG) src/$(BIN_SLAVE).c src/$(INTERFACE_MPPA).c -o $(BIN_SLAVE) -lmppaipc -fopenmp 
	rm -f bin/$(IMAGE).mpk
	createImage.rb --clusters $(BIN_SLAVE) --boot=$(BIN_MASTER) -T bin/$(IMAGE).mpk	
	rm -f $(BIN_MASTER) $(BIN_SLAVE)

clean:
	rm -f *~ $(BIN_MASTER) $(BIN_SLAVE) bin/$(IMAGE).mpk
