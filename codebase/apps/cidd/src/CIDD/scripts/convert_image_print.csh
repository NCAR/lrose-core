#! /bin/csh
#  Convert a file to postscript and then print to default printer

set new_file = $1:r.ps
set tmp_file = $1:r.tmp.ps

convert  -pen gray1 -opaque black $1 $tmp_file 

mv $tmp_file $new_file

lpr $new_file 

\rm $1
\rm $new_file
