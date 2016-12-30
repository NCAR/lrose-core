#!/bin/sh
#
# usage: python_copyright.csh python_script ...
#
# Inserts the copyright notice copyright.m after the 1st line in each python script named
#

TEMPFILE=/tmp/python_copyright$$.tmp

for file in $@; do

  (head -1 $file; cat `dirname $0`/copyright.m; tail +2 $file) > $TEMPFILE || exit 1
  mv $TEMPFILE $file || exit 1

done
