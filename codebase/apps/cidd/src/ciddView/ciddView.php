<HTML xmlns="http://www.w3.org/1999/xhtml" lang="en" xml:lang="en">

<!--

 Copyright (c) 2005, University Corporation for Atmospheric Research(UCAR)
 National Center for Atmospheric Research(NCAR)
 Research Applications Program(RAP)
 Copyright (c) 2005 National Center for Atmospheric Research 
 BSD-style license applies.

 This is the part of the system that presents the initial form to the
 user and allows them to make selections that are then passed off to
 make_prod.php - which makes the products and packages them off in XML
 to the javaScript running on the client.

 The top half of the file is HTML with PHP invoked a lot. The second
 half is the javaScript that passes off to make_prod.php.

 Niles Oien January 2006.

-->
<?php

require("scripts/fileSelect_php.inc");


// Set the defaults.

if(empty($fl))$fl="CTI_VEL";
if(empty($zm))$zm="REAL_FULL";
if(empty($tr))$tr=1800;
if(empty($ht))$ht=24;

$dir = "images";

$select = "{$fl}_{$zm}";


//
// This is kludgey - if it is a CTI field, then
// we want to include the height in the selector since
// its a 3D field - otherwise, don't include height.
//
if (strstr($fl, "CTI")){
 $select = "{$fl}_{$zm}_{$ht}";
} else {
 $select = "{$fl}_{$zm}";
}

$fname = get_best_file($select,$dir, $tr, 'now');
$pathname = "{$dir}/{$fname}";

?>

<!-- ******************************************************************** -->


<HEAD>
  <TITLE>ciddView</TITLE>
  <META http-equiv="Content-Type" content="text/html; charset=iso-8859-1" />
  <LINK type="text/css" href="css/ciddView.css" media="screen" rel="stylesheet" title="Default" />

</HEAD>

<!-- ******************************************************************** -->



<BODY onload="timer_f()" >
<?php


?>


<!--
  Start the form we present to the user. Note that we do NOT want the form
  itself to present ANY callbacks that might do anything - hence all the
  callbacks are set to do_nothing().

  Experience has shown that if an element in the form - in particular
  the text box - has a callback, AND the form has a callback, then the
  invoking of the two callbacks results in one of them being a dud
  in some way, which messes up the XmlHTTPRequest object (it caused
  the "oddball error" that I trap in scripts/ajax.js). It is rather odd! The
  bottom line is make sure everything only has one callback action.

  Niles.

 -->


<DIV CLASS="product_choices">

