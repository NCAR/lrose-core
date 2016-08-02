.\" CVS: $Id: scrub.l,v 1.1 1997/11/17 19:33:29 dixon Exp $
.TH scrub L "October 5th, 1994" scrub
.SH NAME
scrub \- remove old files and directories
.SH SYNOPSIS
.B scrub [-dnv] [-p
.IR pattern
]
.IR age-in-days
.IR pathname
\&.\|.\|.
.SH DESCRIPTION
.B scrub
removes files, and optionally directories, whose modification time is
greater than
.IR age-in-days,
from the directory tree at each
.IR pathname.
Optionally, it will remove only those files matching a Perl regular
expression.  By default, only files are removed, leaving the existing
directory structure intact.

The argument
.IR age-in-days
specifies the age, in days, any file older than which should be removed.
It may be any valid Perl expression which evaluates to a number.  Note
that, unlike
.BR find (1),
this may be a fractional number, e.g. "1/12", for two hours.
.SH OPTIONS
.TP
.B \-d
Remove directories as well as files.
.TP
.B \-n
Do not actually remove anything, but print the list of files and/or
directories that would be removed.
.TP
.B \-p
Remove only files or directories matching the next argument
.IR pattern,
which may be a Perl regular expression.
.TP
.B \-v
Print the file and/or directory names to be removed.
.SH EXAMPLES
To remove all files older than two hours ending with ".o", in or below the
current directory:
.IP
scrub -p '\\.o$' 1/12 .
.SH SEE ALSO
.BR perl (1),
.BR find (1)
.SH AUTHOR
Tres Hofmeister <tres@ncar.ucar.edu>
.SH BUGS
.B scrub
represents a fairly simple approach to file aging.  If you need a solution
to a complex file aging problem, you might take a look at
.BR find (1)
and
.B find2perl.

.B find2perl
is a Perl program which takes a find(1) command and translates it into
Perl code which performs the same task, which may then be modified to do
something that
.BR find (1)
perhaps cannot do.  It is documented in O'Reilly & Associates'
.IR "Programming perl."

An example:
.IP
find2perl / -name .nfs\\* -mtime +7 \\
    -exec rm -f {} \\; -o -fstype nfs -prune
.TP
\&.\|.\|.produces the following Perl code:
.IP
require "find.pl";

&find('/');

sub wanted {
    /^\\.nfs.*$/ &&
    (($dev,$ino,$mode,$nlink,$uid,$gid) = lstat($_)) &&
    int(-M _) > 7 &&
    (unlink($_) || 1)
    ||
    ($nlink || (($dev,$ino,$mode,$nlink,$uid,$gid) = lstat($_))) &&
    $dev < 0 &&
    ($prune = 1);
}
