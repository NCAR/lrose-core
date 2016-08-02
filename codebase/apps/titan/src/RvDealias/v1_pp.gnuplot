# Gnuplot script file for plotting p1 data
#
set title "PP-estimated moment vs. simulated data - Trip 1 velocity"
set xlabel "V1 - simulated (dBM)"
set ylabel "V1 - PP (dBM)"
set xrange[-30:30] 
set yrange[-30:30] 
set nolabel
set nokey
plot "verify_pp/moments.table" using 4:3
