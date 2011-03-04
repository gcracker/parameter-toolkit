#!/bin/bash

EXE=${HOME}/hgrepo/cmpopp/adamant

time $EXE/./adamantium -t ~/lstrace.bz2 -p ~/lsprogram.bz2 --dddinvsrdy /dev/null --ddslotsmulti=100 --usezdds

time $EXE/./adamantium -t ~/lstrace.bz2 -p ~/lsprogram.bz2 --dddinvsrdy /dev/null --ddslotsmulti=200 --usezdds

time $EXE/./adamantium -t ~/lstrace.bz2 -p ~/lsprogram.bz2 --dddinvsrdy /dev/null --ddslotsmulti=300 --usezdds

time $EXE/./adamantium -t ~/lstrace.bz2 -p ~/lsprogram.bz2 --dddinvsrdy /dev/null --ddslotsmulti=100 --usezdds

time $EXE/./adamantium -t ~/lstrace.bz2 -p ~/lsprogram.bz2 --dddinvsrdy /dev/null --ddslotsmulti=200 --usezdds

time $EXE/./adamantium -t ~/lstrace.bz2 -p ~/lsprogram.bz2 --dddinvsrdy /dev/null --ddslotsmulti=300 --usezdds

#$EXE/./adamantium -t ~/lstrace.bz2 -p ~/lsprogram.bz2 --dddinvsrdy /dev/null --ddslotsmulti=100 --tick=100000 --usezdds

#time $EXE/./adamantium -t ~/lstrace.bz2 -p ~/lsprogram.bz2 --dddinvsrdy /dev/null --ddslotsmulti=1

#$EXE/./adamantium -t ~/lstrace.bz2 -p ~/lsprogram.bz2 --dddinvsrdy /dev/null --ddslotsmulti=100 --tick=100000
