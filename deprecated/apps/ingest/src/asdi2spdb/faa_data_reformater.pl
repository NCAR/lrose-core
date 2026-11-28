#!/usr/bin/perl
# Jason Craig June 2005
#
# faa_data_reformater is used to reformat and parse the faa's data cd.
# Each output file starts with the number of lines within it.
#
# Fix: 1 Fix per line followed by the lat, lon
# SSD: 1 SSD route per line, where a ssd route is defined as a fix then a '.' 
#      then the ssd name. Following the name are waypoint names, lats, lons
# NAV: 1 Nav per line followed by the lat, lon
# AWY: 1 Awy route per line, where a awy route is the whole route, (waypoint
#      names, lats, lons),  decoding software must parse the line to determine
#      where a aircraft enters the awy and where it exits.
# APT: 1 Apt per line followed by the lat, lon, alt
#
# Notes:
#   Updated Aug 22nd for Sept 1st's new APT format
###############################################################################
#
# File names
#
###############################################################################
$input_dir = "/rap/data/faa_data_cdrom/";
$output_dir = "/d1/jcraig/asdi/";
$fix_file="FIX";
$nav_file="NAV";
$apt_file="APT";
$ssd_file="SSD";
$awy_file="AWY";

#print("INPUT FILES:\n");
#printf("\tFix File = %s\n", $fix_file);
#printf("\tNav File = %s\n", $nav_file);
#printf("\tApt File = %s\n", $apt_file);
#printf("\tSsd File = %s\n", $ssd_file);
#printf("\tAwy File = %s\n", $awy_file);
print("OUTPUT FILES:\n");

process_fix();
process_ssd();
process_nav();
process_awy();
process_apt();

exit 0;

###############################################################################
#
# Process the FIX file
#
###############################################################################
sub process_fix()
{
  $nFix = 0;
  open(FIX_IN, "${input_dir}${fix_file}.txt") || die "Can't Open Input Fix File: ${input_dir}${fix_file}.txt ($!)\n";
  open(FIX_OUT, ">${output_dir}${fix_file}.loc") || die "Can't Open Output Fix File: ${output_dir}${fix_file}.loc ($!)\n";
  print "Fix: ${output_dir}${fix_file}.loc\n";
  print FIX_OUT "     \n";
  while($line=<FIX_IN>) {
    # Location Identifier
    $id = substr($line, 0, 30);
    $id =~ tr/ //d;
    #Only do all lettered fixes 
    if($id =~ /^[A-Z]{5}$/ ) {

      # Latitude in DD-MM-SS.SSSX format
      $lat = substr($line, 50, 14);
      $lat =~ tr/ //d;
    
      # Longitude in DDD-MM-SS.SSSX format
      $lon = substr($line, 64, 14);
      $lon =~ tr/ //d;
      
      &calc_lat($lat, $dlat);
      &calc_lon($lon, $dlon);
      
      if($dlat eq 0 && $dlon eq 0) {
        next;
      }
      printf FIX_OUT ("%s %.6f %.6f\n", $id, $dlat, $dlon);
      $nFix++;
    }
  }
  close(FIX_IN);
  print "nfix = " . $nFix . "\n";
  seek FIX_OUT, 0, 0;
  print FIX_OUT $nFix . "\n";
  close(FIX_OUT);
}

###############################################################################
#
# Process the NAV file
#
###############################################################################
sub process_nav()
{
  $nNave = 0;
  open(NAV_IN, "${input_dir}${nav_file}.txt") || die "Can't Open Input Nav File: ${input_dir}${nav_file}.txt ($!)\n";
  open(NAV_OUT, ">${output_dir}${nav_file}.loc") || die "Can't Open Output Nav File: ${output_dir}${nav_file}.loc ($!)\n";
  print "Nav: ${output_dir}${nav_file}.loc\n";
  print NAV_OUT "    \n";
  while($line=<NAV_IN>) {

    # Skip lines not beginning with NAV1
    $type = substr($line, 0, 4);
    if($type ne "NAV1") {
      next;
    }

    # Location Identifier
    $id = substr($line, 28, 4);
    $id =~ tr/ //d;
    
    # Latitude in DD-MM-SS.SSSX format
    $lat = substr($line, 283, 14);
    $lat =~ tr/ //d;
    
    # Longitude in DDD-MM-SS.SSSX format
    $lon = substr($line, 308, 14);
    $lon =~ tr/ //d;
    
    &calc_lat($lat, $dlat);
    &calc_lon($lon, $dlon);
    
    if($dlat eq 0 && $dlon eq 0) {
      next;
    }
    printf NAV_OUT ("%s %.6f %.6f\n", $id, $dlat, $dlon);
    $nNav++;
  }
  close(NAV_IN);
  print "nNav = " . $nNav . "\n";
  seek NAV_OUT, 0, 0;
  print NAV_OUT $nNav . "\n";
  close(NAV_OUT);
}

