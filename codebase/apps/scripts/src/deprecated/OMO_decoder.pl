#!/usr/bin/perl
##########################################################
#
# This script decodes OMO formatted data downloaded from WMSCR
# and outputs delimited ASCII data containing ASOS, AWOS, 
# and AOS station reports
#
##########################################################

($prog) = ( $0 =~ /.*\/(.+)$/ );

# Change if needed
$delimiter = ",";

$usage=<<EOF;
   Usage: $prog <omo_file> [<log_file>]
        <log_file> is optional.
        Decoded OMOs are printed to STDOUT
EOF

($infile, $error_file) = @ARGV;

if (!$infile) { die $usage; }

open (IN, $infile) or die "Could not open '$infile': $!\n$usage";

$log_errors = 0;
if ($error_file) {
    $log_errors = 1;
    open (ERROR, ">>$error_file") or die "Could not write to log file $error_file: $!\n";
}    


$header = <<EOF;
##########################################
# OMO data downloaded from WMSCR.
#
# 1)  Date (UTC)
# 2)  Time (UTC)
# 3)  Station ID
# 4)  Federally purchased and maintained? (Y/N)
# 5)  Units for temperature and dewpoint (C/F)
# 6)  Company code
# 7)  Hardware code
# 8)  Software code or version
# 9)  Alerts
# 10) Cloud height 1 (feet)
# 11) Cloud amount 1
# 12) Cloud height 2 (feet)
# 13) Cloud amount 2
# 14) Cloud height 3 (feet)
# 15) Cloud amount 3
# 16) Visibility (statute miles)
# 17) Obscurations to vision
# 18) Precipitation accumulation (in)
# 19) Precipitation type
# 20) Ambient Temperature
# 21) Dew point temperature
# 22) Wind direction true
# 23) Wind direction magnetic
# 24) Wind speed average (kn)
# 25) Wind speed gust (kn)
# 26) Pressure (inHg)
# 27) Density Altitude over 1000ft (ft)
# 28) Sea Level Pressure (mb)
# 29) RVR Runway ID (deg)
# 30) RVR (ft)
# 31) RVR parallel runway
# 32) RVR high/low flag
# 33) Supplementary Obscurations and Precipitation Types
# 34) Lightning Activity
# 35) Site Status Data
# 36) Wind Direction Sensor Status
# 37) Wind Speed Sensor Status
# 38) Ambient Temperature Sensor Status
# 39) Dew Point Temperature Sensor Status
# 40) Pressure Sensor Status
# 41) Ceiling Height Indicator Sensor Status
# 42) Precipitation Occurrence/Type Sensor Status
# 43) Precipitation Accumulation Sensor Status
# 44) Visibility Sensor Status
# 45) Lightning Sensor Status
# 46) Freezing Rain Sensor Status
# 47) RVR Sensor Status
# 48) Parameter Activation Status
# 49) Automated Remarks Data
# 50) Optional Remarks
# Date, Time, ID, federally_owned, type, tempdew_units, company_code, hardware_code, software_version, alerts, cloud_height1, cloud_amount1, cloud_height2, cloud_amount2, cloud_height3, cloud_amount3, vis, obscurations, paccum, ptype, ambient_temp, dew_pt, wdir_true, wdir_mag, wspd_ave, wspd_gust, pressure, density_altitude, sealv_pressure, rvr_runway, rvr, rvr_parallel, rvr_flag, sup_obscurations, lightning, site_status, wind_dir_status, wind_spd_status, temp_status, dew_pt_status, pressure_status, ceil_status, precip_type_status, precip_accum_status, vis_status, lightning_status, fzra_status, rvr_status, param_activation_status, auto_remarks_data, remark
EOF

# change to configured delimiter
$header =~ s/, /$delimiter/g;

print $header;

# read the file as a byte stream
binmode IN;
local $/ = \1;

$num_parsed = 0;
$num_errors = 0;
$wmo_length = 0;
$last_id = "";

if ($log_errors) {
    print ERROR "Processing file '$infile'\n";
}

