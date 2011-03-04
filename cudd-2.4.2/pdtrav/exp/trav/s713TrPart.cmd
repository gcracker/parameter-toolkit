#
# PdTRAV-1.2 package
# Revision: Version 1.2 June 01, 1999
# Authors: Gianpiero Cabodi and Stefano Quer
#
# This Script File Creates the Monolothic Transition Relation for
# circuit s713 and then partition it.
#
# Usage: run pdtrav, then on the pdtrav prompt type
# @s713TrPart.cmd
#
### Script starts here #################################################

  # Initialize Decision Diagram Manager

ddm_init

  # Read blif file and create BDD for deltas and lambdas with default
  # variable order

fsm_read --blif s713.blif
tr_init --build
tr_cluster --mono

  # Get Statistics on Transition Relation and Manager

stats tr.tr
stats tr

  # Partition the transition relation using one among different
  # possible paritition methods.

#part_make --verbosity devMax --method cofactor --threshold 1000 tr.tr
#part_make --verbosity devMax --method estimate --threshold 1000 tr.tr
part_make --verbosity devMax --method fast --threshold 1000 tr.tr

stats tr.tr
quit
