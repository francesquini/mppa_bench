This benchmark computes the latency betweem IO-node and clusters using portals OR channels.

It takes the number of clusters to be used as the input parameter.
For instance, to run with 2 clusters type:

$ ./run.sh 2

The user must set USE_PORTAL=1 to use portals or USE_CHANNEL=1 to use channels.
This can be done in the common.h header file.

First, the IO-node sends messages of different sizes to all clusters using portals (asynchronous) or channels (synchronous).
Then, all clusters send messages of different sizes to the IO-node using portals (asynchronous) or channels (synchronous).

The result is written in the output directory (output/data.csv).