# Go through each WMO section 
while (my $byte = <IN>) {

    # look for the zero indicating the end of the
    # WMO header
    $byte = ord($byte);
    $wmo_length++;
    if ($byte != 0) { next; }
    
    # next byte is the number of reports in this section
    $num_reports = ord(<IN>);

    # read and decode each report
    for ($i=0;$i<$num_reports; $i++) {
        $length = ord(<IN>);        
        $id = <IN>.<IN>.<IN>.<IN>;

        # OMOs are supposed to be at least 68 bytes, so why
        # are there shorter ones in there?!?
        # also filter out bogus stations
        #if ($length < 68) { 
        $shifted_byte = 0;
        if ($length < 68 || !($id =~ /^[A-Z|0-9]+$/)) { 
            if ($log_errors) {
                print ERROR "    ERROR: OMO length $length after id '$last_id'\n";
            }

            $parseok = 0;
            # HACK - try to account for one more OR one less byte in the previous record
            if (!($id =~ /^[A-Z|0-9]+$/)) {
                if ($log_errors) {
                    print ERROR "    ERROR: Can't parse id '$id'\n";
                }
                # check for a missing byte
                $newid = chr($length).substr($id,0,3);
                if ($newid =~ /^[A-Z|0-9]+$/) {
                    $parseok = 1;
                    $shifted_byte = substr($id,-1);
                    $id = $newid;
                    $length = ord($lastbyte);
                }
                # check for extra byte
                if (!$parseok) {
                    $newid = substr($id,-3).<IN>;
                    if ($newid =~ /^[A-Z|0-9]+$/) {
                        $parseok = 1;
                        $length = ord (substr($id,0,1));
                        $id = $newid;
                    }
                }
                # if the extra byte didn't help, I guess assume it is part of this packet...?
                if (!$parseok) {
                    $length -= 1; # since we shifted an extra byte off the stream to try to get a valid id.
                } elsif ($log_errors) {
                    print ERROR "FIXED! new id is '$id'\n";
                }

            }

            if (!$parseok) {
                $num_errors++;
                for ($j = 0; $j < $length; $j++) {
                    $byte = <IN>;
                }
                next;
            } 
        }

        ##### site config #######
        if ($shifted_byte) {
            $config = ord($shifted_byte)*256 + ord(<IN>);
        }
        else {
            $config = ord(<IN>)*256 + ord(<IN>);
        }
        
        # F part
        $f = $config >> 15;
        $federal = ($f) ? "Y": "N";
        # A part
        $a = ($config >> 13) & 0b11;
        if ($a == 1) {$type = "ASOS"; }
        elsif ($a == 2) { $type = "AWOS"; }
        elsif ($a == 3) { $type = "AOS"; }
        else {$a = "UNDEF"; }
        # M part
        $m = ($config >> 9) & 0b1111;
        $units = ($m) ? "C" : "F";
        if ($type eq "AWOS") {
            # C part
            $c = ($config >> 6) & 0b111;
            $company = $c;
            # H part
            $h = ($config >> 3) & 0b111;
            $hardware = $h;        
            # S part
            $s = $config & 0b111;
            $software = $s;
        } else {
            $sw = $config & 0b111111111;
            $software = (240+$sw)/100.0;
            $company = "NA";
            $hardware = "NA";
        }

        ##### Date and time #######
        # TODO need to update this in 2100 :P
        $year = ord(<IN>) + 2000;
        $month = ord(<IN>);
        $day = ord(<IN>);
        $hour = ord(<IN>);
        $minute = ord(<IN>);

        $date = "$year".sprintf("%02d", $month).sprintf("%02d",$day);
        $time = sprintf("%02d",$hour).":".sprintf("%02d",$minute).":00";

        ##### Alert data ##########
        $alert_flags = ( ord(<IN>) << 24 ) | ( ord(<IN>) << 16 ) | ( ord(<IN>) << 8 ) | ( ord(<IN>) );
        $alerts = getAlertString($alert_flags);

        ################### Data Fields ###########################
        
        # cloud height and amount 
        $cloudheight1 = ord(<IN>)*100;
        $cloudamountflags1 = ord(<IN>);
        $cloudamount1 = getCloudString($cloudamountflags1);
        $cloudheight2 = ord(<IN>)*100;
        $cloudamountflags2 = ord(<IN>);
        $cloudamount2 = getCloudString($cloudamountflags2);
        $cloudheight3 = ord(<IN>)*100;
        $cloudamountflags3 = ord(<IN>);
        $cloudamount3 = getCloudString($cloudamountflags3);
        
        # Visibility
        $vis = ( ( ord(<IN>) << 8 ) | ord(<IN>) ) / 100.0;

        # Obscurations
        $obs_flags = ( ord(<IN>) << 8 ) | ord(<IN>);
        $obs = getObscurationString( $obs_flags ); 

        # Precip Accumulation
        $paccum = ( ( ord(<IN>) << 8 ) | ord(<IN>) ) / 100.0;

        # Precipitation type
        $ptype_flags = ( ord(<IN>) << 24 ) | ( ord(<IN>) << 16 ) | ( ord(<IN>) << 8 ) | ( ord(<IN>) ); 
        $ptype = getPrecipTypeString( $ptype_flags );

        # Ambient temperature
        $ambient_temp = ( ord(<IN>) - 100);

        # Dew point
        $dew_pt = (ord(<IN>) - 100);

        # Wind direction
        $wdir_true = (ord(<IN>) * 10.0);
        $wdir_mag = (ord(<IN>) * 10.0);

        # Wind speed
        $wspd_ave = (ord(<IN>));
        $wspd_gust = (ord(<IN>));

        # Pressure
        $pressure = ( ( ord(<IN>) << 8 ) | ord(<IN>) ) / 100.0;

	# Density altitude
	$density_altitude = (ord(<IN>) * 100);

	# Sea level pressure
	$sealv_pressure =( ( ord(<IN>) << 8 ) | ord(<IN>) );
	if ($sealv_pressure == 0xff){ $sealv_pressure = "MISS";}
	else { $sealv_pressure = $sealv_pressure /10.0;}

	# RVR runway ID
        $rvr_runway = ord(<IN>);
        if ($rvr_runway == 0b11111110) { $rvr_runway = "MISS"; }
        else { $rvr_runway *= 10; }
	
	# RVR
        $rvr = ord(<IN>);
        if ($rvr == 0b11111110) { $rvr = "MISS"; }
        else { $rvr *= 100; }

	#RVR parallel runway and flag
	$rvr_flags = ord(<IN>);
	$rvr_parallel = ($rvr_flags & 0b00001111);
	$rvr_flag = ($rvr_flags >> 4);

	#Supplementary Obscurations and Precip Types
	$sup_obscurations = ( ( ord(<IN>) << 8 ) | ord(<IN>) );
	$sup_obscurations = getSupObscurationString($sup_obscurations);

	#Reserved bytes for future expansions
	for ($j=0;$j<9;$j++){
	    $byte = <IN>;
	}
	
	#Lightning Activity
	$lightning = ( ( ord(<IN>) << 8 ) | ord(<IN>) );
	$lightning = getLightningString($lightning);

	#Site status data
	$site_status = ord(<IN>);
	$site_status = getSiteStatusString($site_status);

	#Sensor data status
	$wind_dir_status = ord(<IN>);
	$wind_spd_status = $wind_dir_status;
	$wind_dir_status = ($wind_dir_status & 0b00001111);
	$wind_spd_status = ($wind_spd_status >> 4);
	$wind_dir_status = getSensorStatusString($wind_dir_status);
	$wind_spd_status = getSensorStatusString($wind_spd_status);         

        $temp_status = ord(<IN>);
        $dew_pt_status = $temp_status;
        $temp_status = ($temp_status & 0b00001111);
        $dew_pt_status = ($dew_pt_status >> 4);
        $temp_status = getSensorStatusString($temp_status);
        $dew_pt_status = getSensorStatusString($dew_pt_status);

        $pressure_status = ord(<IN>);
        $chi_status = $pressure_status;
        $pressure_status = ($pressure_status & 0b00001111);
        $chi_status = ($chi_status >> 4);
        $pressure_status = getSensorStatusString($pressure_status);
        $chi_status = getSensorStatusString($chi_status);

        $precip_type_status = ord(<IN>);
        $precip_accum_status = $precip_type_status;
        $precip_type_status = ($precip_type_status & 0b00001111);
        $precip_accum_status = ($precip_accum_status >> 4);
        $precip_type_status = getSensorStatusString($precip_type_status);
        $precip_accum_status = getSensorStatusString($precip_accum_status);

        $vis_status = ord(<IN>);
        $lightning_status = $vis_status;
        $vis_status = ($vis_status & 0b00001111);
        $lightning_status = ($lightning_status >> 4);
        $vis_status = getSensorStatusString($vis_status);
        $lightning_status = getSensorStatusString($lightning_status);

	$fzra_status = ord(<IN>);
        $rvr_status = $fzra_status;
        $fzra_status = ($fzra_status & 0b00001111);
        $rvr_status = ($rvr_status >> 4);
        $fzra_status = getSensorStatusString($fzra_status);
        $rvr_status = getSensorStatusString($rvr_status);

	#Parameter Activation Status
        $param_activation_status = ( ( ord(<IN>) << 8 ) | ord(<IN>) );
	$param_activation_status = getParamActivationString($param_activation_status);

	#Automated remards data
        $lastbyte = <IN>;
	$auto_remark_data = ord($lastbyte);
	$auto_remark_data = getAutoRemarkString($auto_remark_data);

	# TODO temp bleed off the rest of the bytes
	$remark = ""; 
        for ($j=0;$j<$length-68;$j++) {
            $lastbyte = <IN>;
            $remark .= $lastbyte;
        }

        # Output data line
        $dataline = "$date, $time, $id, $federal, $type, $units, $company, $hardware, $software, $alerts, "
                ."$cloudheight1, $cloudamount1, $cloudheight2, $cloudamount2, $cloudheight3, $cloudamount3, $vis, "
                ."$obs, $paccum, $ptype, $ambient_temp, $dew_pt, $wdir_true, $wdir_mag, "
                ."$wspd_ave, $wspd_gust, $pressure, $density_altitude, $sealv_pressure, $rvr_runway, $rvr, $rvr_parallel, "
		."$rvr_flag, $sup_obscurations, $lightning, $site_status, $wind_dir_status, $wind_spd_status, $temp_status, "
		."$dew_pt_status, $pressure_status, $chi_status, $precip_type_status, $precip_accum_status, $vis_status, "
		."$lightning_status, $fzra_status, $rvr_status, $param_activation_status, $auto_remark_data, $remark"; 
        $dataline =~ s/, /$delimiter/g;
    
        # TODO print to a file?
        print $dataline."\n";

        $num_parsed++;
        $last_id = $id;
    }

}
close IN;
if ($log_errors) {
    print ERROR "---- Parsed: $num_parsed   Errors: $num_errors\n";
    close ERROR;
}

