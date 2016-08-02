#! /bin/sh
#
# putXwd2Ftp.sh
#
# Purpose - do xwd on root window and send output to ftp server
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

cd /tmp
xwd -display :0.0 -root > titanmx.xwd
gzip -f titanmx.xwd
FILE="titanmx.xwd.gz"
anon_ftp_put


