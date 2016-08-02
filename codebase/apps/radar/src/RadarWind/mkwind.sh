#!/bin/bash

# Set up input for and run RadarWind.cc for the Eldora aircraft data.
#
# Typically followed by:
#   ./mkplotvec.sh
#   ./mkplotplan.sh


if [ "$#" -ne "12" ]
then
  echo mkwind.sh: wrong num args
  echo 'mkwind.sh: Positional parms:'
  echo '(See RadarWind.cc "badparms" for doc)'
  echo '  numNbrMax:  max num nearest nbrs'
  echo '  forceOk:    force all ncp, dbz to ok'
  echo '  useEigen:   y: use Eigen  n: use Cramer'
  echo '  zgrid:      inc, or min,max,inc'
  echo '  ygrid:      inc, or min,max,inc'
  echo '  xgrid:      inc, or min,max,inc'
  echo '  testMode'
  echo '  synWinds'
  echo '  radFiles'
  echo '  baseW'
  echo '  detailSpec'
  echo '  outDir'
  echo ''
  echo 'See further doc in README.'
  exit 1
fi
numNbrMax=$1
forceOk=$2
useEigen=$3
zgrid=$4
ygrid=$5
xgrid=$6
testMode=$7
synWinds=$8
radFiles=$9
baseW=${10}
detailSpec=${11}
outDir=${12}

echo mkwind.sh: numNbrMax: $numNbrMax
echo mkwind.sh: forceOk: $forceOk
echo mkwind.sh: useEigen: $useEigen
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

# get test swp files
qcType=qc
rm -rf /d1/steves/tda/tdswp
mkdir  /d1/steves/tda/tdswp
if [ "$qcType" == "raw" ]; then
  # Copy all
  cp /home/steves/tech/radar/data/michael.bell/tda/synthetic/* /d1/steves/tda/tdswp
  # Copy only a few
  ###(cd /home/steves/tech/radar/data/michael.bell/tda/synthetic; cp $(/bin/ls swp* | head -n 10)  /d1/steves/tda/tdswp)
elif [ "$qcType" == "qc" ]; then
  # Copy all
  cp /d1/steves/data/radar/bell.synthetic.qced.by.cory/tda/qced/* /d1/steves/tda/tdswp
  # Copy only a few
  ###(cd /d1/steves/data/radar/bell.synthetic.qced.by.cory/tda/qced; cp $(/bin/ls swp* | head -n 10)  /d1/steves/tda/tdswp)
else
  echo 'mkWind.sh: bad qcType'
  exit 1
fi


##vmstat 1 1000000 > $outDir/vmstat.log &
##sleep 2
##vmstatid=$!
##echo vmstatid: $vmstatid

LIBDIRB=/d1/steves/ftp/geographicLib/tdi/lib
LIBDIRD=/usr/local/netcdf4/lib
LIBDIRE=/d1/steves/cvs/libs/kd/src

echo 'mkwind.sh: start make'
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
  -projLat0 16.5 \
  -projLon0 148.0 \
  -baseW $baseW \
  -epsilon 1.e-6 \
  -maxDeltaAltKm 0 \
  -maxAbsElevDeg 50 \
  -minRadialDistKm 0 \
  -numNbrMax $numNbrMax \
  -maxDistBase 1 \
  -maxDistFactor 0.016666 \
  -forceOk $forceOk \
  -useEigen $useEigen \
  -inDir /d1/steves/tda/tdswp \
  -fileRegex '^swp' \
  -radialName VG \
  -dbzName DBZ \
  -ncpName NCP \
  -outTxt $outDir/res.txt \
  -outNc $outDir/ \
  -detailSpec $detailSpec \
  > $outDir/wind.log 2>&1


##sleep 2
##kill $vmstatid


##tar cvzf /d1/steves/tda/junka.tgz $outDir
##(cd /d1/steves/tda; putralftp junka.tgz)



