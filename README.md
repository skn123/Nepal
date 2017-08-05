# Nepal
**N**eural network **E**nhanced **P**rofile **A**lignment **L**ibrary.

## Abstract
Nepal is a profile comparison aligner with optimized scoring function for remote sequence alignment.

## Usage
This package include two programs, nepal and pssmcompiler.

### Nepal
Nepal accepts a pairwise PSSM text file, which is generated by pssmcompiler described below, as an input and outputs an alignment in fasta format.

$ nepal --input [PSSM text file]

### Pssmcompiler
The python program, pssmcompiler accepts a pairwise sequence fasta file and outputs Nepal readable profile file. You need to set the correct directory of a binary of deltablast, cdd_delta and a pseudo dataset in advance of use of the program.

$ pssmcompiler -i [pairwise sequence fasta file]

## Reference
Yamada KD, Improving quality of pairwise profile alignment by optimizing scoring function of dynamic programming using derivative free neural network, 2017
