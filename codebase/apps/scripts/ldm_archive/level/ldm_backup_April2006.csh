#!/bin/csh -x
#


#############################################################################################
## UGLY little scripting to hopefully get most/all of the data that got dropped when the MSS project # got changed.

### NOTE THIS SCRIPT IS NOT COMPLETE -- data was archived, so I just moved it into the correct location by hand

# 4/21 4/22 4/23

#nids 
msrcp -pr 48500002 -pe 4096 /home/rapdmg/archive_staging/nids/20060421_all.nids.tar mss:/RAPDMG/LDM/ARCHIVE/2006/0421/20060421_all.nids.tar
msclass -class reliability=economy /RAPDMG/LDM/ARCHIVE/2006/0421/20060421_all.nids.tar"
msrcp -pr 48500002 -pe 4096 /home/rapdmg/archive_staging/nids/20060422_all.nids.tar mss:/RAPDMG/LDM/ARCHIVE/2006/0422/20060422_all.nids.tar
msclass -class reliability=economy /RAPDMG/LDM/ARCHIVE/2006/0421/20060422_all.nids.tar"
msrcp -pr 48500002 -pe 4096 /home/rapdmg/archive_staging/nids/20060423_all.nids.tar mss:/RAPDMG/LDM/ARCHIVE/2006/0423/20060423_all.nids.tar
msclass -class reliability=economy /RAPDMG/LDM/ARCHIVE/2006/0421/20060423_all.nids.tar"

#nowrad
srcp -pr 48500002 -pe 4096 /home/rapdmg/archive_staging/nowrad/20060421_MASTER.nowrad.tar.gz mss:/RAPDMG/LDM/ARCHIVE/2006/0421/20060421_MASTER.nowrad.tar.gz
msclass -class reliability=economy /RAPDMG/LDM/ARCHIVE/2006/0421/20060421_MASTER.nowrad.tar.gz
msrcp -pr 48500002 -pe 4096 /home/rapdmg/archive_staging/nowrad/20060421_MASTER15.nowrad.tar.gz mss:/RAPDMG/LDM/ARCHIVE/2006/0421/20060421_MASTER15.nowrad.tar.gz
msclass -class reliability=economy /RAPDMG/LDM/ARCHIVE/2006/0421/20060421_MASTER15.nowrad.tar.gz
msrcp -pr 48500002 -pe 4096 /home/rapdmg/archive_staging/nowrad/20060421_USRAD.nowrad.tar.gz mss:/RAPDMG/LDM/ARCHIVE/2006/0421/20060421_USRAD.nowrad.tar.gz
msclass -class reliability=economy /RAPDMG/LDM/ARCHIVE/2006/0421/20060421_USRAD.nowrad.tar.gz
msrcp -pr 48500002 -pe 4096 /home/rapdmg/archive_staging/nowrad/20060421_USRAD15.nowrad.tar.gz mss:/RAPDMG/LDM/ARCHIVE/2006/0421/20060421_USRAD15.nowrad.tar.gz
msclass -class reliability=economy /RAPDMG/LDM/ARCHIVE/2006/0421/20060421_USRAD15.nowrad.tar.gz

