#!/bin/bash

# Use gnuplot to create a vector field plot
# of the output of RadarWind.cc.
#
# Follows mkwind.sh.


if [ "$#" -ne "12" ]
then
  echo mkplotvec.sh: wrong num args
  echo mkplotvec.sh: Parms:
  echo "  ndim:"
  echo "    2:       make 2 dim png"
  echo "    3a:      make 3 dim gnuplot interactive"
  echo "    3b:      make 3 dim rotating gif"
  echo "  zlims:     zlow,zhigh  (reqd but not used for ndim==2)"
  echo "  ylims:     ylow,yhigh"
  echo "  xlims:     xlow,xhigh"
  echo "  layer:     either all or number like 3 for z level in km MSL"
  echo "  voffset    subtract this value from V winds"
  echo "  uoffset    subtract this value from U winds"
  echo "  originLat  orgin latitude"
  echo "  originLon  orgin longitude"
  echo "  scales:    scales parm for gnuplot.arrows.py"
  echo "               2d: 0.2,0.2   3d: 0.2,0.2,0.2"
  echo "  outdir:    dir containing output of mkwind.sh"
  echo "  outtag:    tag for output files, like plotSub"
  exit 1
fi

ndim=$1
zlims=$2
ylims=$3
xlims=$4
layer=$5
voffset=$6
uoffset=$7
originLat=$8
originLon=$9
scales=${10}
outdir=${11}
outtag=${12}

echo "mkplotvec.sh"
echo "  ndim: $ndim"
echo "  zlims: $zlims"
echo "  ylims: $ylims"
echo "  xlims: $xlims"
echo "  layer: $layer"
echo "  voffset: $voffset"
echo "  uoffset: $uoffset"
echo "  originLat: $originLat"
echo "  originLon: $originLon"
echo "  scales: $scales"
echo "  outdir: $outdir"
echo "  outtag: $outtag"

# Select the specified layer
if [[ "$layer" == "all" ]]; then
  cp $outdir/res.txt $outdir/$outtag.data
else
  egrep " locZYX:  $layer " $outdir/res.txt > $outdir/$outtag.data
fi

# column, origin 1:
#                  4                   8                    12                              16                                       20                                               24                      26                 28

# ckv:ok: 1  izyx: 0  22  71  locZYX:  0  -6  1  verifWVU:  -0  11.6332  10.6765  calcWVU:  -0.4517335  9.89126  13.25029  diffWVU:  -0.4517335  -1.741943  2.573792  meanNbrElevDeg: -18.97385  meanNbrDist: 2.810743  condNum: 4.232566


if [[ "$ndim" == "2" ]]; then

  ./gnuplot.arrows.py \
    -ndim 2 \
    -tag true  -inFile $outdir/$outtag.data \
      -lt 2 -lw 1.0 -scales $scales -locCols 10,9 -valCols 14,13 \
      -offsets $voffset,$uoffset \
    -head 'set term png large size 1000,800' \
    -head "set output '$outdir/$outtag.true.png'" \
    -head "set size square" \
    -head "set title 'True wind field at $layer km height'" \
    -head "set bmargin 6" \
    -head "set label 1111 'Notes: Velocities have offsets V=$voffset, U=$uoffset removed.  Origin is lat=$originLat, lon=$originLon' at screen 0.1,0.03" \
    -head "set xlabel 'E-W distance in km from longitude $originLon'" \
    -head "set ylabel 'N-S distance in km from latitude $originLat'" \
    -head "unset key" \
    -tail 'set grid' \
    -tail 'plot NaN lt 2' \
    -xlims $xlims \
    -ylims $ylims \
    -outFile $outdir/$outtag.true.cmd

  ./gnuplot.arrows.py \
    -ndim 2 \
    -tag calc  -inFile $outdir/$outtag.data \
      -lt 3 -lw 1.0 -scales $scales -locCols 10,9 -valCols 18,17 \
      -offsets $voffset,$uoffset \
    -head 'set term png large size 1000,800' \
    -head "set output '$outdir/$outtag.calc.png'" \
    -head "set size square" \
    -head "set title 'Calculated wind field at $layer km height'" \
    -head "set bmargin 6" \
    -head "set label 1111 'Notes: Velocities have offsets V=$voffset, U=$uoffset removed.  Origin is lat=$originLat, lon=$originLon' at screen 0.1,0.03" \
    -head "set xlabel 'E-W distance in km from longitude $originLon'" \
    -head "set ylabel 'N-S distance in km from latitude $originLat'" \
    -head "unset key" \
    -tail 'set grid' \
    -tail 'plot NaN lt 3' \
    -xlims $xlims \
    -ylims $ylims \
    -outFile $outdir/$outtag.calc.cmd

  # diffs use offset = 0
  ./gnuplot.arrows.py \
    -ndim 2 \
    -tag true  -inFile $outdir/$outtag.data \
      -lt 1 -lw 1.0 -scales $scales -locCols 10,9 -valCols 22,21 \
      -offsets 0,0 \
    -head 'set term png large size 1000,800' \
    -head "set output '$outdir/$outtag.diff.png'" \
    -head "set size square" \
    -head "set title 'Difference (calc - true) wind field at $layer km height'" \
    -head "set bmargin 6" \
    -head "set label 1111 'Notes: Velocities have offsets V=$voffset, U=$uoffset removed.  Origin is lat=$originLat, lon=$originLon' at screen 0.1,0.03" \
    -head "set xlabel 'E-W distance in km from longitude $originLon'" \
    -head "set ylabel 'N-S distance in km from latitude $originLat'" \
    -head "unset key" \
    -tail 'set grid' \
    -tail 'plot NaN lt 1' \
    -xlims $xlims \
    -ylims $ylims \
    -outFile $outdir/$outtag.diff.cmd

