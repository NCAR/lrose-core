#!/bin/csh
#
# Get Yesterdays lightning data - Output goes into YYYYMMDD.txt
# Run from cron after 0Z, daily.

set d2=`date -u "+00:00_%m/%d/%Y" -d "1 day ago"`
set d1=`date -u "+00:00_%m/%d/%Y" -d "2 days ago"`

set fname = `date -u "+%Y%m%d_ltg.txt" -d "1 day ago"`

PrintLtg -l spdbp:://ncwf-4::ncwf/spdb/ltg $d1 $d2  -180 180 -90 90 > ${fname}