srcp -pr 48500002 -pe 4096 /home/rapdmg/archive_staging/nowrad/20060422_MASTER.nowrad.tar.gz mss:/RAPDMG/LDM/ARCHIVE/2006/0422/20060422_MASTER.nowrad.tar.gz
msclass -class reliability=economy /RAPDMG/LDM/ARCHIVE/2006/0422/20060422_MASTER.nowrad.tar.gz
msrcp -pr 48500002 -pe 4096 /home/rapdmg/archive_staging/nowrad/20060422_MASTER15.nowrad.tar.gz mss:/RAPDMG/LDM/ARCHIVE/2006/0422/20060422_MASTER15.nowrad.tar.gz
msclass -class reliability=economy /RAPDMG/LDM/ARCHIVE/2006/0422/20060422_MASTER15.nowrad.tar.gz
msrcp -pr 48500002 -pe 4096 /home/rapdmg/archive_staging/nowrad/20060422_USRAD.nowrad.tar.gz mss:/RAPDMG/LDM/ARCHIVE/2006/0422/20060422_USRAD.nowrad.tar.gz
msclass -class reliability=economy /RAPDMG/LDM/ARCHIVE/2006/0422/20060422_USRAD.nowrad.tar.gz
msrcp -pr 48500002 -pe 4096 /home/rapdmg/archive_staging/nowrad/20060422_USRAD15.nowrad.tar.gz mss:/RAPDMG/LDM/ARCHIVE/2006/0422/20060422_USRAD15.nowrad.tar.gz
msclass -class reliability=economy /RAPDMG/LDM/ARCHIVE/2006/0422/20060422_USRAD15.nowrad.tar.gz

msrcp -pr 48500002 -pe 4096 /home/rapdmg/archive_staging/nowrad/20060423_MASTER.nowrad.tar.gz mss:/RAPDMG/LDM/ARCHIVE/2006/0423/20060423_MASTER.nowrad.tar.gz
msclass -class reliability=economy /RAPDMG/LDM/ARCHIVE/2006/0423/20060423_MASTER.nowrad.tar.gz
msrcp -pr 48500002 -pe 4096 /home/rapdmg/archive_staging/nowrad/20060423_MASTER15.nowrad.tar.gz mss:/RAPDMG/LDM/ARCHIVE/2006/0423/20060423_MASTER15.nowrad.tar.gz
msclass -class reliability=economy /RAPDMG/LDM/ARCHIVE/2006/0423/20060423_MASTER15.nowrad.tar.gz
msrcp -pr 48500002 -pe 4096 /home/rapdmg/archive_staging/nowrad/20060423_USRAD.nowrad.tar.gz mss:/RAPDMG/LDM/ARCHIVE/2006/0423/20060423_USRAD.nowrad.tar.gz
msclass -class reliability=economy /RAPDMG/LDM/ARCHIVE/2006/0423/20060423_USRAD.nowrad.tar.gz
msrcp -pr 48500002 -pe 4096 /home/rapdmg/archive_staging/nowrad/20060423_USRAD15.nowrad.tar.gz mss:/RAPDMG/LDM/ARCHIVE/2006/0423/20060423_USRAD15.nowrad.tar.gz
msclass -class reliability=economy /RAPDMG/LDM/ARCHIVE/2006/0423/20060423_USRAD15.nowrad.tar.gz

# WXBUG data archive
#
msrcp -pr 48500002 -pe 4096 /home/rapdmg/archive_staging/wxbug/20060423.wxbug.tar.gz mss:/RAPDMG/LDM/ARCHIVE/2006/0423/20060423.wxbug.tar.gz
msclass -class reliability=economy /RAPDMG/LDM/ARCHIVE/2006/0423/20060423.wxbug.tar.gz
msrcp -pr 48500002 -pe 4096 /home/rapdmg/archive_staging/wxbug/20060422.wxbug.tar.gz mss:/RAPDMG/LDM/ARCHIVE/2006/0423/20060423.wxbug.tar.gz
msclass -class reliability=economy /RAPDMG/LDM/ARCHIVE/2006/0423/20060422.wxbug.tar.gz
msrcp -pr 48500002 -pe 4096 /home/rapdmg/archive_staging/wxbug/20060421.wxbug.tar.gz mss:/RAPDMG/LDM/ARCHIVE/2006/0423/20060423.wxbug.tar.gz
msclass -class reliability=economy /RAPDMG/LDM/ARCHIVE/2006/0423/20060421.wxbug.tar.gz



