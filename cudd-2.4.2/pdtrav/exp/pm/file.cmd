ddm_init
#ddm_dynord --enable --method sift --threshold 50000 ddm
fsm_read --blif s27.blif
fsm_pmbuild --blif s27.blif
tr_init --build
set tr.verbosity appMin
set tr.clustTh 500
tr_cluster --sort weight
stats tr.tr
stats tr
trav_init
set trav.verbosity appMax
traverse --verbosity appMax --logPeriod 1
stats ddm                                