<FORM  
    name="choiceform" 
    method="POST" 
    action="<?php echo $PHP_SELF?>"
    onChange="return do_nothing()"
    onSubmit="return do_nothing()"
    onReset="return do_nothing()"
    onSelect="return do_nothing()"
    onBlur="return do_nothing()"
    onFocus="return do_nothing()" >

    <!-- LOOKBACK SELECT  - The lookback time in seconds. Niles. -->
   <DIV CLASS="choice_item">
     <h3> timeRange </h3>
                <SELECT name="timeRange" onChange="return set_timeRange()" >
                  <option value=300    <?php if($tr == 300)  echo "selected"?> >  5 Minutes </option>
                  <option value=900    <?php if($tr == 900)  echo "selected"?> > 15 Minutes </option>
                  <option value=1800   <?php if($tr == 1800) echo "selected"?> > 30 Minutes </option>
                  <option value=3600   <?php if($tr == 3600) echo "selected"?> > 60 Minutes </option>
                  <option value=7200   <?php if($tr == 7200) echo "selected"?> >120 Minutes </option>
                </SELECT>
   </DIV>  <!-- choice_item -->


    <!-- HEIGHT SELECT - This applies only to 3D fields. If you use this, you have
                         to edit make_prod.php as well since that needs to
                         have some smarts about what is a 3D field and what is not. Niles. -->
   <DIV CLASS="choice_item">
     <h3> Height </h3>
                <SELECT name="height" onChange="return set_height()" >
                  <option value=-1     <?php if($ht == -1)   echo "selected"?> >  -1 </option>
                  <option value=1.5    <?php if($ht == 1.5)  echo "selected"?> > 1.5 </option>
                  <option value=4      <?php if($ht == 4)    echo "selected"?> > 4   </option>
                  <option value=6.5    <?php if($ht == 6.5)  echo "selected"?> > 6.5 </option>
                  <option value=9      <?php if($ht == 9)    echo "selected"?> > 9   </option>
                  <option value=11     <?php if($ht == 11)   echo "selected"?> > 11  </option>
                  <option value=14     <?php if($ht == 14)   echo "selected"?> > 14  </option>
                  <option value=16     <?php if($ht == 16)   echo "selected"?> > 16  </option>
                  <option value=19     <?php if($ht == 19)   echo "selected"?> > 19  </option>
                  <option value=21     <?php if($ht == 21)   echo "selected"?> > 21 </option>
                  <option value=24     <?php if($ht == 24)   echo "selected"?> > 24 </option>
                </SELECT>
   </DIV>  <!-- choice_item -->


   <!-- FIELD SELECT - Fairly self-explanitory - the names line up with what CIDD uses in its filenames. Niles. -->
   <DIV CLASS="choice_item">
     <h3> Field </h3>
                <SELECT name="field"  onChange="return set_field()">
                  <option value="PolarReal"     <?php if($fl == "PolarReal")     echo "selected"?> > REAL Color </option>
                  <option value="AnotherField"  <?php if($fl == "AnotherField")  echo "selected"?> > REAL Grayscale </option>
                  <option value="CTI_VEL"       <?php if($fl == "CTI_VEL")       echo "selected"?> > CTI Velocity </option>
                </SELECT>
   </DIV>  <!-- choice_item -->

   <!-- ZOOM SELECT - again, the names line up with what CIDD uses in its filenames. Niles. -->
   <DIV CLASS="choice_item">
     <h3> View </h3>
                <SELECT name="zoom"  onChange="return set_zoom()">
                  <option value="REAL_FULL"     <?php if($zm == "REAL_FULL")    echo "selected"?> > Wide Area </option>
                  <option value="ANOTHER_ZOOM"  <?php if($zm == "ANOTHER_ZOOM") echo "selected"?> > Zoom In </option>
                </SELECT>
   </DIV>  <!-- choice_item -->

   <!-- END TIME SELECT - a textbox that lets the user "movie back" in time. The Janitor should save images
                          pertaining to things on the eventlist, if there is one, so this allows the user to
                          run ciddView in archive mode. Niles. -->
   <DIV CLASS="choice_item">
     <h3> End time YYYY-MM-DD_hh:mm:ss </h3>
                <INPUT type="text" size=25 name="endTime"
                  onChange="return set_endTime()"
                  onFocus="return do_nothing()"
                  onBlur="return do_nothing()"
                  onSelect="return do_nothing()"
                  onReset="return do_nothing()"
                  onSubmit="return do_nothing()" > 
                </INPUT>
   </DIV>  <!-- choice_item -->

</FORM>

<BR CLEAR=ALL>

</DIV>  <!-- product_choices -->



<!-- The main image viewing area - Niles. -->

<DIV ID="mainimage">
	<DIV ID="Pan_div" STYLE="position:absolute;visibility: visible;cursor:crosshair;"
               onSelectStart="return false" onMouseover="return false" 
               onMouseout="return false" onClick="return false" >
       <IMG ID="Pan_img"  BORDER=0 <?php echo "SRC=\"$pathname\""?> >
    </DIV> <!-- Pan_div -->
</DIV> <!-- mainimage -->
   

<!-- The animation control buttons - play, etc. Niles. -->


