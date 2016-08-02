#!/bin/csh
# Process a set of maps with Shape2Map 
# Usage: process_maps file file file ...
# F. Hage Dec 2010

foreach i ($*)
  echo "Converting $i to $i:r.map"
  Shape2Map $i > $i:r.map
end
