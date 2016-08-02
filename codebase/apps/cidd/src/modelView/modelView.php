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

if(empty($fl))$fl="850htt";
if(empty($dm))$dm="d1";
if(empty($tr))$tr=43200;

$dir = "images";

$fname = get_best_file($dm, $fl, $dir, $tr, '2006-01-06_23:00:00');
$pathname = "{$dir}/{$fname}";

?>

<!-- ******************************************************************** -->


<HEAD>
  <TITLE>modelView</TITLE>
  <META http-equiv="Content-Type" content="text/html; charset=iso-8859-1" />
  <LINK type="text/css" href="css/modelView.css" media="screen" rel="stylesheet" title="Default" />

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
                  <option value=21600    <?php if($tr == 21600)  echo "selected"?> >   6 Hours </option>
                  <option value=43200    <?php if($tr == 43200)  echo "selected"?> >  12 Hours </option>
                  <option value=86400    <?php if($tr == 86400)  echo "selected"?> >  24 Hours </option>
                  <option value=172800   <?php if($tr == 172800) echo "selected"?> >  48 Hours </option>
                </SELECT>
   </DIV>  <!-- choice_item -->


   <!-- FIELD SELECT - Fairly self-explanitory - the names line up with what the model uses in its filenames. Niles. -->
   <DIV CLASS="choice_item">
     <h3> Field </h3>
                <SELECT name="field"  onChange="return set_field()">
                  <option value="850htt"         <?php if($fl == "850htt")          echo "selected"?> > 850htt </option>
                  <option value="2mtempA10mwnd"  <?php if($fl == "2mtempA10mwnd")   echo "selected"?> > 2mtempA10mwnd </option>
                </SELECT>
   </DIV>  <!-- choice_item -->

   <!-- DOMAIN SELECT - again, the names line up with what the model uses in its filenames. Niles. -->
   <DIV CLASS="choice_item">
     <h3> View </h3>
                <SELECT name="domain"  onChange="return set_domain()">
                  <option value="d1"     <?php if($dm == "d1")    echo "selected"?> > d1 </option>
                  <option value="d2"     <?php if($dm == "d2")    echo "selected"?> > d2 </option>
                  <option value="d3"     <?php if($dm == "d3")    echo "selected"?> > d3 </option>
                  <option value="d4"     <?php if($dm == "d4")    echo "selected"?> > d4 </option>
                </SELECT>
   </DIV>  <!-- choice_item -->

   <!-- END TIME SELECT - a textbox that lets the user "movie back" in time. The Janitor should save images
                          pertaining to things on the eventlist, if there is one, so this allows the user to
                          run modelView in archive mode. Niles. -->
   <DIV CLASS="choice_item">
     <h3> End time YYYY-MM-DD_hh:mm:ss </h3>
                <INPUT type="text" size=25 name="endTime" value = "2006-01-06_23:00:00"
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
 Entering "now" for data end time invokes realtime operation<BR>
 <HR>
 This is an experimental site created by Frank Hage and currently under Development by Niles Oien, oien@ucar.edu<BR>
 Copyright 2006 UCAR,RAL. BSD-sylte license applies. <BR>
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

var field = "850htt";
var domain = "d1";
var timeRange = 43200;
var endTime = "2006-01-06_23:00:00";

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

function set_domain() {
 domain = document.choiceform.domain.options[document.choiceform.domain.selectedIndex].value;
 loadImageData();
 return true;
}

function set_timeRange() {
 timeRange = document.choiceform.timeRange.options[document.choiceform.timeRange.selectedIndex].value;
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
 var args = '?fl=' + field + '&dm=' + domain + '&tr=' + timeRange + '&et=' + endTime;
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
             "             domain : " +  domain + "\n" +
             " for period ending " + endTime + " UTC" + "\n" +
             "with time range set to : " + timeRange + " seconds\n\n" +
             "REVERTING TO REALTIME OPERATION.");
//
// Go back to realtime mode.
// Reset all settings.
//
         field = "850htt";
         domain = "d1";
         timeRange = 43200;
         endTime = "2006-01-06_23:00:00";
         a_mode =  3;
         nframes = 1;
         cur_frame = -1;
         last_frame = -1;
         var now = new Date();
         reload_time = now.getTime() + 60000;
         loadImageData();
         timer_f();
      }
     return true;
    }

    if ((reload_time == 0) || (endTime == "now")){ // Realtime mode - do updates

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
   alert (" modelView internal status : \n" +
          "         endTime : " + endTime + "\n" +
          "         timeRange : " + timeRange + "\n" +
          "         Domain : " + domain + "\n" +
          "         Field : " + field + "\n" +
          "         a_mode : " + a_mode + "\n" +
          "         nframes : " + nframes +  "\n" +
          "         cur_frame : " + cur_frame +  "\n" +
          "         last_frame : " + last_frame);
   return true;
}


</SCRIPT>

</HTML>







