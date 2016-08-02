#! /bin/sh
#
# getXwdFromFtp.sh
#
# Purpose - get xwd from ftp server
#

# Debugging (off by default)

set -x

#   PATH

PATH=/bin:/usr/bin:/usr/sbin:/usr/bsd:/usr/local/bin:/usr/bin/X11:
export PATH

# trap signals

trap "echo ; echo 'Exiting at `date`' ; echo ; exit" 1 2 3 15

#   Definitions

PROGNAME=$0
PID=$$
FTP_SERVER="ftp.rap.ucar.edu"
LOGIN="anonymous"
PASSWD="dixon@ucar.edu"
FILE_DIR="incoming"

# subroutine to get file from anon ftp

anon_ftp_get()
{
	ftp -n << +EOF+
	open ${FTP_SERVER}
	user ${LOGIN} ${PASSWD}
	binary
	cd ${FILE_DIR}
	get ${FILE}
+EOF+
}

# subroutine to put file to anon ftp

anon_ftp_put()
{
	ftp -n << +EOF+
	open ${FTP_SERVER}
	user ${LOGIN} ${PASSWD}
	binary
	cd ${FILE_DIR}
	put ${FILE}
+EOF+
}

#   main()

cd /www/htdocs/projects/mexico/drip
FILE="titanmx.xwd.gz"
anon_ftp_get
gunzip -f titanmx.xwd.gz
imconv titanmx.xwd titanmx.gif