######### Subroutines #############

# Convert cloud amount byte to string
sub getCloudString {
    my $byte = shift;
    if ($byte == 0xff) { return "MISS"; }

    my $string = "";

    # TODO check 'NONE' and 'FEW' and 'ICD' and 'MISS'
    if ($byte & 0b00000001) { $string .= "SCT "; }
    if ($byte & 0b00000010) { $string .= "BKN "; }
    if ($byte & 0b00000100) { $string .= "OVC "; }
    if ($byte & 0b00001000) { $string .= "OBS "; }
    if ($byte & 0b00100000) { $string .= "VV "; }
    if ($byte & 0b01000000) { $string .= "NONE "; }
    if ($byte & 0b10000000) { $string .= "FEW "; }

    chop $string;
    return $string;
}

# Convert auto remark to string
sub getAutoRemarkString {
    my $byte = shift;
    my $string = "";

    if ($byte & 0b00000001) { $string .= "VRBVIS "; }
    if ($byte & 0b00000010) { $string .= "VRBWDIR "; }
    if ($byte & 0b00000100) { $string .= "VRBCEIL "; }
    if ($byte & 0b00001000) { $string .= "AUTOLTG "; }
    if ($byte & 0b00010000) { $string .= "VISOFFSITE "; }
    if ($byte & 0b00100000) { $string .= "CEILOFFSITE "; }
    chop $string;
    return $string;
}

