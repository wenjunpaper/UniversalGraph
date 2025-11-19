## HeavyGraph
### Introduction
In this paper, we in-
troduce UniversalGraph, a novel universal sketch tailored for graph
streams. The core ideas of UniversalGraph are threefold: (1) It for-
mulates the first node-level universal query task for graph streams,
enabling diverse frequency-based statistics on hot nodes; (2) Hot
nodes and edges are maintained in a node-centric structure for
lossless counting, with a descending-order replacement policy that
retains hot items whenever possible to reduce estimation error; and
(3) Other items are estimated through a simple multi-layer sampling
process for fidelity, where each layer uses a specific compressed
matrix to further improve accuracy and throughput.

### Datasets
Our code runs on the following dataset:

-[Caida](./Data/caida.zip)

-[Campus](./Data/campus.zip)

Please use zip to extract.
### How to Run
Suppose you have already navigated into the project directory.
```sh
cmake ./
# run gsum estimation:
make gsum
./gsum
#run heavy hitter detection:
make heavy
./heavy
```