#    -tag estim -inFile $outdir/$outtag.data \
#      -lt 1 -lw 1.0 -scales $scales -locCols 11,10 -valCols 27,21 \
#      -offsets $voffset,$uoffset \
#    -tail 'plot NaN lt 2 title "true", NaN lt 1 title "estimated"' \



elif [[ "$ndim" == "3a" ]]; then
  ./gnuplot.arrows.py \
    -ndim 3 \
    -tag true  -inFile $outdir/$outtag.data \
      -lt 2 -lw 1.0 -scales $scales -locCols 11,10,9 -valCols 11,10,9 \
      -offsets $voffset,$uoffset,0 \
    -tag estim -inFile $outdir/$outtag.data \
      -lt 1 -lw 1.0 -scales $scales -locCols 11,10,9 -valCols 14,13,12 \
      -offsets $voffset,$uoffset,0 \
    -head "set title 'Wind field at $layer km height'" \
    -head "set bmargin 6" \
    -head "set label 1111 'Notes: Velocities have offsets V=$voffset, U=$uoffset removed.  Origin is lat=$originLat, lon=$originLon' at screen 0.1,0.03" \
    -head "set xlabel 'E-W distance in km from longitude $originLon'" \
    -head "set ylabel 'N-S distance in km from latitude $originLat'" \
    -head "set zlabel 'km MSL' rotate by 90" \
    -head "set grid" \
    -head "set key right top width 2 height 2 box" \
    -tail 'splot [][][] NaN lt 2 title "true", NaN lt 1 title "estimated"' \
    -tail 'pause -1 "Grab image with mouse.  Press enter to end."' \
    -xlims $xlims \
    -ylims $ylims \
    -zlims $zlims \
    -outFile $outdir/$outtag.cmd

elif [[ "$ndim" == "3b" ]]; then
  ./gnuplot.arrows.py \
    -ndim 3 \
    -tag true  -inFile $outdir/$outtag.data \
      -lt 2 -lw 1.0 -scales $scales -locCols 11,10,9 -valCols 25,19,13 \
      -offsets $voffset,$uoffset,0 \
    -tag estim -inFile $outdir/$outtag.data \
      -lt 1 -lw 1.0 -scales $scales -locCols 11,10,9 -valCols 27,21,15 \
      -offsets $voffset,$uoffset,0 \
    -head 'set term gif large size 1000,800 animate delay 10 loop 0 optimize' \
    -head "set output '$outdir/$outtag.gif'" \
    -head "set size square" \
    -head "set title 'Wind field at $layer km height'" \
    -head "set bmargin 6" \
    -head "set label 1111 'Notes: Velocities have offsets V=$voffset, U=$uoffset removed.  Origin is lat=$originLat, lon=$originLon' at screen 0.1,0.03" \
    -head "set xlabel 'E-W distance in km from longitude $originLon'" \
    -head "set ylabel 'N-S distance in km from latitude $originLat'" \
    -head "set zlabel 'km MSL' rotate by 90" \
    -head "set grid" \
    -head "set key right top width 2 height 2 box" \
    -tail 'splot [][][] NaN lt 2 title "true", NaN lt 1 title "estimated"' \
    -animate 70,30,390,1 \
    -xlims $xlims \
    -ylims $ylims \
    -zlims $zlims \
    -outFile $outdir/$outtag.cmd

fi