# Convert site status byte to string
sub getSiteStatusString {
    my $byte = shift;

    my $string = "";

    if (!($byte & 0b00001111)) { return "NORMAL"; }
    if ($byte & 0b00000001) { $string .= "OPER "; } else { $string .= "NOOPER "; }
    if ($byte & 0b00000010) { $string .= "TEST "; } else { $string .= "NORM "; }
    if ($byte & 0b00000100) { $string .= "MAN "; } else { $string .= "AUTO "; }
    if ($byte & 0b00001000) { $string .= "SUSPECT "; } else { $string .= "OK "; }

    chop $string;
    return $string;
}


# Convert two byte obscuration to string
sub getObscurationString {
    my $bytes = shift;
    my $string = "";

    if ($bytes & 0b0000000000000001) { $string .= "UNKNOWN " }
    if ($bytes & 0b0000000000000010) { $string .= "FG "; }
    if ($bytes & 0b0000000000000100) { $string .= "GROUNDFG "; }
    if ($bytes & 0b0000000000001000) { $string .= "PRFG "; }
    if ($bytes & 0b0000000000010000) { $string .= "HZ "; }
    if ($bytes & 0b0000000000100000) { $string .= "FU "; }
    if ($bytes & 0b0000000001000000) { $string .= "DRDU "; }
    if ($bytes & 0b0000000010000000) { $string .= "DRSA "; }
    if ($bytes & 0b0000000100000000) { $string .= "BLSA "; }
    if ($bytes & 0b0000001000000000) { $string .= "BLDU "; }
    if ($bytes & 0b0000010000000000) { $string .= "BLPY "; }
    if ($bytes & 0b0000100000000000) { $string .= "BR "; }
    if ($bytes & 0b0001000000000000) { $string .= "VA "; }
    if ($bytes & 0b0010000000000000) { $string .= "SAPO "; }
    if ($bytes & 0b0100000000000000) { $string .= "SAPOVC "; }
    if ($bytes & 0b1000000000000000) { $string .= "BCFG "; }

    chop $string;
    return $string;
}

