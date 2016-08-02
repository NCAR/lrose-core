#!/bin/bash

# Use gnuplot to create a plan-view plots of the
# wind components U, V, W, and some diagnostic measures,
# using the output of RadarWind.cc.
#
# Follows mkwind.sh.


if [ "$#" -ne "4" ]
then
  echo mkplotdim.sh: wrong num args
  echo mkplotdim.sh: Parms:
  echo "  layer:   a number like 3 for z level in km MSL"
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

/bin/rm $outdir/$outtag.*.png

egrep " locZYX:  $layer " $outdir/res.txt > $outdir/temp.plan.data
mawk 'BEGIN {prev=-999999} {if ($9 != prev) print(""); prev = $9; print}' \
  $outdir/temp.plan.data > $outdir/temp.plan.datb

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


gnuplot << eof

smin(a,b) = (a < b) ? a : b
smax(a,b) = (a > b) ? a : b

unset key

set xlabel 'E-W distance in km from longitude 148.0'
set ylabel 'N-S distance in km from latitude 16.5'
set term $termstg


####set size square
#set palette rgbformulae 7,5,15    # black-violet-red-yellow. pm3d default


# True W, V, U

set output '$outdir/$outtag.true.w.$suffix'
set title 'True W wind at $layer km height (restricted to [-10,10])'
##plot [-80:80][-10:60] "$outdir/temp.plan.data" using 10:9:(smax(-10,smin(10,\$12))) with points palette pt 7 ps 1.
set pm3d map
splot [-80:80][-10:60] "$outdir/temp.plan.datb" using 10:9:(smax(-10,smin(10,\$12)))

set output '$outdir/$outtag.true.v.$suffix'
set title 'True V wind, less 10, at $layer km height (restricted to [-5,5])'
##plot [-80:80][-10:60] "$outdir/temp.plan.data" using 10:9:(smax(-5,smin(5,\$13-10))) with points palette pt 7 ps 1.
set pm3d map
splot [-80:80][-10:60] "$outdir/temp.plan.datb" using 10:9:(smax(-5,smin(5,\$13-10)))

set output '$outdir/$outtag.true.u.$suffix'
set title 'True U wind, less 10, at $layer km height (restricted to [-5,5])'
##plot [-80:80][-10:60] "$outdir/temp.plan.data" using 10:9:(smax(-5,smin(5,\$14-10))) with points palette pt 7 ps 1.
set pm3d map
splot [-80:80][-10:60] "$outdir/temp.plan.datb" using 10:9:(smax(-5,smin(5,\$14-10)))


# Calculated W, V, U

set output '$outdir/$outtag.calc.w.$suffix'
set title 'Calculated W wind at $layer km height (restricted to [-10,10])'
##plot [-80:80][-10:60] "$outdir/temp.plan.data" using 10:9:(smax(-10,smin(10,\$16))) with points palette pt 7 ps 1.
set pm3d map
splot [-80:80][-10:60] "$outdir/temp.plan.datb" using 10:9:(smax(-10,smin(10,\$16)))

set output '$outdir/$outtag.calc.v.$suffix'
set title 'Calculated V wind, less 10, at $layer km height (restricted to [-5,5])'
##plot [-80:80][-10:60] "$outdir/temp.plan.data" using 10:9:(smax(-5,smin(5,\$17-10))) with points palette pt 7 ps 1.
set pm3d map
splot [-80:80][-10:60] "$outdir/temp.plan.datb" using 10:9:(smax(-5,smin(5,\$17-10)))

set output '$outdir/$outtag.calc.u.$suffix'
set title 'Calculated U wind, less 10, at $layer km height (restricted to [-5,5])'
##plot [-80:80][-10:60] "$outdir/temp.plan.data" using 10:9:(smax(-5,smin(5,\$18-10))) with points palette pt 7 ps 1.
set pm3d map
splot [-80:80][-10:60] "$outdir/temp.plan.datb" using 10:9:(smax(-5,smin(5,\$18-10)))


# Diff calc-true for W, V, U

