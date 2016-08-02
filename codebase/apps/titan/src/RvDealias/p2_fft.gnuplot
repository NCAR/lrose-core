# Gnuplot script file for plotting p2 data
#
set title "FFT-estimated moment vs. simulated data - Trip 2 power"
set xlabel "P2 - simulated (dBM)"
set ylabel "P2 - FFT (dBM)"
set xrange[-100:0] 
set yrange[-100:0] 
set nolabel
set nokey
plot "verify_fft/moments.table" using 8:7