###############################################################################
#
# Process the APT file
#
###############################################################################
sub process_apt()
{
  $nApt = 0;
  open(APT_IN, "${input_dir}${apt_file}.txt") || die "Can't Open Input Apt File: ${input_dir}${apt_file}.txt ($!)\n";
  open(APT_OUT, ">${output_dir}${apt_file}.loc") || die "Can't Open Output Apt File: ${output_dir}${apt_file}.loc ($!)\n";
  print "Apt: ${output_dir}${apt_file}.loc\n";
  print APT_OUT "     \n";
  while($line=<APT_IN>) {

    # Skip lines that are not airports
    $type = substr($line, 0, 3);
    if($type ne "APT") {
      next;
    }

    # Airport types
    if($type eq "APT") {
      # Location Identifier
      $id = substr($line, 27, 4);
      $id =~ tr/ //d;

      # Latitude in DD-MM-SS.SSSX format
      $lat = substr($line, 515, 15);     #507(Sept 1)

      # Longitude in DDD-MM-SS.SSSX format
      $lon = substr($line, 542, 15);     #534(Sept 1)

      # Elevation in feet
      $elev = substr($line, 570, 5);     #562(Sept 1)

      $elev =~ tr/ //d;
    }
    # Runway types
    elsif($type eq "RWY") { 
      #These numbers are now wrong for rwy as of Sept 1
      # Location Identifier
      $id = substr($line, 14, 7);
      $id =~ tr/ //d;

      # Latitude in DD-MM-SS.SSSX format
      # Latitude of Physical Runway End
      $lat = substr($line, 94, 15);

      # Longitude in DDD-MM-SS.SSSX format
      # Longitude of Physical Runway End
      $lon = substr($line, 121, 15);

      # Elevation in feet of Physical Runway End
      $elev = substr($line, 148, 7);
      $elev =~ tr/ //d;
    }
    # Invalid types
    else {
      print "Error - Invalid type = $type";
      die;
    }


    &calc_lat($lat, $dlat);
    &calc_lon($lon, $dlon);

    printf APT_OUT ("%s %.6f %.6f %d\n", $id, $dlat, $dlon, $elev);
    $nApt++;
  }
  close(APT_IN);
  print "nApt = " . $nApt . "\n";
  seek APT_OUT, 0, 0;
  print APT_OUT $nApt . "\n";
  close(APT_OUT);
}