# SOUNDINGS data archive
#
msrcp -pr 48500002 -pe 4096 /home/rapdmg/archive_staging/soundings/20060423.soundings.tar.gz mss:/RAPDMG/LDM/ARCHIVE/2006/0423/20060423.soundings.tar.gz
msclass -class reliability=economy /RAPDMG/LDM/ARCHIVE/2006/0423/20060423.soundings.tar.gz
msrcp -pr 48500002 -pe 4096 /home/rapdmg/archive_staging/soundings/20060423.soundings.tar.gz mss:/RAPDMG/LDM/ARCHIVE/2006/0423/20060423.soundings.tar.gz
msclass -class reliability=economy /RAPDMG/LDM/ARCHIVE/2006/0423/20060423.soundings.tar.gz
msrcp -pr 48500002 -pe 4096 /home/rapdmg/archive_staging/soundings/20060423.soundings.tar.gz mss:/RAPDMG/LDM/ARCHIVE/2006/0423/20060423.soundings.tar.gz
msclass -class reliability=economy /RAPDMG/LDM/ARCHIVE/2006/0423/20060423.soundings.tar.gz


# AMDAR data archive
#
msrcp -pr 48500002 -pe 4096 /home/rapdmg/archive_staging/amdar/20060423.amdar.tar.gz mss:/RAPDMG/LDM/ARCHIVE/2006/0423/20060423.amdar.tar.gz
msclass -class reliability=economy /RAPDMG/LDM/ARCHIVE/2006/0423/20060423.amdar.tar.gz
msrcp -pr 48500002 -pe 4096 /home/rapdmg/archive_staging/amdar/20060423.amdar.tar.gz mss:/RAPDMG/LDM/ARCHIVE/2006/0423/20060423.amdar.tar.gz
msclass -class reliability=economy /RAPDMG/LDM/ARCHIVE/2006/0423/20060423.amdar.tar.gz
msrcp -pr 48500002 -pe 4096 /home/rapdmg/archive_staging/amdar/20060423.amdar.tar.gz mss:/RAPDMG/LDM/ARCHIVE/2006/0423/20060423.amdar.tar.gz
msclass -class reliability=economy /RAPDMG/LDM/ARCHIVE/2006/0423/20060423.amdar.tar.gz

# PROF data archive
#
msrcp -pr 48500002 -pe 4096 /home/rapdmg/archive_staging/prof/20060423.prof.tar.gz mss:/RAPDMG/LDM/ARCHIVE/2006/0423/20060423.prof.tar.gz
msclass -class reliability=economy /RAPDMG/LDM/ARCHIVE/2006/0423/20060423.prof.tar.gz
msrcp -pr 48500002 -pe 4096 /home/rapdmg/archive_staging/prof/20060423.prof.tar.gz mss:/RAPDMG/LDM/ARCHIVE/2006/0423/20060423.prof.tar.gz
msclass -class reliability=economy /RAPDMG/LDM/ARCHIVE/2006/0423/20060423.prof.tar.gz
msrcp -pr 48500002 -pe 4096 /home/rapdmg/archive_staging/prof/20060423.prof.tar.gz mss:/RAPDMG/LDM/ARCHIVE/2006/0423/20060423.prof.tar.gz
msclass -class reliability=economy /RAPDMG/LDM/ARCHIVE/2006/0423/20060423.prof.tar.gz

# PIREP data archive
#
msrcp -pr 48500002 -pe 4096 /home/rapdmg/archive_staging/pirep/20060423.pirep.tar.gz mss:/RAPDMG/LDM/ARCHIVE/2006/0423/20060423.pirep.tar.gz
msclass -class reliability=economy /RAPDMG/LDM/ARCHIVE/2006/0423/20060423.pirep.tar.gz
msrcp -pr 48500002 -pe 4096 /home/rapdmg/archive_staging/pirep/20060423.pirep.tar.gz mss:/RAPDMG/LDM/ARCHIVE/2006/0423/20060423.pirep.tar.gz
msclass -class reliability=economy /RAPDMG/LDM/ARCHIVE/2006/0423/20060423.pirep.tar.gz
msrcp -pr 48500002 -pe 4096 /home/rapdmg/archive_staging/pirep/20060423.pirep.tar.gz mss:/RAPDMG/LDM/ARCHIVE/2006/0423/20060423.pirep.tar.gz
msclass -class reliability=economy /RAPDMG/LDM/ARCHIVE/2006/0423/20060423.pirep.tar.gz

