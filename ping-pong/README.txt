This benchmark computes the latency betweem IO-node and clusters using portals.
The IO sends a message to a cluster and computes the time it takes to arrive (T1).
Then the cluster sends a message to the IO and computes the time it takes to arrive (T2)
The benchmark ouputs T1, T2 and (T1 + T2) / 2
This procedure is done for each IO/cluster pair.
