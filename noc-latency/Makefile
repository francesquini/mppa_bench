
K1_TOOLCHAIN_DIR=/usr/local/k1tools

IMAGE=noc-cost
BIN_MASTER=master
BIN_SLAVE=slave
INTERFACE_MPPA=interface_mppa
DEBUG=-DDEBUG

$(IMAGE).mpk: master.c slave.c
	k1-gcc -Wall -mos=rtems $(DEBUG) $(BIN_MASTER).c $(INTERFACE_MPPA).c -o $(BIN_MASTER) -lmppaipc
	k1-gcc -Wall -mos=nodeos -DOMP $(DEBUG) $(BIN_SLAVE).c $(INTERFACE_MPPA).c -o $(BIN_SLAVE) -lmppaipc -fopenmp 
	rm -f $(IMAGE).mpk
	createImage.rb --clusters $(BIN_SLAVE) --boot=$(BIN_MASTER) -T $(IMAGE).mpk	
	rm -f $(BIN_MASTER) $(BIN_SLAVE)

clean:
	rm -f *~ $(BIN_MASTER) $(BIN_SLAVE) $(IMAGE).mpk
