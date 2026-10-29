.\" @(#)fdist v1.0b2 9/1/94
.TH fdist l "July 5th, 1995" fdist
.SH NAME
fdist \- file distribution utility
.SH SYNOPSIS
.B fdist
[-acdkls]
.if n .ti +4
[-f
.IR cfile ]
[-p
.IR pattern ]
[-q
.IR seconds ]
[-t
.IR tag ]
.if n .ti +4
[-x
.IR channels ]
.SH DESCRIPTION
.B fdist
is a configuration-file based file distribution utility.  It monitors a
directory for new files, and attempts to send them to a list of hosts
in a reliable fashion.
.SH OPTIONS
.TP
.B \-a
Send all files found in the data directory at startup, rather than
only sending new files.
.TP
.B \-c
Create a default configuration file, by default named 
.IR fdist.cf .
.TP
.B \-d
Enable debugging output.
.TP
.BI \-f " cfile"
Use the configuration file
.IR cfile
as the configuration file, rather than the default file
.IR fdist.cf .
.TP
.B \-k
Enable remote filesystem capacity checking.
.TP
.B \-l
Log miscellaneous status information via syslog(3).
.TP
.BI \-p " pattern"
Only send files matching the Perl regular expression
.IR pattern .
.TP
.BI \-q " seconds"
Send files only after they have been quiescent for the specified number
of
.IR seconds .
.TP
.B \-r
Recursively search for files in subdirectories.  Normally,
.IR fdist
ignores subdirectories in the interest of efficiency.
.TP
.B \-s
Enable checksum testing for files sent.  Note: Requires perl on the
remote systems.
.TP
.BI \-t " tag"
Specify a
.IR tag
for use with a master configuration file.  This allows one configuration
file to be customized for use by multiple instances of
.IR fdist .
See the comments in the default configuration file for details.
.TP
.BI \-x " channels"
Specify the number of file transmit channels to use.  This is the
number of remote copies
.IR fdist
will run in parallel.  The default is four.
.SH USAGE
.SS Configuration file
.B fdist
requires a configuration file.  A sample file may be created using the
-c command line option.  The new configuration file must then be
modified for use.  The sample file contains comments describing the
meaning of each possible keyword.
.SH SIGNALS
.B fdist
takes the following actions upon receiving these signals:
.TP 15
.B SIGHUP
Reread the configuration file just prior to the next time the data
directory is scanned.
.TP
.B SIGUSR1
Toggle the printing of debugging messages.
.SH FILES
.TP 15
.B fdist.cf
Default configuration file.
.SH SEE ALSO
.BR rdist (1)
.SH AUTHOR
Tres Hofmeister <tres@ncar.ucar.edu>
.SH BUGS
Unless the -q option is used,
.B fdist
expects files to appear in the data directory atomically.  If a file
appears and then grows or shrinks in size,
.B fdist
may not handle it gracefully.
