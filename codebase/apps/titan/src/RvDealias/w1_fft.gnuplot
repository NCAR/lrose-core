# Gnuplot script file for plotting p1 data
#
set title "FFT-estimated trip 1 width vs. truth - simulated data"
set xlabel "W1 - simulated (dBM)"
set ylabel "W1 - FFT-estimated (dBM)"
set xrange[-1:7] 
set yrange[-1:10] 
set nolabel
#set nokey
#plot "verify_fft/moments.table" using 6:5
plot "verify_fft/moments.table" using 6:($14 <= 5? $5 : 1/0) title "P1/P2 < 5", \
     "verify_fft/moments.table" using 6:($14 <= 10 && $14 > 5? $5 : 1/0) title "10 < P1/P2 < 5", \
     "verify_fft/moments.table" using 6:($14 > 10? $5 : 1/0) title "P1/P2 > 10"
