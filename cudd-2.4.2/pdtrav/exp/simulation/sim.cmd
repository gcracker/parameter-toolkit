#
# PdTRAV-2.0 package
# Revision: Version 2.0Beta April 01, 2001
# Authors: Gianpiero Cabodi and Stefano Quer
#
# Script file for simulation
#
# Usage: run pdtrav, then on the pdtrav prompt type
# @sim.cmd
# or
# source sim.cmd
#
### Script starts here #################################################

ddm_init
fsm_read --blif s27.blif

  #
  # Simulate:
  # S0: random; Input: random; Step: 10; Output: "Kiss" format on stdout
  #

simulate --depth 10 --simFlag 0 --depth -1 --result stdout

  #
  # Simulate:
  # S0: file; Input: random; Output: "Kiss" format on stdout
  #

#simulate --simFlag 0 --depth -1 --pattern s27pat.sim --result stdout

  #
  # Simulate:
  # S0: file; Input: file; Output: "Kiss" format on stdout
  #

#simulate --simFlag 0 --depth -1 --init s27init.sim --pattern s27pat.sim --result s27out1.sim

  #
  # Simulate:
  # S0: file; Input: file; Output: Bit Values on File s27out1.sim
  #

#simulate --simFlag 1 --depth -1 --init s27init.sim --pattern s27pat.sim --result s27out2.sim

  #
  # Simulate:
  # S0: file; Input: file; Output: Wave Forms on File s27out1.sim
  #

#simulate --simFlag 2 --depth -1 --init s27init.sim --pattern s27pat.sim --result s27out3.sim

  #
  # Simulate With DAC'99 ---> NOT YET IMPLEMENTED
  #

#set fsm.verbosity 9
#simulate --depth 10 --deadEnd 3 --simulaWithDac99
#simulate --depth -1 --deadEnd -1 --logPeriod 10 --depthBreadth 1 --random 0 

  #
  # Quit PdTrav
  #

quit