# NLDN data archive
#
msrcp -pr 48500002 -pe 4096 /home/rapdmg/archive_staging/nldn/20060423.nldn.tar.gz mss:/RAPDMG/LDM/ARCHIVE/2006/0423/20060423.nldn.tar.gz
msclass -class reliability=economy /RAPDMG/LDM/ARCHIVE/2006/0423/20060423.nldn.tar.gz
msrcp -pr 48500002 -pe 4096 /home/rapdmg/archive_staging/nldn/20060423.nldn.tar.gz mss:/RAPDMG/LDM/ARCHIVE/2006/0423/20060423.nldn.tar.gz
msclass -class reliability=economy /RAPDMG/LDM/ARCHIVE/2006/0423/20060423.nldn.tar.gz
msrcp -pr 48500002 -pe 4096 /home/rapdmg/archive_staging/nldn/20060423.nldn.tar.gz mss:/RAPDMG/LDM/ARCHIVE/2006/0423/20060423.nldn.tar.gz
msclass -class reliability=economy /RAPDMG/LDM/ARCHIVE/2006/0423/20060423.nldn.tar.gz

#
# ACARS_UA data archive
msrcp -pr 48500002 -pe 4096 /home/rapdmg/archive_staging/acars_ua/20060423.acars_ua.tar.gz mss:/RAPDMG/LDM/ARCHIVE/2006/0423/20060423.acars_ua.tar.gz
msclass -class reliability=economy /RAPDMG/LDM/ARCHIVE/2006/0423/20060423.acars_ua.tar.gz
msrcp -pr 48500002 -pe 4096 /home/rapdmg/archive_staging/acars_ua/20060423.acars_ua.tar.gz mss:/RAPDMG/LDM/ARCHIVE/2006/0423/20060423.acars_ua.tar.gz
msclass -class reliability=economy /RAPDMG/LDM/ARCHIVE/2006/0423/20060423.acars_ua.tar.gz
msrcp -pr 48500002 -pe 4096 /home/rapdmg/archive_staging/acars_ua/20060423.acars_ua.tar.gz mss:/RAPDMG/LDM/ARCHIVE/2006/0423/20060423.acars_ua.tar.gz
msclass -class reliability=economy /RAPDMG/LDM/ARCHIVE/2006/0423/20060423.acars_ua.tar.gz

#
# ACARS_UPS data archive
msrcp -pr 48500002 -pe 4096 /home/rapdmg/archive_staging/acars_ups/20060423.acars_ups.tar.gz mss:/RAPDMG/LDM/ARCHIVE/2006/0423/20060423.acars_ups.tar.gz
msclass -class reliability=economy /RAPDMG/LDM/ARCHIVE/2006/0423/20060423.acars_ups.tar.gz
msrcp -pr 48500002 -pe 4096 /home/rapdmg/archive_staging/acars_ups/20060423.acars_ups.tar.gz mss:/RAPDMG/LDM/ARCHIVE/2006/0423/20060423.acars_ups.tar.gz
msclass -class reliability=economy /RAPDMG/LDM/ARCHIVE/2006/0423/20060423.acars_ups.tar.gz
msrcp -pr 48500002 -pe 4096 /home/rapdmg/archive_staging/acars_ups/20060423.acars_ups.tar.gz mss:/RAPDMG/LDM/ARCHIVE/2006/0423/20060423.acars_ups.tar.gz
msclass -class reliability=economy /RAPDMG/LDM/ARCHIVE/2006/0423/20060423.acars_ups.tar.gz

