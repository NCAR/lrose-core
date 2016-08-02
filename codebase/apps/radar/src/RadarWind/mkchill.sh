#!/bin/bash

# Set up input for and run RadarWind.cc for data from the stationary
# radars CSU CHILL and CSU PAWNEE.
#
# Typically followed by:
#   ./mkplotvec.sh
#   ./mkplotplan.sh


if [ "$#" -ne "10" ]
then
  echo mkwind.sh: wrong num args
  echo mkwind.sh: Positional parms:
  echo "  numNbr:  num nearest nbrs"
  echo "  zgrid:   inc, or min,max,inc"
  echo "  ygrid:   inc, or min,max,inc"
  echo "  xgrid:   inc, or min,max,inc"
  echo "  testMode"
  echo "  synWinds"
  echo "  radFiles"
  echo "  baseW"
  echo "  detailSpec"
  echo "  outDir"
  echo ""
  echo 'See further doc in README.'
  exit 1
fi
numNbr=$1
zgrid=$2
ygrid=$3
xgrid=$4
testMode=$5
synWinds=$6
radFiles=$7
baseW=$8
detailSpec=$9
outDir=${10}

echo mkwind.sh: numNbr: $numNbr
echo mkwind.sh: zgrid: $zgrid
echo mkwind.sh: ygrid: $ygrid
echo mkwind.sh: xgrid: $xgrid
echo mkwind.sh: testMode: $testMode
echo mkwind.sh: synWinds: $synWinds
echo mkwind.sh: radFiles: $radFiles
echo mkwind.sh: baseW: $baseW
echo mkwind.sh: detailSpec: $detailSpec
echo mkwind.sh: outDir: $outDir

if [[ -e "$outDir" ]]; then
  echo "outDir already exists: $outDir"
  exit 1
fi

mkdir $outDir


LIBDIRB=/d1/steves/ftp/geographicLib/tdi/lib
LIBDIRD=/usr/local/netcdf4/lib
LIBDIRE=/d1/steves/cvs/libs/kd/src

make RadarWind
if [[ $? -ne 0 ]]; then exit 1; fi
echo 'mkwind.sh: make successful'


LD_LIBRARY_PATH=${LIBDIRB}:${LIBDIRD}:${LIBDIRE} \
/usr/bin/time ./RadarWind \
  -bugs 1 \
  -testMode $testMode \
  -synWinds $synWinds \
  -radFiles $radFiles \
  -zgrid $zgrid \
  -ygrid $ygrid \
  -xgrid $xgrid \
  -projName transverseMercator \
  -projLat0 40.44625 \
  -projLon0 -104.63708 \
  -baseW $baseW \
  -epsilon 1.e-6 \
  -maxDeltaAltKm 0 \
  -numNbr $numNbr \
  -fileList csu.chill.files \
  -radialName VE \
  -dbzName DZ \
  -ncpName NC \
  -outTxt $outDir/res.txt \
  -outNc $outDir/res.nc \
  -detailSpec $detailSpec \
  > $outDir/wind.log 2>&1


##sleep 2
##kill $vmstatid


##tar cvzf /d1/steves/tda/junka.tgz $outDir
##(cd /d1/steves/tda; putralftp junka.tgz)