<DIV CLASS="anim_control" ID="anim_controls" >
   <FORM  name="animform" title="Animation control" >
		<title> Animation Control </TITLE>
		<INPUT name="First" value="First" type="Button" onClick="return first_f()"> </INPUT>
		<INPUT name="Next"  value="Next"  type="Button" onClick="return next_f()"> </INPUT>
		<INPUT name="Stop"  value="Stop"  type="Button" onClick="return stop_f()"> </INPUT>
		<INPUT name="Play"  value="Play"  type="Button" onClick="return play_f()"> </INPUT>
		<INPUT name="Prev"  value="Prev"  type="Button" onClick="return prev_f()"> </INPUT>
		<INPUT name="Last"  value="Last"  type="Button" onClick="return last_f()"> </INPUT>
                <INPUT name="Status"  value="Status"  type="Button" onClick="return debug_showStatus()"> </INPUT>
   </FORM>
</DIV>

<DIV ID="status_panel">
<H2> NCAR/RAL </H2>
</DIV>

<BR CLEAR=ALL>

<!-- The footer comes fitted with the standard attempt
     at protecting our intellekshual propertee, plus a couple of
     "public service announcements" for the user. Niles. -->

<SPAN CLASS="footer">
<div id="footer">
 ONLINE <A HREF=help.html> <I> HELP </I>  </a> AND INSTALLATION
 <A HREF=techSupport.html> <I> TECHNICAL SUPPORT </I>  </a>
 ARE AVALABLE<BR>
 Height selection applies only to CTI velocity field, REAL data are 2D<BR>
 REAL data come in every minute, CTI data every 5 minutes <BR>
 Data are resident on disk for 3 hours <BR>
 Entering "now" for data end time invokes realtime operation<BR>
 <HR>
 This is an experimental site created by Frank Hage and currently under Development by Niles Oien, oien@ucar.edu<BR>
 Copyright 2006 UCAR,RAL  BSD-style license applies. <BR>
 Do not publish links to this site
 </div> <!-- footer -->
 </SPAN>

</BODY>


<!-- PHP/HTML now mostly ends, we break out into javaScript at this point to pass the information
     from the FORM elements back to make_prod.php so that it can package up XML and
     send it off to the javaScript running on the client. Niles. -->

<SCRIPT type="text/javascript" src="scripts/funcs.js"></SCRIPT>
<SCRIPT type="text/javascript" src="scripts/ajax.js"></SCRIPT>

<SCRIPT>
<?php 

// Set the Current image height
echo "img_ht = 658;\n";
echo "img_wd = 821;\n";
?>
</SCRIPT>

<SCRIPT LANGUAGE="JavaScript">


// Set defaults. We have to do this repeatedly, we find. Niles.

var field = "CTI_VEL";
var zoom = "REAL_FULL";
var timeRange = 1800;
var endTime = "now";
var height = 24;

var a_mode =  0; // 0=Stop 1=Play 2=First  3=Last  4=Prev  5=Next
var nframes = 1;
var cur_frame = -1;
var last_frame = -1;


<?php 
?>

// Deal with the people who push our buttons - the animation controls, I mean. Niles.
function stop_f() { a_mode = 0; }
function play_f() { a_mode = 1; }
function first_f() { a_mode = 2; }
function last_f() { a_mode = 3; }
function prev_f() { a_mode = 4; }
function next_f() { a_mode = 5; }



// Record the results of the HTML form entries with small functions.
function set_field() {
 field = document.choiceform.field.options[document.choiceform.field.selectedIndex].value;
 loadImageData();
 return true;
}

function set_zoom() {
 zoom = document.choiceform.zoom.options[document.choiceform.zoom.selectedIndex].value;
 loadImageData();
 return true;
}

function set_timeRange() {
 timeRange = document.choiceform.timeRange.options[document.choiceform.timeRange.selectedIndex].value;
 loadImageData();
 return true;
}

function set_height() {
 height = document.choiceform.height.options[document.choiceform.height.selectedIndex].value;
 loadImageData();
 return true;
}

