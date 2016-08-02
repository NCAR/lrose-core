#! /usr/local/bin/perl

#
# convert a map to zeb format
#

$prev_lat = -1000;
$prev_lon = -1000;

while ($line = <>) {

  ($lon1, $lat1, $lon2, $lat2) = ($line =~ m%(\d+\.\d+)\s*(\d+\.\d+)\s*(\d+\.\d+)\s*(\d+\.\d+).*%);
  
  if ($prev_lon != $lon1 || $prev_lat != $lat1) {

    #
    # print out header line and point data
    #

    if ($npoints > 0) {
    
      printf(STDOUT "%d %.4f %.4f %.4f %.4f\n",
	     $npoints * 2, $min_lat, $max_lat, $min_lon, $max_lon);

      for ($i = 0; $i < $npoints; $i++) {
	
	printf(STDOUT " %.4f %.4f", $lat_array[$i], $lon_array[$i]);
      
	$line_count++;
      
	if ($line_count == 4) {
	  printf(STDOUT "\n");
	  $line_count = 0;
	}
      
      }
    
      if ($line_count != 0) {
	printf(STDOUT "\n");
      }
    
    } # if ($npoints > 0) #

    #
    # initialize  
    #
    
    $lat = $lat1;
    $lon = $lon1;

    $min_lat = $lat;
    $max_lat = $lat;
    $min_lon = $lon;
    $max_lon = $lon;

    $lat_array[0] = $lat;
    $lon_array[0] = $lon;
    
    $npoints = 1;

  } # if ($prev_lon != $lon1 || $prev_lat != $lat1) #

  $lat = $lat2;
  $lon = $lon2;

  if ($lat < $min_lat) {
    $min_lat = $lat;
  }
  
  if ($lat > $max_lat) {
    $max_lat = $lat;
  }
  
  if ($lon < $min_lon) {
    $min_lon = $lon;
  }
  
  if ($lon > $max_lon) {
    $max_lon = $lon;
  }
  
  $lat_array[$npoints] = $lat;
  $lon_array[$npoints] = $lon;

  $npoints++;

  $prev_lat = $lat;
  $prev_lon = $lon;

} # while

#
# print out header line and point data for last segment
#

if ($npoints > 0) {
    
  printf(STDOUT "%d %.4f %.4f %.4f %.4f\n",
	 $npoints * 2, $min_lat, $max_lat, $min_lon, $max_lon);
  
  for ($i = 0; $i < $npoints; $i++) {
	
    printf(STDOUT " %.4f %.4f", $lat_array[$i], $lon_array[$i]);
      
    $line_count++;
      
    if ($line_count == 4) {
      printf(STDOUT "\n");
      $line_count = 0;
    }
      
  }
    
  if ($line_count != 0) {
    printf(STDOUT "\n");
  }
    
} # if ($npoints > 0) #










