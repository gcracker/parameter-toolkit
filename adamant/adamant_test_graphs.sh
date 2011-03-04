#!/bin/bash

make clean;make debug;

./adamantium_debug
# dot -Tps /tmp/ddtestmainNode.dot > ddMainNode.ps
# dot -Tps /tmp/ddtestSwapNode.dot > ddMainSwap.ps
# dot -Tps /tmp/ddtestIntervalX.dot > ddIntervalX.ps
# dot -Tps /tmp/ddtestIntervalX.dot > ddIntervalY.ps
# #dot -Tps /tmp/ddtestIntervalProd.dot > ddIntervalProd.ps
# dot -Tps /tmp/ddtestInterFinal.dot > ddFinal.ps
# dot -Tps /tmp/ddtestSliceY.dot > ddSliceY.ps
# dot -Tps /tmp/ddtestSliceX.dot > ddSliceX.ps

dot -Tps /tmp/ddtestsliceNode.dot > ddMainNode.ps
dot -Tps /tmp/ddtestsliceTuple.dot > ddTuple.ps
dot -Tps /tmp/ddtestsliceInitAb.dot > ddAb.ps
dot -Tps /tmp/ddtestsliceInitSwap.dot > ddInitSwap.ps
dot -Tps /tmp/ddtestsliceFinal.dot > ddFinal.ps

#rm /tmp/ddtestsliceInter.dot ddIn.ps /tmp/ddtestsliceAbstract.dot ddabs.ps
#rm /tmp/ddtestsliceSwapped.dot ddswap.ps /tmp/ddtestsliceUnion.dot ddUnion.ps
#rm /tmp/ddtestslicexFullCube.dot ddxfullcube.ps /tmp/ddtestsliceTuple.dot ddTuple.ps
#rm /tmp/ddtestsliceNode.dot ddMainNode.ps /tmp/ddtestsliceProduct.dot ddProduct.ps

#./adamantium_debug
#dot -Tps /tmp/ddtestsliceNode.dot > ddMainNode.ps
#dot -Tps /tmp/ddtestsliceTuple.dot > ddTuple.ps
#dot -Tps /tmp/ddtestsliceInter.dot > ddIn.ps
#dot -Tps /tmp/ddtestsliceAbstract.dot > ddabs.ps
#dot -Tps /tmp/ddtestsliceSwapped.dot > ddswap.ps
#dot -Tps /tmp/ddtestsliceUnion.dot > ddUnion.ps
#dot -Tps /tmp/ddtestslicexFullCube.dot > ddxfullcube.ps
#dot -Tps /tmp/ddtestsliceProduct.dot > ddProduct.ps
