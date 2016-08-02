#!/bin/csh -x 
#
# 01/26/2005  Celia Chen
# Script to archive RUC13km model files of yesterday (in Z time), received via LDM's CONDUIT feed.
# It first runs FTP scripts to make sure all the files are in before sending files to the MSS
#
# Alternatively, the user can specify the date string on the command line for a defined
# archive date.
#
#############################################################################################
set EXE_home = /home/rapdmg/exe
set ARC_SCRIPTS_home  = /home/rapdmg/archive_scripts

# MSS_path = /RAPDMG/grib/model_id/date/model_output_file_name

# find a specific date defined on the command line: 
# Get today's date 

  if ( $1 == "" ) then
    set aryr = `$EXE_home/gemtime -o -48h %y`
    set aryear = `$EXE_home/gemtime -o -48h %Y`
    set aryrmn = `$EXE_home/gemtime -o -48h %Y%m`
    set ardate = `$EXE_home/gemtime -o -48h %Y%m%d`
    set armmdd = `$EXE_home/gemtime -o -48h %m%d`
    set aryrmmddhhmm = `$EXE_home/gemtime -o -48h %Y%m%d%H%M`
    echo "ardate, arymdhm =" $ardate $aryrmmddhhmm
#    echo "ardate, armmdd, aryrmn, aryr, aryear =" $ardate $armmdd $aryrmn $aryr $aryear
  else
    set ardate = $1
  endif
  echo "ardate, armmdd=" $ardate $armmdd
#

foreach mid (RUC13kmDEV2s RUC13kmDEV2b RUC13kmDEV2p)
    set mid_dir = "/grib/RUC13-20/$mid/$ardate"
    set MSS_path = "/RAPDMG/grib/$mid"

    set time1 = `date`
    echo "Start time for $mid : " $time1
    echo " Sending data to the MSS from Model ID = $mid"

    msrcp -pr 48500002 -R -pe 4096 $mid_dir mss:$MSS_path
    msclass -class -R reliability=economy $MSS_path
    echo "msrcp -pr 48500002 -R -pe 4096 $mid_dir mss:$MSS_path"
#    msls -lR /RAPDMG/grib/$mid/$ardate > /home/celia/Archive/MSlist/$mid"_"$ardate"_msout"
#
#
# Now schedule a job to make sure the archive went OK, and if not, resend the files to the MSS
#
  sleep 10
  echo "Now run the $ARC_SCRIPTS_home/compare_msarchive.script for $mid on $ardate"
  $ARC_SCRIPTS_home/compare_msarchive.script_ruc13-20 $mid $ardate
#
  set time2 = `date`
  echo "End time for $mid : " $time2

end

exit