###############################################################################
#
# Process the SSD file
#
###############################################################################
sub process_ssd()
{
  $nSsd = 0;
  open(SSD_IN, "${input_dir}${ssd_file}.txt") || die "Can't Open Input SSD File: ${input_dir}${ssd_file}.txt ($!)\n";
  open(SSD_OUT, ">${output_dir}${ssd_file}.loc") || die "Can't Open Output SSD File: ${output_dir}${ssd_file}.loc ($!)\n";
  print "Ssd: ${output_dir}${ssd_file}.loc\n";
  print SSD_OUT "    ";
  $last_computer_code = 'S0000';
  while($line=<SSD_IN>) {
    # Internal Sequence Number, used for seperating routes
    $computer_code = substr($line, 0, 5);
    # Sid Computer Code
    $id = substr($line, 36, 13);
    $id =~ tr/ //d;
    # Type code, fix or NavAid or Airport
    $type_code = substr($line, 10, 2);
    $type_code =~ tr/ //d;
    # Fix/NavAid/Airport Identifier
    $fix = substr($line, 30, 6);
    $fix =~ tr/ //d;
    if($id eq '' and $fix =~ /([0-9])/) {
	next;
    }
    # Lat of Fix
    $lat = substr($line, 13, 8);
    $lat =~ tr/ //d;
    # Lon of Fix
    $lon = substr($line, 21, 9);
    $lon =~ tr/ //d;
    &calc_lat_2($lat, $dlat);
    &calc_lon_2($lon, $dlon);
    if(substr($computer_code, 0, 1) eq 'S') { # Star Route
      if($last_computer_code ne $computer_code && $id eq "") {
	print "Error1 with line '", $line, "'\n";
	exit 1;
      } elsif($last_computer_code ne $computer_code) {
	$nSsd++;
	if($add_basic_star == 1) {
	  if($basic_star_str ne '') {
	    printf SSD_OUT (" %s\n", $basic_star_str);
	  } else {
	    print SSD_OUT "\n";
	  }
	  $add_basic_star = 0;
	} else {
	  print SSD_OUT "\n";
	}
	$last_computer_code = $computer_code;
	$in_basic_star = 1;
	($basic_transition, $basic_star) = split /\./, $id;
	if($type_code eq 'AA') {
	  print "Error2 with line '", $line, "'\n";
	  exit 1;
	}
	printf SSD_OUT ("%s %s %.6f %.6f", $id, $fix, $dlat, $dlon);
	$basic_star_str = '';
      } elsif($id eq "") {
	if($in_basic_star == 1) {
	    $type_code = substr($line, 10, 2);
	    $type_code =~ tr/ //d;
	    if($type_code eq 'AA') {
		next;
	    }
	    $fix = substr($line, 30, 6);
	    $fix =~ tr/ //d;
	    printf SSD_OUT (" %s %.6f %.6f", $fix, $dlat, $dlon);
	    if($basic_star_str eq '') {
		$basic_star_str = sprintf ("%s %.6f %.6f", $fix, $dlat, $dlon);
	    } else {
		$basic_star_str = $basic_star_str . sprintf (" %s %.6f %.6f", $fix, $dlat, $dlon);
	    }
	} else {
	    $type_code = substr($line, 10, 2);
	    $type_code =~ tr/ //d;
	    if($type_code eq 'AA') {
		next;
	    }
	    $fix = substr($line, 30, 6);
	    $fix =~ tr/ //d;
	    printf SSD_OUT (" %s %.6f %.6f", $fix, $dlat, $dlon);
	}
      } else {
	  $nSsd++;
	  if($add_basic_star == 1) {
	    if($basic_star_str ne '') {
	      printf SSD_OUT (" %s\n", $basic_star_str);
	    } else {
	      print SSD_OUT "\n";
	    }
	    $add_basic_star = 0;
	  } else {
	    print SSD_OUT "\n";
	  }
	  $in_basic_star = 0;
	  $add_basic_star = 1;
	  printf SSD_OUT ("%s %s %.6f %.6f", $id, $fix, $dlat, $dlon);
      }
    } elsif(substr($computer_code, 0, 1) eq 'D') { # Sid Route
      if($last_computer_code ne $computer_code && $id eq "") {
	print "Error1 with line '", $line, "'\n";
	exit 1;
      } elsif($last_computer_code ne $computer_code) {
	$nSsd++;
	$last_computer_code = $computer_code;
	$in_basic_sid = 1;
	($basic_sid, $basic_transition) = split /\./, $id;
	if($type_code eq 'AA') {
	  print "Error2 with line '", $line, "'\n";
	  exit 1;
	}
	printf SSD_OUT ("\n%s %s %.6f %.6f", $id, $fix, $dlat, $dlon);
	if($fix eq $basic_transition) {
	  $basic_sid_str = '';
	  next;
	}
	$basic_sid_str = sprintf ("%s %.6f %.6f", $fix, $dlat, $dlon);
      } elsif($id eq "") {
	if($in_basic_sid == 1) {
	    $type_code = substr($line, 10, 2);
	    $type_code =~ tr/ //d;
	    if($type_code eq 'AA') {
		next;
	    }
	    $fix = substr($line, 30, 6);
	    $fix =~ tr/ //d;
	    printf SSD_OUT (" %s %.6f %.6f", $fix, $dlat, $dlon);
	    if($fix eq $basic_transition) {
		next;
	    }
	    $basic_sid_str = $basic_sid_str . sprintf (" %s %.6f %.6f", $fix, $dlat, $dlon);
	} else {
	    $type_code = substr($line, 10, 2);
	    $type_code =~ tr/ //d;
	    if($type_code eq 'AA') {
		next;
	    }
	    $fix = substr($line, 30, 6);
	    $fix =~ tr/ //d;
	    printf SSD_OUT (" %s %.6f %.6f", $fix, $dlat, $dlon);
	}
      } else {
	  $nSsd++;
	  $in_basic_sid = 0;
#	  if($fix ne $basic_transition) {
#	      print "Error3 with line '", $fix, ",", $basic_transition, ",", $line, "'\n";
#	      exit 1;
#	  }
#	  print $basic_sid_str, "\n";
	  if($basic_sid_str eq '') {
	    printf SSD_OUT ("\n%s %s %.6f %.6f", $id, $fix, $dlat, $dlon);
	  } else {
	    printf SSD_OUT ("\n%s %s %s %.6f %.6f", $id, $basic_sid_str, $fix, $dlat, $dlon);
	  }
      }
    } else { # Not a Sid or a STAR
	print "Error with line '", $line, "'\n";
	exit 1;
    }
  } # End While
  close(SSD_IN);
  $nSsd--;
  print "nSsd = " . $nSsd . "\n";
  seek SSD_OUT, 0, 0;
  print SSD_OUT $nSsd . "\n";
  close(SSD_OUT);
}

