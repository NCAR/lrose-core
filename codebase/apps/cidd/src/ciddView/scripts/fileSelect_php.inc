<?php
// PHP functions
// Niles Oien from Frank Hage's April 2005 work.
// Copyright UCAR (c) 2006 */
// University Corporation for Atmospheric Research(UCAR)
// National Center for Atmospheric Research(NCAR)
// Research Applications Program(RAP)
// BSD-style license applies.
//
// These are a collection of functions pertaining to getting image
// files for a certain field in a certain time range - Niles.
//
////////////////////////////////////////////////////////////////////////////
// get_best_array - Return an array of file names in dir_path where each contain
//              match_name
//

function get_best_array($match_name,$dir_path, $tr, $et) {

    $f = array();

    $handle = @opendir($dir_path);

    if($handle != FALSE) {
      while (false !== ($filename = readdir($handle))) {
        if(strstr($filename,$match_name)) {
           $age = get_age( $filename, $et );
	   if (($age > -1) && ($age < $tr)){
            $f[] = $filename;
         }
        }
      }
      sort($f);
    }

    $nFound = count( $f );

    if ($nFound == 0){
      echo "<H2> No data found for selection {$match_name} in time period ending {$et}, range {$tr} seconds. </H2>";
    }

    return $f;
}

////////////////////////////////////////////////////////////////////////////
// get_best_file - Return the latest file name in dir_path that contains
//                 match_name
//
function get_best_file($match_name,$dir_path, $tr, $et) {

    $f = array();
    $f = get_best_array($match_name,$dir_path, $tr, $et);

    $numFound = count( $f );
    if ($numFound == 0){
      echo "<H2> No data found for selection {$match_name} in time period ending {$et}, range {$tr} seconds. </H2>";
      return null;
    } else {
      return $f[ $numFound -1 ];
    }
}

/////////////////////////////////////////////////////
//
// Small function to get the file age by parsing a filename - Niles Oien.
//
function get_age( $fileName, $et ) {

//  echo "<H2> {$fileName} <--- </H2>";

  $start = strlen( $fileName ) - strlen( "YYYYMMDD_hhmmss.jpg" );

  if ($start < 0) return -1;

  $timeString = $fileName{$start} . $fileName{$start+1} . $fileName{$start+2} . $fileName{$start+3} . "-" . 
		$fileName{$start+4} . $fileName{$start+5} . "-" . 
                $fileName{$start+6} . $fileName{$start+7} . " " .
                $fileName{$start+9} . $fileName{$start+10} . ":" .
                $fileName{$start+11} . $fileName{$start+12} . ":" .
		$fileName{$start+13} . $fileName{$start+14} . " UTC";

  $utime = strtotime( $timeString );

  $nw = time();

   if (strstr($et, "_")){

     $len = strlen( $et );
     $j = 0;
     $adj = "";
     while ($j < $len){
       if ( $et{$j} == "_") {
            $adj = $adj . " ";
       } else {
            $adj = $adj . $et{$j};
       }
       $j++;
     }
     $adj = $adj . " UTC";
     $nw = strtotime( $adj );
   }

  if (($nw == 0) || ($utime == 0)) return -1;
  $age = $nw - $utime;

//  if (($age < 600) && ( $age > -1)){
//    echo "<H2> {$fileName} <==> {$timeString} <==> {$utime} <==> {$age} </H2>";
//  }

  return $age;

}

?>
