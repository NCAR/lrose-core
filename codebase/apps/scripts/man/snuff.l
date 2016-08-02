.\" CVS: $Id: snuff.l,v 1.1 1997/11/17 19:33:30 dixon Exp $
.TH snuff 1L "July 1st, 1996" snuff
.SH NAME
snuff \- kill processes by name
.SH SYNOPSIS
.B snuff [-l] [-s
.IR siglist
]
.IR process
\&.\|.\|.
.SH DESCRIPTION
.B snuff
kills the processes specified by name on the command line.  By default
snuff sends first the TERM signal, then the KILL signal, to each
process.  Optionally, snuff will send a user-specified list of signals,
.IR siglist.

snuff finds each process using
.BR ps (1),
with the appropriate operating-system-specific options.  Normally, snuff
kills only those
.IR processes
which appear to be actual program names.
Optionally, snuff will kill any process whose process table entry
contains a simple match for
.IR process.
.SH OPTIONS
.TP
.B \-l
Perform a "loose" match.  Kill any process which contains the string
specified in the
.IR process
argument(s).
.TP
.B \-s
Send the list of signals specified in
.IR siglist.
The signals in the list may be either signal names or numeric signal
values.  The list may be delimited by commas or spaces.  If spaces are
used, remember to quote the list to protect it from the invoking shell.
.SH SEE ALSO
.BR ps (1)
.SH AUTHOR
Tres Hofmeister <tres@ncar.ucar.edu>
.SH BUGS
.B snuff
will report that it has failed to kill
.IR process
if
.IR process
is still in the process table after all signals have been sent.  This is
not necessarily the desired result if an alternate
.IR siglist
was specified.