###############################################################################
#
# Process the AWY file
#
###############################################################################
sub process_awy()
{
  $nAwy = 0;
  open(AWY_IN, "${input_dir}${awy_file}.txt") || die "Can't Open Input Awy File: ${input_dir}${awy_file}.txt ($!)\n";
  open(AWY_OUT, ">${output_dir}${awy_file}.loc") || die "Can't Open Output Awy File: ${output_dir}${awy_file}.loc ($!)\n";
  print "Awy: ${output_dir}${awy_file}.loc\n";
  print AWY_OUT "    ";
  $last_awy = 'J000';
  while($line=<AWY_IN>) {

    # Skip lines not beginning with AWY2
    # AWY2 are the airway point descriptors
    $type = substr($line, 0, 4);
    if($type ne "AWY2") {
      next;
    }
    
    # Airway Designation (Jet/Victor Route Number)
    $awy = substr($line, 4, 5);
    $awy =~ tr/ //d;

    # Navid/Fix Name
    $name = substr($line, 15, 30);
    $name =~ tr/ //d;
    if(length($name) > 14) {
	if($name eq 'U.S.CANADIANBORDER') {
	    next;
	    $name = 'USCANADABORDER';
	} elsif($name eq 'U.S.MEXICANBORDER') {
	    next;
	    $name = 'USMEXICOBORDER';
	} else {
	    $name = substr($name, 14);
	}
    }

    # Nav ID Identifier
    $id = substr($line, 114, 4);
    $id =~ tr/ //d;

    if($id eq '' and $name =~ /([0-9])/) {
	next;
    }

    # Latitude in DD-MM-SS.S[SS]X format
    $lat = substr($line, 81, 14);
    $lat =~ tr/ //d;

    # Longitude in DDD-MM-SS.S[SS]X format
    $lon = substr($line, 95, 14);
    $lon =~ tr/ //d;

    &calc_lat($lat, $dlat);
    &calc_lon($lon, $dlon);

    if($awy ne $last_awy) {
      $nAwy++;
      $last_awy = $awy;
      if($id eq '') {
	printf AWY_OUT ("\n%s %s %.6f %.6f", $awy, $name, $dlat, $dlon);
      } else {
        printf AWY_OUT ("\n%s %s %.6f %.6f", $awy, $id, $dlat, $dlon);
      }
    } else {
      if($id eq '') {
	printf AWY_OUT (" %s %.6f %.6f", $name, $dlat, $dlon);
      } else {
        printf AWY_OUT (" %s %.6f %.6f", $id, $dlat, $dlon);
      }
    }
  }
  close(AWY_IN);
  $nAwy--;
  print "nAwy = " . $nAwy . "\n";
  seek AWY_OUT, 0, 0;
  if($nAwy < 1000) {
      print AWY_OUT $nAwy . " \n";
  } else {
      print AWY_OUT $nAwy . "\n";
  }
  close(AWY_OUT);
}


###############################################################################
#
# Subroutines to reformat the lat/lon strings
#
###############################################################################

# Latitude in DD-MM-SS.SSSX format
sub calc_lat {
   $dir_loc = length($_[0]);

   $deg = substr($_[0], 0, 2);
   $min = substr($_[0], 3, 2);
   $sec = substr($_[0], 6, $dir_loc - 6 - 1);
   $dir = substr($_[0], $dir_loc-1, 1);
   $_[1] = $deg + ($min + ($sec/60))/60;

   if($dir eq "S") {
      $_[1] = $_[1] * -1;
   }
}

# Latitude in XDDDMMSST format
sub calc_lat_2 {
   $dir = substr($_[0], 0, 1);
   $deg = substr($_[0], 1, 2);
   $min = substr($_[0], 3, 2);
   $sec = substr($_[0], 5, 2);
   $tenth = substr($_[0], 7, 1);
   $sec = "$sec.$tenth";
   $_[1] = $deg + ($min + ($sec/60))/60;
   if($dir eq "S") {
      $_[1] = $_[1] * -1;
   }
}

sub calc_lon {

   # Delete any excess spaces
   $_[0] =~ tr/ //d;

   $dir_loc = length($_[0]);
   # Longitude in DDD-MM-SS.SSSX format
   $deg = substr($_[0], 0, 3);
   $min = substr($_[0], 4, 2);
   $sec = substr($_[0], 7, $dir_loc - 7 - 1);
   $dir = substr($_[0], $dir_loc-1, 1);
   $_[1] = $deg + ($min + ($sec/60))/60;

   if($dir eq "W") {
      $_[1] = $_[1] * -1;
   }

}

# Longitude in XDDDMMSST format
sub calc_lon_2 {
   $dir = substr($_[0], 0, 1);
   $deg = substr($_[0], 1, 3);
   $min = substr($_[0], 4, 2);
   $sec = substr($_[0], 6, 2);
   $tenth = substr($_[0], 8, 1);
   $sec = "$sec.$tenth";
   $_[1] = $deg + ($min + ($sec/60))/60;
   if($dir eq "W") {
      $_[1] = $_[1] * -1;
   }
}
