#
# PdTRAV-1.2 package
# Revision: Version 1.2 June 01, 1999
# Authors: Gianpiero Cabodi and Stefano Quer
#
# Script file for computing the reachable states at level
# 10 levels of circuit s1423.
# It uses a variable order optimized for representing the
# transition relation in a monolithic form.
#
# Usage: run pdtrav, then on the pdtrav prompt type
# @s1423TrMon.cmd
# or
# source s1423TrMon.cmd
#
### Script starts here #################################################

  # Initialize Decision Diagram Manager

ddm_init -c 1100000

  # Read blif file and create BDD for deltas and lambdas function
  # using the variable order contained in s1423TrMon.ord

fsm_read --blif --ord s1423TrMon.ord s1423.blif
tr_init --build
tr_cluster --sort weight --threshold 2000
tr_cluster --mono

  # Get statistics on Transition Relation Manager

stats tr

  # Get statistics on Transition Relation

stats tr.tr

  # Initialize Traversal Manager

trav_init

  # Enable two managers during traversal

#ddm_dup trav.reached trav

  # To read in a different variable order for the second manager

#ord_read trav.reached s1423Trav.ord

  # To enable dynamic variable reordering for the two managers

#ddm_dynord --enable --method sift trav
#ddm_dynord --enable --method sift trav.reached

  # Traverse first 10 level of s1423

traverse --depth 10
 
  # Other solutions:
  # use an auxiliary Decision Diagram Manager inside (--auxMgr)
  # use partitioning (fast heuristic, 10000 node threshold)

#traverse --auxMgr --depth 10
#traverse --partMethod fast --partThreshold 30000 --depth 10
#traverse --auxMgr --partMethod fast --partThreshold 30000 --depth 10

  # Print statistic on the Decision Diagram Manager

stats ddm

  # Try to optimize with reordering the final reached size (twice)
  # and write final variable order on file

#ddm_dynord --force --method sift trav.reached
#ddm_dynord --force --method sift trav.reached
#ord_write s1423r10.ord trav.reached

  # End the program

quit
