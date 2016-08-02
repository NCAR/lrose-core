# Gnuplot script file for plotting p2 data
#
set title "FFT-estimated moment vs. simulated data - Trip 2 velocity"
set xlabel "V2 - simulated (dBM)"
set ylabel "V2 - FFT (dBM)"
set xrange[-30:30] 
set yrange[-30:30] 
set nolabel
set nokey
plot "verify_fft/moments.table" using 10:9

