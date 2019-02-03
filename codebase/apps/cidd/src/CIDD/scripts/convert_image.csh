#! /bin/csh
#  Convert a file to png

set new_file = $1:r.png
set tmp_file = $1:r.tmp.png

convert  -pen gray1 -opaque black $1 $tmp_file 

mv $tmp_file $new_file

\rm $1
