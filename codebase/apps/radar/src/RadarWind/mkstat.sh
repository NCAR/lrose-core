#!/bin/bash


# OBSOLETE OBSOLETE OBSOLETE OBSOLETE OBSOLETE OBSOLETE OBSOLETE

# Calulate overall statistics on the output of RadarWind.cc.
# Typically follows mkwind.sh.


if [ "$#" -ne "1" ]
then
  echo mkstat.sh: wrong num args
  echo mkstat.sh: Parms:
  echo "  outdir"
  exit 1
fi
outdir=$1

echo outdir: $outdir

echo 'num ok:0 lines:' $(egrep 'ckv:ok:0' $outdir/res.txt | wc -l)
echo 'num ok:1 lines:' $(egrep 'ckv:ok:1' $outdir/res.txt | wc -l)

column -t <(
  for tag in zmap ymap xmap; do
    egrep "^  $tag: " $outdir/wind.log | head -n 1
    egrep "^  $tag: " $outdir/wind.log | tail -n 1
  done
) > $outdir/map.log

egrep 'ckv:ok:1' $outdir/res.txt > $outdir/res.ok

cd $outdir
R --vanilla --quiet << reof > r.log
options(width=200)
fm <- read.table('res.ok')
colnames(fm) <- c(
  'tagckv',
  'tagiz', 'iz', 'tag', 'iy', 'tag', 'ix',
  'tagloc', 'z', 'y', 'x',
  'tagkz', 'kz', 'tag', 'ky', 'tag', 'kx',
  'tag', 'verW', 'tag', 'estW', 'tag', 'difW',
  'tag', 'verV', 'tag', 'estV', 'tag', 'difV',
  'tag', 'verU', 'tag', 'estU', 'tag', 'difU')

summary(fm)

cnames <- c('z', 'y', 'x', 'verW', 'verV', 'verU', 'difW', 'difV', 'difU')
fmb <- fm[,cnames]

mapply( summary, fmb)


cat( names( summary( fm$verV)), '\n')

resfm <- data.frame()
rownms <- c()

for (tp in c('ver', 'est', 'dif')) {
  for (fld in c('W', 'V', 'U')) {
    tag <- sprintf('%s%s', tp, fld)
    col <- fm[,tag]
    ##sm <- summary( col)
    sm <- c( length(col), min(col), median(col), mean(col), sd(col),
      median(abs(col)), mean(abs(col)), max(col))
    resfm <- rbind( resfm, sm)
    rownms <- c( rownms, tag)
    cat( tp, fld, sm, '\n' )
  }
}

colnames(resfm) <- c('num', 'min', 'median', 'mean', 'sd',
  'medianAbs', 'meanAbs', 'max')
rownames(resfm) <- rownms

resfm
reof

cd ..



