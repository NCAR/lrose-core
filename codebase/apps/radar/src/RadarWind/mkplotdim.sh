#!/bin/bash

# Use gnuplot to create a plot of "calculated vs true"
# for each dimension of the output of RadarWind.cc.
#
# Follows mkwind.sh.


if [ "$#" -ne "4" ]
then
  echo mkplotdim.sh: wrong num args
  echo mkplotdim.sh: Parms:
  echo "  layer:   either all or number like 3 for z level in km MSL"
  echo "  fmt:     pdf or png"
  echo "  outdir:  dir containing output of mkwind.sh"
  echo "  outtag:  tag for output files, like plotSub"
  exit 1
fi

layer=$1
fmt=$2
outdir=$3
outtag=$4

echo layer: $layer
echo fmt: $fmt
echo outdir: $outdir
echo outtag: $outtag


if [[ "$layer" == "all" ]]; then
  cp $outdir/res.txt $outdir/tempa
else
  egrep " locZYX:  $layer " $outdir/res.txt | egrep -v ' nan ' > $outdir/tempa
fi

if [[ "$fmt" == "pdf" ]]; then
  termstg="pdf color size 9in,9in"
  suffix="pdf"
elif [[ "$fmt" == "png" ]]; then
  termstg="png large size 1000,800"
  suffix="png"
else
  echo invalid fmt: $fmt
  exit 1
fi


#OLD:
## column, origin 1:
##           3      5       7          9  10 11        13       15                17                19       21                23                25               27               29                  31
## ckv:  iz: 5  iy: 55  ix: 195  loc:  3  2  28  Wver: 0  calc: 0.00706628  diff: 0.00706628  Vver: 0  calc: 0.09732759  diff: 0.09732759  Uver: 0.3349882  calc: 0.3345233  diff: -0.0004648696  mse: 0.0003399935
#
#





# column, origin 1:
#                  4                   8                    12                              16                                       20                                               24                      26                 28

# ckv:ok: 1  izyx: 0  22  71  locZYX:  0  -6  1  verifWVU:  -0  11.6332  10.6765  calcWVU:  -0.4517335  9.89126  13.25029  diffWVU:  -0.4517335  -1.741943  2.573792  meanNbrElevDeg: -18.97385  meanNbrDist: 2.810743  condNum: 4.232566




gnuplot << eof
set size square
unset key

set term $termstg
set output '$outdir/$outtag.w.$suffix'
set title "W wind: calculated vs true, at layer: $layer"
set xlabel "true W, m/s"
set ylabel "calculated W, m/s"
##set arrow 333 from -15,-15 to 15,15 nohead front
plot [][] '$outdir/tempa' using 12:16 with p pt 7 ps 0.5, \
  (x)

set term $termstg
set output '$outdir/$outtag.v.$suffix'
set title "V wind: calculated vs true, at layer: $layer"
set xlabel "true V, m/s"
set ylabel "calculated V, m/s"
##set arrow 333 from 2,2 to 20,20 nohead front
plot [][] '$outdir/tempa' using 13:17 with p pt 7 ps 0.5, \
  (x)

set term $termstg
set output '$outdir/$outtag.u.$suffix'
set title "U wind: calculated vs true, at layer: $layer"
set xlabel "true U, m/s"
set ylabel "calculated U, m/s"
##set arrow 333 from 2,2 to 20,20 nohead front
plot [][] '$outdir/tempa' using 14:18 with p pt 7 ps 0.5, \
  (x)

eof