function set_endTime() {
 endTime = document.choiceform.endTime.value;
// alert("Endtime set : " + endTime);
 loadImageData();
 return true;
}

function do_nothing() {
 return false;
}


// Load the appropriate images - go to make_prod.php with the right argument list.
function loadImageData() {

 last_frame = -1; // Force re-draw
 cur_frame = -1;
 var args = '?fl=' + field + '&zm=' + zoom + '&tr=' + timeRange + '&et=' + endTime + '&ht=' + height;
// alert("Args : " + args);
//  debug_showStatus();
 window.status = 'Gather Radar: make_prod.php' + args;
 loadXMLDoc('make_prod.php' + args);
 return true;
}


var reload_time = 0;
var wait_time = 100;

var img_arr = new Array(50);

/************************************************************************************
*/

//
// timer_f actually perpetrates the animation - it also updates the page in
// realtime mode (if endTime is set to "now"). Niles.
//
function timer_f()
{

//
// Cope with the fact that we may have no images - Niles.
//

    if (nframes == 0){
      if (endTime == "now"){ // Realtime mode - no images - this is a real problem!
       alert("No images found in realtime - system may not be confgured correctly.");
      } else { // Archive mode - I bet the user has entered a time with no data -
      alert ("No data found for field : " + field + "\n" +
             "             zoom state : " +  zoom + "\n" +
             " for period ending " + endTime + " UTC" + "\n" +
             "with time range set to : " + timeRange + " seconds\n\n" +
             "REVERTING TO REALTIME OPERATION.");
//
// Go back to realtime mode.
// Reset all settings.
//
         field = "CTI_VEL";
         zoom = "REAL_FULL";
         timeRange = 1800;
         endTime = "now";
         height = 24;
         a_mode =  3;
         nframes = 1;
         cur_frame = -1;
         last_frame = -1;
         endTime = "now";
         var now = new Date();
         reload_time = now.getTime() + 60000;
         loadImageData();
         timer_f();
      }
     return true;
    }

    if (endTime == "now"){ // Realtime mode - do updates

      var now = new Date();
      if(now.getTime() > reload_time) {
        reload_time = now.getTime() + 60000; // 60 seconds
        loadImageData();
      }
    }

	if(a_mode == 1) {
	  cur_frame++;
	  if(cur_frame >= nframes ) { cur_frame = 0; }

      if(cur_frame == nframes -1) {
		 wait_time = 1000;
	  } else {
		 wait_time = 100;
	  }
	}

	if(a_mode == 2) {
	  a_mode = 0;
	  cur_frame = 0; 
	}

	if(a_mode == 3) {
	  a_mode = 0;
	  cur_frame = nframes -1; 
	}

	if(a_mode == 4) {
	  a_mode = 0;
	  cur_frame --;
	  if(cur_frame < 0 ) { cur_frame = 0; }
	}

	if(a_mode == 5) {
	  a_mode = 0;
	  cur_frame++;
	  if(cur_frame >= nframes ) { cur_frame = nframes -1; }
	}

	if(cur_frame != last_frame) {
	    //alert("Set Frame: " + cur_frame);
	    _el('Pan_img').src = img_arr[cur_frame].src;
		last_frame = cur_frame;
	}

    setTimeout("timer_f()",wait_time);
 return true;
}


function debug_showStatus(){
   alert (" ciddView internal status : \n" +
          "         endTime : " + endTime + "\n" +
          "         timeRange : " + timeRange + "\n" +
          "         Zoom : " + zoom + "\n" +
          "         Field : " + field + "\n" +
          "         height : " + height  + "\n" +
          "         a_mode : " + a_mode + "\n" +
          "         nframes : " + nframes +  "\n" +
          "         cur_frame : " + cur_frame +  "\n" +
          "         last_frame : " + last_frame);
   return true;
}


</SCRIPT>

</HTML>







