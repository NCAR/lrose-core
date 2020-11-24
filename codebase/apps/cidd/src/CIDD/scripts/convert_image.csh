#! /bin/csh
#  Convert a file to png

set new_file = $1:r.png

echo "INFO - convert_image.csh - converting $1 to $new_file"

if (-e $new_file) then
  echo "ERROR - $new_file already exists"
  exit 1
endif

set tmp_file = $1:r.tmp.png

echo "convert -stroke gray1 -opaque black $1 $tmp_file"
convert -stroke gray1 -opaque black $1 $tmp_file 

echo "mv $tmp_file $new_file"
mv $tmp_file $new_file
\rm $1