# Convert two byte parameter activation status to string
sub getParamActivationString {
    my $bytes = shift;
    my $string = "";

    if ($bytes & 0b0000000000000001) { $string .= "BR " }
    if ($bytes & 0b0000000000000010) { $string .= "FG "; }
    if ($bytes & 0b0000000000000100) { $string .= "GF "; }
    if ($bytes & 0b0000000000001000) { $string .= "ICFG "; }
    if ($bytes & 0b0000000000010000) { $string .= "HZ "; }
    if ($bytes & 0b0000000000100000) { $string .= "SMK "; }
    if ($bytes & 0b0000000001000000) { $string .= "DU VA "; }
    if ($bytes & 0b0000000010000000) { $string .= "BS BN BD BPY "; }
    if ($bytes & 0b0000000100000000) { $string .= "RA "; }
    if ($bytes & 0b0000001000000000) { $string .= "SG "; }
    if ($bytes & 0b0000010000000000) { $string .= "FZRA "; }
    if ($bytes & 0b0000100000000000) { $string .= "GS "; }
    if ($bytes & 0b0001000000000000) { $string .= "PL "; }
    if ($bytes & 0b0010000000000000) { $string .= "SN "; }
    if ($bytes & 0b0100000000000000) { $string .= "IC "; }
    if ($bytes & 0b1000000000000000) { $string .= "GR "; }

    chop $string;
    return $string;
}

