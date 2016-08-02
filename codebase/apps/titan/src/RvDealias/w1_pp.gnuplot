# Gnuplot script file for plotting p1 data
#
set title "Pulse-pair-estimated trip 1 width vs. truth - simulated data"
set xlabel "W1 - simulated (dBM)"
set ylabel "W1 - PP-estimated (dBM)"
set xrange[-1:7] 
set yrange[-1:10] 
set nolabel
#set nokey
#plot "verify_pp/moments.table" using 6:5
plot "verify_pp/moments.table" using 6:($14 <= 5? $5 : 1/0) title "P1/P2 < 5", \
     "verify_pp/moments.table" using 6:($14 <= 10 && $14 > 5? $5 : 1/0) title "10 < P1/P2 < 5", \
     "verify_pp/moments.table" using 6:($14 > 10? $5 : 1/0) title "P1/P2 > 10"
# Gnuplot script file for plotting p1 data
#
#set title "PP-estimated moment vs. simulated data - Trip 1 width"
#set xlabel "W1 - simulated (dBM)"
#set ylabel "W1 - PP R1R2 (dBM)"
#set xrange[-2:50] 
#set yrange[-2:50] 
#set nolabel
#set nokey
#plot "verify_pp/moments.table" using 6:5
