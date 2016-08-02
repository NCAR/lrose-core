# Gnuplot script file for plotting p2 data
#
set title "FFT-estimated moment vs. simulated data - Trip 2 width"
set xlabel "W2 - simulated (dBM)"
set ylabel "W2 - FFT (dBM)"
set xrange[-2:8] 
set yrange[-2:8] 
set nolabel
set nokey
plot "verify_fft/moments.table" using 12:11
