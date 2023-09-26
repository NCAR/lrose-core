<!-------------------------------------------------------------------

  AOAWS Cross Section PHP SCRIPT - Present a form for selecting
  predefined Cross sections

-------------------------------------------------------------------->
<HTML>
<BODY BGCOLOR="#FFFFFF">

<?php

  // Compute the time
  $t1 = time();
  $utcstr =  gmdate("H:i M d");  
  $gmtstr =  gmdate("D, j M Y  H:i:s",$t1); 
  $dstr =  gmdate("ymdHis",$t1); 
  $locstr =  date("H:i M d");

  // Build unique names for Request and Output Images
  $pid = getmypid();
  $cmd_fname = "/tmp/AOAWS_XSECT_REQ_$pid";
  $im_fname = "AOAWS_XSECT_IM_$pid" . "_" .  $dstr;
  $png_im_fname = "/home/fhage/public_html/cidd_images/" . $im_fname . ".png";
  $png_im_url = "http://oberon/~fhage/cidd_images/" . $im_fname . ".png";

  $him_fname = "AOAWS_PLAN_IM_$pid" . "_" .  $dstr;
  $png_him_fname = "/home/fhage/public_html/cidd_images/" . $him_fname . ".png";
  $png_him_url = "http://oberon/~fhage/cidd_images/" . $him_fname . ".png";

  // CIDD's Input Command Queue File name
  $fmq_name = "/tmp/remoteUI";

  if($request_time == "Now") {
     $rtime = $t1;
  } else {
     $rtime = intval($request_time) * 3600 + $t1;
  }

  // Process form - - Submit commands,  put image in line when done
  if ($REQUEST_METHOD=='POST') {
    echo "<HEAD> <TITLE>AOAWS Route Cross Section Results</TITLE> </HEAD>";
    echo "<CENTER><H2>AOAWS Route Cross Section Results</H2></CENTER><BR>";
    echo "<H4> Image Returned at $utcstr  UTC</H4> ";

    $fp = fopen($cmd_fname,"w");

    fputs($fp,"# COMMANDS from AOAWS Cross Section PHP SCRIPT 1.0\n");
    fputs($fp,"SET_REQUEST_TIME $rtime\n");
    fputs($fp,"SELECT_V_PAGE $data_field\n");
    fputs($fp,"SELECT_ROUTE $route\n");
    fputs($fp,"SET_ALTITUDE $altitude\n");
    fputs($fp,"SET_V_IMAGE_NAME $im_fname\n");
    fputs($fp,"DUMP_V_IMAGE\n");

    if($include_plan) {
      fputs($fp,"SELECT_H_PAGE $data_field\n");
      fputs($fp,"SET_H_IMAGE_NAME $him_fname\n");
      fputs($fp,"DUMP_H_IMAGE\n");
    }

    fclose($fp);

    system("/home/fhage/cvs/bin/RemoteUI2Fmq $cmd_fname $fmq_name");

    // Remove the command file
    unlink($cmd_fname);

    usleep(500000); // Wait 500 miliseconds

    if($include_plan) {
        $count = 0;
        $time_out = 0;

        // Wait for file to show up - Check 10 times/sec
        while(!file_exists($png_him_fname) && ($time_out == 0)) {
         if($count++ >= 120) $time_out = 1;
         usleep(100000);
        }

        if( $time_out) {
           echo "<H3>Response From Image Server Timed out - Sorry </H3>";
           echo "Looking for" . $png_him_fname;
        } else {
           echo "<CENTER> <IMG ALIGN=CENTER VALIGN=TOP SRC=\"$png_him_url\"> <BR></CENTER><P>\n";
        }
    }
     
    $count = 0;
    $time_out = 0;

    // Wait for file to show up - Check 10 times/sec
    while(!file_exists($png_im_fname) && ($time_out == 0)) {
     if($count++ >= 20) $time_out = 1;
     usleep(100000);
    }

    if( $time_out) {
       echo "<H3> Response From Image Server Timed out - Sorry </H3>";
       echo "Looking for" . $png_im_fname;
    } else {
       echo "<CENTER> <IMG ALIGN=CENTER VALIGN=TOP SRC=\"$png_im_url\"> <BR></CENTER>\n";
    }


  } else {  // Code Only  for original
    $route = "A-1";
    $altitude = "220";
    $request_time = "Now";
    $data_field = "Wind_Speed";

    echo "<HEAD> <TITLE>AOAWS Route Cross Section Prototype Request Form</TITLE> </HEAD>";
    echo "<CENTER><H2>AOAWS Route Cross Section</H2></CENTER><BR>";
    echo "<H4> Page Loaded at:  $utcstr  UTC</H4> ";

  } // end if ($submit)
?>


<!----------   The Selection table shows up always  ------------>



<FORM method="POST" action="<?php echo $PHP_SELF?>">

