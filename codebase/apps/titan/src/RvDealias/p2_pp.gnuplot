# Gnuplot script file for plotting p1 data
#
set title "PP-estimated moment vs. simulated data - Trip 2 power"
set xlabel "P2 - simulated (dBM)"
set ylabel "P2 - PP (dBM)"
set xrange[-100:0] 
set yrange[-100:0] 
set nolabel
set nokey
plot "verify_pp/moments.table" using 8:7
