# Gnuplot script file for plotting p1 data
#
set title "FFT-estimated moment vs. simulated data - Trip 1 power"
set xlabel "P1 - simulated (dBM)"
set ylabel "P1 - FFT (dBM)"
set xrange[-100:0] 
set yrange[-100:0] 
set nolabel
set nokey
plot "verify_fft/moments.table" using 2:1