<TABLE BORDER=0 ALIGN=CENTER CELLPADDING=2 CELLSPACING=0>
    <TR> 
      <TD ALIGN=CENTER> <H4> Route </H4> </TD>
      <TD ALIGN=CENTER> <H4> Flight Level </H4> </TD>
      <TD ALIGN=CENTER> <H4> Time </H4> </TD>
      <TD ALIGN=CENTER> <H4> Data </H4> </TD>
      <TD>
	  <INPUT  TYPE=checkbox NAME=include_plan value="1"
	      <?php if($include_plan == 1) echo "checked"?>>
	      Include Plan View?
      </TD>
    </TR>

    <TR>
      <TD ALIGN=CENTER>
        <SELECT name="route" >
	  <option <?php if($route == "A-1") echo "selected"?>>A-1 
	  <option <?php if($route == "M-750") echo "selected"?>>M-750
	  <option <?php if($route == "G-581") echo "selected"?>>G-581
	  <option <?php if($route == "B-591") echo "selected"?>>B-591
	  <option <?php if($route == "R-583") echo "selected"?>>R-583
	  <option <?php if($route == "G-86") echo "selected"?>>G-86
	  <option <?php if($route == "B-576") echo "selected"?>>B-576
	</SELECT>
      </TD>

      <TD ALIGN=CENTER>
        <SELECT name="altitude" >
	  <option <?php if($altitude == 010) echo "selected"?>>010
	  <option <?php if($altitude == 020) echo "selected"?>>020
	  <option <?php if($altitude == 030) echo "selected"?>>030
	  <option <?php if($altitude == 040) echo "selected"?>>040
	  <option <?php if($altitude == 050) echo "selected"?>>050
	  <option <?php if($altitude == 060) echo "selected"?>>060
	  <option <?php if($altitude == 070) echo "selected"?>>070
	  <option <?php if($altitude == 080) echo "selected"?>>080
	  <option <?php if($altitude == 090) echo "selected"?>>090
	  <option <?php if($altitude == 100) echo "selected"?>>100
	  <option <?php if($altitude == 110) echo "selected"?>>110
	  <option <?php if($altitude == 120) echo "selected"?>>120
	  <option <?php if($altitude == 130) echo "selected"?>>130
	  <option <?php if($altitude == 140) echo "selected"?>>140
	  <option <?php if($altitude == 150) echo "selected"?>>150
	  <option <?php if($altitude == 160) echo "selected"?>>160
	  <option <?php if($altitude == 170) echo "selected"?>>170
	  <option <?php if($altitude == 180) echo "selected"?>>180
	  <option <?php if($altitude == 190) echo "selected"?>>190
	  <option <?php if($altitude == 200) echo "selected"?>>200
	  <option <?php if($altitude == 210) echo "selected"?>>210
	  <option <?php if($altitude == 220) echo "selected"?>>220
	  <option <?php if($altitude == 230) echo "selected"?>>230
	  <option <?php if($altitude == 240) echo "selected"?>>240
	  <option <?php if($altitude == 250) echo "selected"?>>250
	  <option <?php if($altitude == 260) echo "selected"?>>260
	  <option <?php if($altitude == 270) echo "selected"?>>270
	  <option <?php if($altitude == 280) echo "selected"?>>280
	  <option <?php if($altitude == 290) echo "selected"?>>290
	  <option <?php if($altitude == 310) echo "selected"?>>310
	  <option <?php if($altitude == 330) echo "selected"?>>330
	  <option <?php if($altitude == 350) echo "selected"?>>350
	  <option <?php if($altitude == 370) echo "selected"?>>370
	  <option <?php if($altitude == 390) echo "selected"?>>390
	  <option <?php if($altitude == 410) echo "selected"?>>410
	  <option <?php if($altitude == 430) echo "selected"?>>430
	  <option <?php if($altitude == 450) echo "selected"?>>450
	</SELECT>
      </TD>

      <TD ALIGN=CENTER>
        <SELECT name="request_time">
	  <option <?php if($request_time == "Now") echo "selected"?>>Now
	  <option <?php if($request_time == "+2 Hrs") echo "selected"?>>+2 Hrs
	  <option <?php if($request_time == "+4 Hrs") echo "selected"?>>+4 Hrs
	  <option <?php if($request_time == "+6 Hrs") echo "selected"?>>+6 Hrs
	  <option <?php if($request_time == "+8 Hrs") echo "selected"?>>+8 Hrs
	  <option <?php if($request_time == "+10 Hrs") echo "selected"?>>+10 Hrs
	  <option <?php if($request_time == "+12 Hrs") echo "selected"?>>+12 Hrs
	  <option <?php if($request_time == "+16 Hrs") echo "selected"?>>+16 Hrs
	  <option <?php if($request_time == "+20 Hrs") echo "selected"?>>+20 Hrs
	  <option <?php if($request_time == "+24 Hrs") echo "selected"?>>+24 Hrs
	</SELECT>
      </TD>

      <TD ALIGN=CENTER>
        <SELECT name="data_field" value="<?php echo $data_field?>">
	  <option <?php if($data_field == "Water_Vapour") echo "selected"?>>Water_Vapour
	  <option <?php if($data_field == "Wind_Speed") echo "selected"?>>Wind_Speed
	  <option <?php if($data_field == "Freezing_Level") echo "selected"?>>Freezing_Level
	  <option <?php if($data_field == "Humidity") echo "selected"?>>Humidity
	  <option <?php if($data_field == "Turb_prob") echo "selected"?>>Turb_prob
	  <option <?php if($data_field == "Icing") echo "selected"?>>Icing
	  <option <?php if($data_field == "Jet_Wind_Speed") echo "selected"?>>Jet_Wind_Speed
	  <option <?php if($data_field == "Temperature") echo "selected"?>>Temperature
	</SELECT>
      </TD>

       <TD> <INPUT type="submit" value="Submit Request" > </TD>
    </TR>
  </TABLE>
</FORM>   

<BR clear=all>
<! -- Directions   --->

<HR>
<OL> <B> <FONT SIZE=+1> Directions: </B></FONT>
<LI>Choose Route
<LI>Choose Route Altitude
<LI>Choose Time of Interest
<LI>Choose Data Field 
<LI>Press Submit.
</OL>

</BODY>
</HTML>
