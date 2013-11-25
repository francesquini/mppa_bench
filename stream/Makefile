
K1_TOOLCHAIN_DIR=/usr/local/k1tools

IMAGE=stream
BIN_MASTER=master
BIN_SLAVE=stream
DEBUG=-DDEBUG

$(IMAGE).mpk: master.c stream.c
	k1-gcc -O -mos=rtems $(DEBUG) $(BIN_MASTER).c -o $(BIN_MASTER) -lmppaipc
	k1-gcc -O -mos=nodeos -DOMP $(DEBUG) $(BIN_SLAVE).c -o $(BIN_SLAVE) -lmppaipc -fopenmp 
	rm -f $(IMAGE).mpk
	createImage.rb --clusters $(BIN_SLAVE) --boot=$(BIN_MASTER) -T $(IMAGE).mpk	
	rm -f $(BIN_MASTER) $(BIN_SLAVE)

clean:
	rm -f *~ $(BIN_MASTER) $(BIN_SLAVE) $(IMAGE).mpk