#Convert two bytes supplementary obscuration to string
sub getSupObscurationString {
    my $bytes = shift;
    my $string = "";

    if ($bytes == 0) { return "NP"; }
    if ($bytes & 0b0000000000000001) { $string .= "SG- " }
    if ($bytes & 0b0000000000000010) { $string .= "SG+ "; }
    if ($bytes & 0b0000000000000100) { $string .= "SS "; }
    if ($bytes & 0b0000000000001000) { $string .= "SS+ "; }
    if ($bytes & 0b0000000000010000) { $string .= "SSVC "; }
    if ($bytes & 0b0000000000100000) { $string .= "DS "; }
    if ($bytes & 0b0000000001000000) { $string .= "DS+ "; }
    if ($bytes & 0b0000000010000000) { $string .= "DSVC "; }
    if ($bytes & 0b0000000100000000) { $string .= "BNVC "; }
    if ($bytes & 0b0000001000000000) { $string .= "BDVC "; }
    if ($bytes & 0b0000010000000000) { $string .= "FGVC "; }
    if ($bytes & 0b0000100000000000) { $string .= "FZFG "; }
    if ($bytes & 0b0001000000000000) { $string .= "SQAL "; }

    chop $string;
    return $string;

}
#Convert 2 Byte lightning observations to string
sub getLightningString {
    my $bytes = shift;
    my $string = "";

    if ($bytes == 0) { return "NP"; }

    if ($bytes & 0b0000000000000010) { return "MISS"; }	
    if ($bytes & 0b0000000000000001) { $string .= "MetarGen " }
    if ($bytes & 0b0000000001000000) { $string .= "LTG "; }
    if ($bytes & 0b0000000010000000) { $string .= "LTGVC "; }
    if ($bytes & 0b0000000100000000) { $string .= "N "; }
    if ($bytes & 0b0000001000000000) { $string .= "NE "; }
    if ($bytes & 0b0000010000000000) { $string .= "E "; }
    if ($bytes & 0b0000100000000000) { $string .= "SE "; }
    if ($bytes & 0b0001000000000000) { $string .= "S "; }
    if ($bytes & 0b0010000000000000) { $string .= "SW"; }
    if ($bytes & 0b0100000000000000) { $string .= "W "; }
    if ($bytes & 0b1000000000000000) { $string .= "NW "; }

    chop $string;
    return $string;

}

# Convert four byte ptype to string
sub getPrecipTypeString {
    my $bytes = shift;
    my $string = "";

    if ($bytes == 4278124286) { return "NOTINST"; }
    if ($bytes == 0) { return "NP"; }

    $string .= getPrecipEncoding( "UP", ( ($bytes) & 0b1111) );
    $string .= getPrecipEncoding( "RA", ( ($bytes >> 4) & 0b1111) );
    $string .= getPrecipEncoding( "DZ", ( ($bytes >> 8) & 0b1111) );
    $string .= getPrecipEncoding( "FZRA", ( ($bytes >> 12) & 0b1111) );
    $string .= getPrecipEncoding( "FZDZ", ( ($bytes >> 16) & 0b1111) );
    $string .= getPrecipEncoding( "IP", ( ($bytes >> 20) & 0b1111) );
    $string .= getPrecipEncoding( "SN", ( ($bytes >> 24) & 0b1111) );

    if ($bytes & 0b00010000000000000000000000000000) { $string .= "SG "; }
    if ($bytes & 0b00100000000000000000000000000000) { $string .= "GS "; }
    if ($bytes & 0b01000000000000000000000000000000) { $string .= "GR "; }
    if ($bytes & 0b10000000000000000000000000000000) { $string .= "IC "; }

    chop $string;
    return $string;
}

