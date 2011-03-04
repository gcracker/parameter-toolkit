#
# PdTRAV-1.2 package
# Revision: Version 1.2 June 01, 1999
# Authors: Gianpiero Cabodi and Stefano Quer
#
# Script file for computing the reachable states of a FSM.
#
# Usage: run pdtrav, then on the pdtrav prompt type
# @trav.cmd <InputBlifFile>.blif
#
### Script starts here #################################################

  # Initialize Decision Diagram Manager

ddm_init

  # Enable Variable Reordering on main Decision Diagram Manager

ddm_dynord --enable --method sift --threshold 250000 ddm

  # Read blif file and create the FSM structure;
  # Use
  # fsm_read --ord <name>.ord --blif $1
  # to read-in an ordering file

fsm_read --blif $1

  # Init Transition Relation Manager and Set Verbosity Level

tr_init --build
set tr.verbosity appMax
set tr.clustTh 2500
tr_cluster --sort size

  # Get Statistics on Transition Relation and Manager

stats tr.tr
stats tr

  # Init Traversal Manager

trav_init

  # Traverse the circuit
  # Use
  # traverse -depth <n>
  # to traverse the circuit up to level n

traverse --verbosity appMax --logPeriod 1

  # Print final statistics

stats ddm

  # quit

quit
