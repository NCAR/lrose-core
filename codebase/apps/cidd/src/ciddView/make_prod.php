<?php
/////////////////////////////////////////////////////////////////////
// Product Servlet - Returns products wrapped in XML.
// Called from ciddView.php
// Copyright (c) 2006, University Corporation for Atmospheric Research(UCAR)
// National Center for Atmospheric Research(NCAR)
// Research Applications Program(RAP)
// BSD-style license applies.

require("scripts/fileSelect_php.inc");


// record when called.
$utcstr =  gmdate("H:i:s M d Y");

/////////////////////////////////////////////////////////////////////
// Start the response.
 header("Content-type: text/xml");
 echo "<?xml version=\"1.0\"  ?>";
 echo "<response>";

//
// Set defaults - although in reality these things are never empty. Niles.
//

  if(empty($fl))$fl="CTI_VEL";
  if(empty($zm))$zm="REAL_FULL";
  if(empty($tr))$tr=1800;
  if(empty($ht))$ht=24;
  if(empty($et))$et="now";

  $dir = "images";
//
// This is kludgey - if it is a CTI field, then
// we want to include the height in the selector since
// a 3D field - otherwise, don't include height.
//
  if (strstr($fl, "CTI")){
   $select = "{$fl}_{$zm}_{$ht}";
  } else {
   $select = "{$fl}_{$zm}";
  }

//
// Get the image file names.
//

  $fnames = get_best_array($select, $dir, $tr, $et);

  $numFound = count( $fnames );

//
// And assemble our XML.
//

 echo "<product>";
   echo "<title>Images</title>";
   echo "<select>{$select}</select>";

   echo "<prod_html><![CDATA[";   // Encapsulated HTML
     echo "<span class=\"data_plot\">";
     echo "<img src=\"{$dir}/{$fnames[ $numFound -1 ]}\" ID=\"Pan_img\">";
     echo "</span>";
   echo "]]></prod_html>";

   echo "<status_html><![CDATA[";   // Encapsulated HTML
     echo "<span class=\"status\">";
	 echo "<h3> Updated: {$utcstr} Z </h3>";
     echo "</span>";
   echo "]]></status_html>";

   echo "<target>Pan_div</target>";

   echo "<zm>{$zm}</zm>";

   echo "<nframes>{$numFound}</nframes>";

   $i = 0;
   while( $i < $numFound) {
	  echo "<frame>{$dir}/{$fnames[$i]} </frame>";
	  $i++;
   }

 echo "</product>";



/////////////////////////////////////////////////////////////////////
// Conclude the response.
  echo "</response>";

?>