set output '$outdir/$outtag.diff.w8.$suffix'
set title 'Difference (calculated - true) for W wind at $layer km height (restricted to [-8,8])'
#plot [-80:80][-10:60] "$outdir/temp.plan.data" using 10:9:(smax(-8,smin(8,\$20))) with points palette pt 7 ps 1.
set pm3d map
splot [-80:80][-10:60] "$outdir/temp.plan.datb" using 10:9:(smax(-8,smin(8,\$20)))

set output '$outdir/$outtag.diff.w4.$suffix'
set title 'Difference (calculated - true) for W wind at $layer km height (restricted to [-4,4])'
#plot [-80:80][-10:60] "$outdir/temp.plan.data" using 10:9:(smax(-4,smin(4,\$20))) with points palette pt 7 ps 1.
set pm3d map
splot [-80:80][-10:60] "$outdir/temp.plan.datb" using 10:9:(smax(-4,smin(4,\$20)))

set output '$outdir/$outtag.diff.v.$suffix'
set title 'Difference (calculated - true) for V wind at $layer km height (restricted to [-2,2])'
#plot [-80:80][-10:60] "$outdir/temp.plan.data" using 10:9:(smax(-2,smin(2,\$21))) with points palette pt 7 ps 1.
set pm3d map
splot [-80:80][-10:60] "$outdir/temp.plan.datb" using 10:9:(smax(-2,smin(2,\$21)))

set output '$outdir/$outtag.diff.u.$suffix'
set title 'Difference (calculated - true) for U wind at $layer km height (restricted to [-2,2])'
#plot [-80:80][-10:60] "$outdir/temp.plan.data" using 10:9:(smax(-2,smin(2,\$22))) with points palette pt 7 ps 1.
set pm3d map
splot [-80:80][-10:60] "$outdir/temp.plan.datb" using 10:9:(smax(-2,smin(2,\$22)))


# meanNbrElevDeg

set output '$outdir/$outtag.nbr.elev20.$suffix'
set title 'Mean neighbor elevation, degrees, at $layer km height (restricted to [-20,20])'
#plot [-80:80][-10:60] "$outdir/temp.plan.data" using 10:9:(smax(-20,smin(20,\$24))) with points palette pt 7 ps 1.0
set pm3d map
splot [-80:80][-10:60] "$outdir/temp.plan.datb" using 10:9:(smax(-20,smin(20,\$24)))

set output '$outdir/$outtag.nbr.elev02.$suffix'
set title 'Mean neighbor elevation, degrees, at $layer km height (restricted to [-2,2])'
#plot [-80:80][-10:60] "$outdir/temp.plan.data" using 10:9:(smax(-2,smin(2,\$24))) with points palette pt 7 ps 1.0
set pm3d map
splot [-80:80][-10:60] "$outdir/temp.plan.datb" using 10:9:(smax(-2,smin(2,\$24)))


# meanNbrDist

set output '$outdir/$outtag.nbr.distAny.$suffix'
set title 'Mean neighbor distance, km, at $layer km height'
set pm3d map
splot [-80:80][-10:60] "$outdir/temp.plan.datb" using 10:9:26

set output '$outdir/$outtag.nbr.dist05.$suffix'
set title 'Mean neighbor distance, km, at $layer km height (restricted to [0,5])'
set pm3d map
splot [-80:80][-10:60] "$outdir/temp.plan.datb" using 10:9:(smin(5,\$26))


# conditionNum

set output '$outdir/$outtag.cond.num.$suffix'
set title 'Matrix condition number, at $layer km height (restricted to [0,10])'
#plot [-80:80][-10:60] "$outdir/temp.plan.data" using 10:9:(smin(10,\$28)) with points palette pt 7 ps 1.0
set pm3d map
splot [-80:80][-10:60] "$outdir/temp.plan.datb" using 10:9:(smin(5,\$26))

eof

##tar cvzf /d1/steves/tda/tempa.tgz $outdir/*.png
##(cd /d1/steves/tda; putralftp tempa.tgz)

