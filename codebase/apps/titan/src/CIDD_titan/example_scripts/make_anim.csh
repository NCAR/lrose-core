#! /bin/csh
#  CIDD example script
#  The Most basic example script used for series save command in CIDD
#
# F. Hage NCAR/RAP  May 2000

echo "Leaving file output.gif in `pwd`"

convert -verbose -loop 0 -delay 50  $* output.gif &

echo "/bin/rm -f $*" | at now + 5 minutes