# Convert four byte alert to string
sub getAlertString {
    my $bytes = shift;
    my $string = "";

    # TODO all bits set?
    if ($bytes == 4278124286) { return ""; }
    if ($bytes == 0) { return ""; }

    if ($bytes & 0b00000000000000000000000000000001) { $string .= "SKY "; }
    if ($bytes & 0b00000000000000000000000000000010) { $string .= "CEILU "; }
    if ($bytes & 0b00000000000000000000000000000100) { $string .= "CEILD "; }
    if ($bytes & 0b00000000000000000000000000001000) { $string .= "VISI "; }
    if ($bytes & 0b00000000000000000000000000010000) { $string .= "VISD "; }
    if ($bytes & 0b00000000000000000000000000100000) { $string .= "WDIRCHG "; }
    if ($bytes & 0b00000000000000000000000001000000) { $string .= "WSPDI "; }
    if ($bytes & 0b00000000000000000000000100000000) { $string .= "GRB "; }
    if ($bytes & 0b00000000000000000000001000000000) { $string .= "GRE "; }
    if ($bytes & 0b00000000000000000000010000000000) { $string .= "IPB "; }
    if ($bytes & 0b00000000000000000000100000000000) { $string .= "IPE "; }
    if ($bytes & 0b00000000000000000001000000000000) { $string .= "FZRAB "; }
    if ($bytes & 0b00000000000000000010000000000000) { $string .= "FZRAE "; }
    if ($bytes & 0b00000000000000000100000000000000) { $string .= "FZDZB "; }
    if ($bytes & 0b00000000000000001000000000000000) { $string .= "FZDZE "; }
    if ($bytes & 0b00000000000000010000000000000000) { $string .= "TSB "; }
    if ($bytes & 0b00000000000000100000000000000000) { $string .= "TSE "; }
    if ($bytes & 0b00000000000001000000000000000000) { $string .= "TSI "; }
    if ($bytes & 0b00000000000010000000000000000000) { $string .= "+FCO "; }
    if ($bytes & 0b00000000000100000000000000000000) { $string .= "FCO "; }
    if ($bytes & 0b00000000001000000000000000000000) { $string .= "+FCO "; }
    if ($bytes & 0b00000000100000000000000000000000) { $string .= "THRESH "; }
    if ($bytes & 0b00000001000000000000000000000000) { $string .= "SNB "; }
    if ($bytes & 0b00000010000000000000000000000000) { $string .= "SNE "; }
    if ($bytes & 0b00000100000000000000000000000000) { $string .= "RAB "; }
    if ($bytes & 0b00001000000000000000000000000000) { $string .= "RAE "; }
    if ($bytes & 0b00010000000000000000000000000000) { $string .= "UPB "; }
    if ($bytes & 0b00100000000000000000000000000000) { $string .= "UPE "; }
    if ($bytes & 0b01000000000000000000000000000000) { $string .= "FGB "; }
    if ($bytes & 0b10000000000000000000000000000000) { $string .= "FGE "; }

    chop $string;
    return $string;
}

# Convert a type to one including intensity flags
sub getPrecipEncoding {
    my $type = shift;
    my $byte = shift; # just a nibble, really

    if ($byte == 0b0000) { return ""; }
    if ($byte == 0b0001) { return $type."? "; }
    if ($byte == 0b0010) { return $type."- "; }
    if ($byte == 0b0011) { return $type." "; }
    if ($byte == 0b0100) { return $type."+ "; }
    if ($byte == 0b0101) { return $type."SH- "; }
    if ($byte == 0b0110) { return $type."SH "; }
    if ($byte == 0b0111) { return $type."SH+ "; }
    if ($byte == 0b1000) { return $type."SHVC "; }
    if ($byte == 0b1001) { return $type."DR "; }
    if ($byte == 0b1010) { return "BL".$type." "; }
    if ($byte == 0b1011) { return "BL".$type."VC " }
    if ($byte == 0b1100) { return $type." "; }
    if ($byte == 0b1101) { return $type." "; }
    if ($byte == 0b1111) { return $type."MISS "; }

    return "";
}

# Convert sensor status to string
sub getSensorStatusString {
    my $byte = shift; # Actually 4 bits

    if ($byte == 0b0000) { return ""; }
    if ($byte == 0b0010) { return "MISS"; }
    if ($byte == 0b0011) { return "MISS"; }
    if ($byte == 0b0101) { return "CONFLICT"; }
    if ($byte == 0b0110) { return "INVRANGE"; }
    if ($byte == 0b0111) { return "INVRATE"; }
    if ($byte == 0b1000) { return "INVCOUNT"; }
    if ($byte == 0b1001) { return "INVALID"; }
    if ($byte == 0b1010) { return "LINKFAIL"; }
    if ($byte == 0b1011) { return "INVDEWPT"; }
    if ($byte == 0b1111) { return "MANUAL"; }
}

