#!/usr/bin/perl
#
# pirep2mysql.pl: Decode the raw text Pilot Reports (PIREPs) and AIREPs.
# A sample PIREP and AIREP are shown below:
# PIREP: DEN UA /OV DEN 225030/TM 1438/FLDURD/TP BA46/TB MDT 130-170
# AIREP: ARP M7949 45N040W 1257 F350 MS55 180/57
#
# See Usage below or run program with a -h command-line option to view
# various options.
#
#  The following entry in the LDM 'pqact.conf' file is used to
#  make this run to ingest/decode realtime stream of data:
#  WMO  ^U[AB].* ([0-3][0-9])([0-2][0-9])    PIPE    pirep2mysql.pl
#
# Last updated 2130 UTC 16 Jun 2004
# Greg Thompson <gthompsn@ucar.edu>  NCAR-RAP
#
use strict;
use vars qw($opt_h $opt_v $opt_O $opt_L $opt_t $opt_n );
use Getopt::Std;
use Time::Local;
use Env qw(ADDSHOME);
use NavAids;
use StationUtils;
use DbConnect;
use Monitor;
no locale;

(my $prog = $0) =~ s%.*/%%;            #...Determine program basename
umask 002;                             #...Set file permissions.
$| = 1;                                #...Unbuffer standard output

#...------------...Global and local vars...---------------------

our ($dbName, $pireps_table, %compassPts, @tbKeys, @icKeys, %pirepVars, $skipDb);
our ($NM2KM, $FT2M, $M2FT, $KM2SM, $verbose, $doMap, $data_time);
$doMap = 0;
$dbName = "weather";
%compassPts = ("NNE",  22.5,
               "ENE",  67.5,
               "ESE", 112.5,
               "SSE", 157.5,
               "SSW", 202.5,
               "WSW", 247.5,
               "WNW", 292.5,
               "NNW", 337.5,
               "NE",   45,
               "SE",  135,
               "SW",  225,
               "NW",  315,
               "N",   360,
               "E",    90,
               "S",   180,
               "W",   270);
@tbKeys = ("SEV_EXTM;SEV-EXTM;7",
           "MOD_SEV;MOD-SEV;5",
           "MOD_HVY;MOD-SEV;5",
           "LGT_MOD;LGT-MOD;3",
           "SMTH_LGT;SMTH-LGT;1",
           "EXTM;EXTM;8",
           "SEV;SEV;6",
           "HVY;MOD-SEV;5",
           "MOD;MOD;4",
           "LGT;LGT;2",
           "SMTH;NEG;-1",
           "NEG;NEG;-1");
@icKeys = ("SEV_EXTM;SEV;8",
           "MOD_SEV;MOD-SEV;6",
           "MOD_HVY;MOD-SEV;6",
           "LGT_MOD;LGT-MOD;4",
           "TRC_LGT;TRC-LGT;2",
           "EXTM;SEV;8",
           "SEV;SEV;8",
           "HVY;HVY;7",
           "MOD;MOD;5",
           "LGT;LGT;3",
           "TRC;TRC;1",
           "NEG;NEG;-1");
$NM2KM = 1.852;
$FT2M = 0.3048;
$M2FT = 1.0/$FT2M;
$KM2SM = 0.6214;

my $timeout = 1200;                    #...Program will exit if stagnant this long.
my ($rin, $rout, $nfound);
my ($sec,$min,$hour,$day,$month,$year,$yyyy,$mm,$dd,$hh,$yr,$mo,$dy,$cur_time);
my ($day_bul, $hour_bul, $min_bul, $tstamp_bul, $bulletin_time, $bulletinTime);
my ($icaoId1, $pirep, @pireps, $status);
my ($midPointAssumedFlag, $noTimestampFlag, $flightLevelRangeFlag, $aglFlag, $noFlightLevelFlag, $badLocationFlag);
my $handle;       #either a db handle or file handle
#initialize qc field flags
$midPointAssumedFlag = 0;
$noTimestampFlag = 0;
$flightLevelRangeFlag = 0;
$aglFlag = 0;
$noFlightLevelFlag = 0;
$badLocationFlag = 0;


#...------------...Usage Info...--------------------------------

$opt_h = $opt_v = 0;
my $usage = <<EOF;
Usage: $prog [-hdv] [-n] [-t yyyymmddhh] [-L log_dir]
  -h  Help:     Print out this usage information.
  -v  Verbose:  Print out verbose debug information.
  -t  yyyymmddhh: adjust the year, month, day, and hour otherwise current date.
  -L  log file dir: Specify where the log file lives.
  expects standard input to be parsed so redirect a file to STDIN if need be.
EOF

#...------------...Sanity check the options...------------------

&getopts('hvt:L:') || die $usage;
die $usage if $opt_h;
$verbose = 1 if $opt_v;
if ($opt_L) {
    open (LOG, ">>$opt_L/pirepLog_$$.log" ) or die "could not open $opt_L/pirepLog_$$.log: $!\n";
    open (STDERR, ">&LOG" ) or die "could not dup stdout: $!\n";
}
select( STDERR ); $| = 1;
select( LOG ); $| = 1;
if ($opt_t) {
    ($yyyy, $mm, $dd, $hh) = $opt_t =~ /([12][90][0-9][0-9])([01][0-9])([0-3][0-9])([0-2][0-9])/;
    print LOG "Expecting archive PIREPs for year=$yyyy, month=$mm, day=$dd, and hour=$hh\n" if $verbose;
    if ($yyyy<1900 || $yyyy>2099 || $mm<1 || $mm>12 || $dd<1 || $dd>31 || $hh>23) {
        warn "Incorrect date format ($opt_t)\n";
        die "Please try again\n\n";
    }
}


#...------------...Set interrupt handler...---------------------

$SIG{ 'INT' }   = 'atexit';
$SIG{ 'KILL' }  = 'atexit';
$SIG{ 'TERM' }  = 'atexit';
$SIG{ 'QUIT' }  = 'atexit';


#...------------...Enable station lookup...---------------------
#   The handle is either a database handle, or a file handle to a file of station locations.
#   The latter is for those who wish to decode without the dependency of a database.
$handle = &NavAids::init();
&StationUtils::init();

#...------------...Begin parsing breaking on cntrl C...---------

$/ = "\cC";
#$/ = "ALERT: End";         # MSC datastream uses this not CNTL-C

#Print the Header for the CSV 
print STDOUT "headerDescriptor, time, icaoId, aircraftType, latitude(decimal deg), longitude(decimal deg), altitude/flightLevel(100 ft MSL), cloudBase1(100 ft MSL), cloudTop1(100 ft MSL), cloudCoverage1(100 ft MSL), cloudBase2(100 ft MSL), cloudTop2(100 ft MSL), cloudCoverage2(100 ft MSL), visibility(mi), weather, airTemperature(C), windDirection, windSpeed(kts), icingBase1(100 ft MSL), icingTop1(100 ft MSL), icingIntensity1, icingType1, icingBase2, icingTop2(100 ft MSL), icingIntensity2, icingType2, turbulenceBase1(100 ft MSL), turbulenceTop1(100 ft MSL), turbulenceIntensity1, turbulenceType1, turbulenceFrequency1, turbulenceBase2(100 ft MSL), turbulenceTop2(100 ft MSL), turbulenceIntensity2, turbulenceType2, turbulenceFrequency2, maximumDerivedEquivalentVerticalGust(m/s), rawObservation, reportType\n";

while ( 1 ) {

    #...------------...Keep STDIN attached unless timeout...----

    open (STDIN, '-');
    binmode (STDIN);
    $rin = ``;
    vec($rin,fileno(STDIN),1) = 1;
    $nfound = select ( $rout = $rin, undef, undef, $timeout );

    &atexit("timeout", $handle) if( ! $nfound );
    &atexit("eof", $handle) if ( eof(STDIN) );

    #...------------...Set/Get current time info...-------------

    if ($yyyy && $mm && $dd) {
        $cur_time = &timegm(0,0,$hh,$dd,$mm-1,$yyyy);
    } else {
        $cur_time = time();
    }
    ($sec,$min,$hour,$day,$month,$year) = (gmtime($cur_time))[0..5];
    $year += 1900;
    $month += 1;

    #...------------...Strip out unwanted stuff...--------------

    $_ = <STDIN>;
    s/ALERT\:.*//g;                 # Ignore MSC data record separators.
    tr/a-z/A-Z/;                    # Upper-case a-z letters.
    tr/ \+,-\.\/0-9\=A-Z\n//cd;     # Allow only the following chars: A-Z 0-9 + , - . / =
    s/\.{2,}/-/g;                   # Swap dash for 2 or more ..
    next if (/^\s*\n?$/);           # Ignore empty stuff between Control-C chars
    s/\n ?([0-9]{3}) ?\n//;         # Ignore 3-digit sequence number
    s/\n {1,}/ /g;                  # Re-attach continued lines indicated by 1+ spaces.
    s/[ \t]+/ /g;                   # Collapse multiple whitespace to single space.
    s/ ?\n+/\n/g;                   # Collapse multiple newlines to single newline.
    s/9999/99/g;                    # Switch bad visibility value to something that makes sense in text file.
                                    # Skip AFOS PIL headers.
    s/(PRCUS|PIRUS|PIREP|AIREP|AMDAR)(( [0-3][0-9][0-2][0-9])| NIL=)?\n//g;
    s/\n[A-Z][A-Z] [0-3][0-9][0-2][0-9]([0-5][0-9])?\n/\n/g;

#   next if (/^\n?UD/);

    # Header date contains bulletin timestamp.
    s/^\n?(U[ABD]|YI)[0-9A-Z]+ +([A-HK-PR-Z][A-Z0-9]{3}) +([0-3][0-9])([0-2][0-9])([0-5][0-9])[ -~]*\n/\n/;
    $icaoId1  = $2   if ( defined($2) );
    $day_bul = 0;
    $day_bul  = $3*1 if ( defined($3) && ($3 <= 31) );
    $hour_bul = $4*1 if ( defined($4) && ($4 <= 23) );
    $min_bul  = $5*1 if ( defined($5) && ($5 <= 59) );

    unless ($day_bul > 0) {
        $min_bul = $min;
        $hour_bul = $hour;
        $day_bul = $day;
        $tstamp_bul = sprintf("%02d%02d%02d", $day_bul, $hour_bul, $min_bul);
        print LOG ("WARNING, proper bulletin time timestamp not found, assuming $tstamp_bul [", substr($_, 0, 20), "]\n");
    }

    $bulletin_time = &my_timegm(0,$min_bul,$hour_bul,$day_bul,$month-1,$year);
    if ($day_bul != $day) {
        $bulletin_time = &construct_timestamp($min_bul,$hour_bul,$day_bul,-3,0.5, "bulletinTime", $icaoId1);
        if ($bulletin_time <= 0) {
            $bulletin_time = $cur_time;
            print LOG ("WARNING, bulletin time not logical (possibly typo), assuming current time [", substr($_, 0, 20), "]\n");
        }
        ($dy, $mo, $yr) = (gmtime($bulletin_time))[3..5];
        $bulletinTime = sprintf("%04d%02d%02d%02d%02d00", $yr+1900, $mo+1, $dy, $hour_bul, $min_bul);
    } elsif ($bulletinTime == -1) {
        $bulletin_time = $cur_time;
    } else {
        $bulletinTime = sprintf("%04d%02d%02d%02d%02d00", $year, $month, $day_bul, $hour_bul, $min_bul);
    }

    #...------------...Split into singular pirep by "="...------

    @pireps = split /\=/;

    foreach $pirep (@pireps) {
  

        $pirep =~ s/\n/ /g;
        $pirep =~ s/^ +//g;
        next unless (length($pirep) > 14);
        next if ( $pirep =~ /TEST/ );

        %pirepVars = &initialize($icaoId1, $bulletin_time, $pirep);
        #The qcField is initialized to 2, indicating no time stamp flag.
        #To be consistent, set the noTimeStampFlag to true and reset to 'false' inside the 
        #decode_tm subroutine if a timestamp exists.
        $noTimestampFlag = 1;

        if ( $pirep =~ /\b(U?UA)\b/ ) {
            print LOG "\n Decoding a $1 voice pirep: $pirep\n" ;
            if ($1 eq "UA") {
                $pirepVars{"reportType"} = "PIREP";
            } elsif ($1 eq "UUA") {
                $pirepVars{"reportType"} = "Urgent PIREP";
            }
            &decode_voicerep( $pirep ) || (warn "WARNING, cannot decode PIREP: $pirep\n", next);
            print LOG ("\tLeftovers not decoded: ", $pirepVars{"leftOver"}, "\n") if ( $pirepVars{"leftOver"});
        } elsif ( $pirep =~ /\n?(LV.|UNS|ASC|DES|\/\/\/) [A-Z]{2,3}[0-9]{3,4}([A-Z]{2,3})?\b/ ) {
            print LOG "\n Not decoding AMDAR: $pirep\n" ;
            # $pirepVars{"reportType"} = "AMDAR";
            # &decode_amdar( $pirep ) || (warn "WARNING, cannot decode AMDAR: $pirep\n", next);
            next;
        } elsif ( $pirep =~ /\n?(ARP)|([A-Z]{3}[0-9]{1,4})\b/ ) {
            print LOG "\n inside main, Decoding AIREP: $pirep\n" ;
            $pirepVars{"reportType"} = "AIREP";
            &decode_airep( $pirep ) || (warn "WARNING, cannot decode AIREP: $pirep\n", next);
        } else {
            warn "WARNING, apparently nothing useful found in: $pirep\n";
            next;
        }
        if ( ($pirepVars{"lat"} == 39.9999 && $pirepVars{"lon"} == -98.9999 ) ||
            $pirepVars{"lat"} < -90 || $pirepVars{"lat"} > 90 ||
            $pirepVars{"lon"} < -180 || $pirepVars{"lon"} > 360 ||
            !defined($pirepVars{"fltLvl"}) )  {
                warn "WARNING, could not determine lat/lon or fltLvl for this pirep:$pirep\n";
                next;
        }


        #...------------...Persist the decoded/parsed PIREP(s) to a CSV file...-------
       
        &writePirep2Csv();

        print LOG "\n";
     
        #Reset the flags for the next PIREP report
        $midPointAssumedFlag = 0;
        $noTimestampFlag = 0;
        $flightLevelRangeFlag = 0;
        $aglFlag = 0;
        $noFlightLevelFlag = 0;
        $badLocationFlag = 0;
    }

    $data_time = sprintf("%04d%02d%02d%02d", $year, $month, $day, $hour);
    Monitor::registerDataMapper($data_time, "pireps", 0, "decoded", 0) if $doMap;

    #Use newlines to delineate one batch of PIREPs from another
    print STDOUT "\n";

}       # End of infinite while loop for realtime mode

print LOG "\n\nExiting program, $prog\n" if $verbose;

#
#+--+------------------------------------------------------------------+
#
sub decode_voicerep {
    my ($line) = @_;
    my ($group_char, @group_chars, @locates, @unsorted, %l_index, %m_index, $key);
    my ($ld, $ov, $tm, $fl, $tp, $sk, $wx, $ta, $wv, $tb, $ic, $rm);
    my ($remain, $dumb, $first, $n, $search);
    @group_chars = ("OV", "TM", "FL", "TP", "SK", "WX", "TA", "WV", "TB", "IC", "RM");
    $first = 0;

#...Make sure that any identifier has a slash ("/") immediately prior to it

    foreach $group_char (@group_chars) {
        $line =~ s%(/) ?($group_char)%$1$2%;
    }

#...Create associative array containing key identifier and index within string

    foreach $group_char (@group_chars) {
        $search = "/" . $group_char;
        $l_index{$group_char} = index($line, $search) unless index($line, $search) == -1;
    }

#...If associative array with key identifiers is empty, then meaningless PIREP

    return 0 if !(keys %l_index);

#...Sort the flipped array by numerical index of the key identifiers
#...Also assume meaningless PIREP if only 1 keyword identifier

    @unsorted = values(%l_index);
    @locates = sort bynumber @unsorted;
    return 0 if $#locates < 2;

    push(@locates,length($line)) if !$l_index{"RM"};
    $first = ($l_index{"OV"})? $l_index{"OV"} : $locates[0];
#...Immediately assume "OV" is first key identifier and "RM" is the last

    $ld = substr($line, 0, $first);
    $rm = substr($line, $l_index{"RM"}+3) if $l_index{"RM"};

#...Create another associative array flipping the keys and values of the l_index array

    foreach $key (keys %l_index) {
        $m_index{$l_index{$key}} = $key;
    }

#...Finally, grab the substrings which make up the individual portions

    for $n (0..$#locates) {
        if ($m_index{$locates[$n]} eq "OV") {
            $ov = substr($line, $locates[$n]+3, $locates[$n+1]-$locates[$n]-3);
        } elsif ($m_index{$locates[$n]} eq "TM") {
            $tm = substr($line, $locates[$n]+3, $locates[$n+1]-$locates[$n]-3);
        } elsif ($m_index{$locates[$n]} eq "FL") {
            $fl = substr($line, $locates[$n]+3, $locates[$n+1]-$locates[$n]-3);
        } elsif ($m_index{$locates[$n]} eq "TP") {
            $tp = substr($line, $locates[$n]+3, $locates[$n+1]-$locates[$n]-3);
        } elsif ($m_index{$locates[$n]} eq "SK") {
            $sk = substr($line, $locates[$n]+3, $locates[$n+1]-$locates[$n]-3);
        } elsif ($m_index{$locates[$n]} eq "WX") {
            $wx = substr($line, $locates[$n]+3, $locates[$n+1]-$locates[$n]-3);
        } elsif ($m_index{$locates[$n]} eq "TA") {
            $ta = substr($line, $locates[$n]+3, $locates[$n+1]-$locates[$n]-3);
        } elsif ($m_index{$locates[$n]} eq "WV") {
            $wv = substr($line, $locates[$n]+3, $locates[$n+1]-$locates[$n]-3);
        } elsif ($m_index{$locates[$n]} eq "TB") {
            $tb = substr($line, $locates[$n]+3, $locates[$n+1]-$locates[$n]-3);
        } elsif ($m_index{$locates[$n]} eq "IC") {
            $ic = substr($line, $locates[$n]+3, $locates[$n+1]-$locates[$n]-3);
        }
    }    

#...Some Canadian PIREPs don't have "/TM" or "/FL" so we have a special case here

    if ($ov =~ s/\b(C?[A-Z][A-Z0-9]{2}( ?[0-3][0-9][0-9]\d{2,3})?) ([0-2][0-9][0-5][0-9]) FL(.*)// && !$tm) {
        $ov = $1;
        $tm = $3;
        $fl = $4;
        print LOG " Canadian PIREP in non-standard form." if $verbose;
        print LOG "\tApparently:  OV=$ov, TM=$tm, FL=$fl\n" if $verbose;
    }

#...Now decode each substring by calling the appropriate portion-decoder
 
    &decode_ov($ld, $ov) if ($ld || $ov);
    &decode_tm($tm) if $tm;
    &decode_fl($fl) if $fl;
    &decode_tp($tp) if $tp;
    &decode_sk($sk) if $sk;
    &decode_wx($wx) if $wx;
    &decode_ta($ta) if $ta;
    &decode_wv($wv) if $wv;
    &decode_wx($wv) if (!$wx && $wv && !$pirepVars{"wdir"});  # sometimes wx found in wv (typo)
    &decode_tb($tb) if $tb;
    &decode_ic($ic) if $ic;
    &decode_rm($rm) if $rm;

#...Attempt to fill in missing data by combining info from different
#   portions using certain assumptions

    &assumptions();

    return 1;

}

#
#+--+------------------------------------------------------------------+
#+--+------- DECODE OV INFO -------------------------------------------+
#
#...Allow for the following possibilities:
#   [4012N 10451W], [DEN], [DEN 120040], [20 SE DEN], [DEN-GLD], [DEN-25SE LIC]

sub decode_ov {
    my ($lead, $string) = @_;
    use vars qw ($dbh $icaoId1);
    my ($remain, $distance, $bearing, @identifier, $latStr, $lonStr, $n_hemi, $e_hemi);
    my ($lat, $lon, $lat1, $lon1, $lat2, $lon2, $elev, $dist);
    my $keeper = 0;

    $lat = $pirepVars{"lat"};
    $lon = $pirepVars{"lon"};
    $elev = $distance = $bearing = 0;
    $identifier[0] = $identifier[1] = $identifier[2] = $n_hemi = $e_hemi = "";

#...First, grab any identifier that may have preceeded the OV group
#   This will go into the 3rd place holder for 3-letter IDs.

    $lead =~ tr/ [A-Z0-9]//cd;
    $lead =~ s/\bUU?A\b//;
    $lead =~ s/^\s+//;
    $lead =~ s/\s+$//;
    ($identifier[2], $remain) = $lead =~ /\b(K?[A-Z0-9]{3})\b(.*)/;
    $string = $remain if !$string;

#...Some PIRUS reports may have lat/lon info in the OV group instead
#   of station identifiers.  If so, great, otherwise, parse for
#   possible distance/bearing info and station ids.
    $string =~ tr/[A-Z0-9] \-//cd;
    $string =~ s/ ?AREA ?//;
    $string =~ s/\bTO\b/-/;
    
    #ADDS-412 Update regex so the e_hemi must be defined, to avoid
    #incorrectly decoding 61S270028 and 12N27003 as latitude/longitudes
    #instead of station identifier+bearing+distance. 
    if ( $string =~ s/(\d{2,4})([NS]) ?(\d{2,5})([WE]) ?// ) {
        $latStr = $1;
        $n_hemi = $2;
        $lonStr = $3;
        $e_hemi = $4 if (defined($4));
        $lat = (length($latStr) > 2)? int($latStr/100) + ($latStr%100)/60: $latStr*1;
        $lat *= -1 if ($n_hemi eq "S");
        $lon = (length($lonStr) > 3)? int($lonStr/100) + ($lonStr%100)/60: $lonStr*1;
        $lon *= -1 if ($e_hemi eq "W");
        $keeper = 1;

    } elsif ( $string =~ s/\b([CKP]?[A-Z0-9]{3}) ?- ?(([0-9]{1,3}) ?([NESW]{1,3}) )([CKP]?[A-Z0-9]{3})\b// ) {
        $identifier[0] = $1;
        $identifier[1] = $5;
        ($lat1, $lon1, $elev) = StationUtils::stationInfo_lookup ($handle, $identifier[0], $icaoId1);
        ($lat2, $lon2, $elev) = StationUtils::stationInfo_lookup ($handle, $identifier[1], $icaoId1);
        $distance = $3*$NM2KM;
        $bearing = $compassPts{$4};
        if ($lat2 != 39.9999 && $lon2 != -98.9999 && $distance > 0) {
            ($lat2, $lon2) = NavAids::comp_new ($lat2, -1.0*$lon2, $bearing, $distance);
            $lon2 *= -1.0;
        }
        if ($lat1 != 39.9999 && $lon1 != -98.9999 && $lat2 != 39.9999 && $lon2 != -98.9999) {
            ($lat, $lon) = NavAids::comp_midpoint ($lat1, -1.0*$lon1, $lat2, -1.0*$lon2);
            $lon *= -1.0;
            $pirepVars{"qcField"} += 1;
            $midPointAssumedFlag = 1;
            $lat = $lat1;
            $lon = $lon1;
            $pirepVars{"qcField"} += 32;
            $badLocationFlag = 1; 
        }elsif ($lat1 != 39.9999 && $lon1 != -98.9999) {
            $lat = $lat1;
            $lon = $lon1;
            $pirepVars{"qcField"} += 32;
            $badLocationFlag = 1;
        } elsif ($lat2 != 39.9999 && $lon2 != -98.9999) {
            $lat = $lat2;
            $lon = $lon2;
            $pirepVars{"qcField"} += 32;
            $badLocationFlag = 1;
        }

    } elsif ( $string =~ s/\b([CKP]?[A-Z0-9]{3}) ?- ?([CKP]?[A-Z0-9]{3})( ?([0-3][0-9]{2})([0-9]{3}))?// ) {
        $identifier[0] = $1;
        $identifier[1] = $2;
        ($lat1, $lon1, $elev) = StationUtils::stationInfo_lookup ($handle, $identifier[0], $icaoId1);
        ($lat2, $lon2, $elev) = StationUtils::stationInfo_lookup ($dbh, $identifier[1], $icaoId1);
        if (defined($3)) {
            $bearing = $4*1;
            $distance = $5*$NM2KM;
            if ($lat2 != 39.9999 && $lon2 != -98.9999) {
                ($lat2, $lon2) = NavAids::comp_new ($lat2, -1.0*$lon2, $bearing, $distance);
                $lon2 *= -1.0;
            }
        }
        if ($lat1 != 39.9999 && $lon1 != -98.9999 && $lat2 != 39.9999 && $lon2 != -98.9999) {
            ($lat, $lon) = NavAids::comp_midpoint ($lat1, -1.0*$lon1, $lat2, -1.0*$lon2);
            $lon *= -1.0;
            $pirepVars{"qcField"} += 1;
            $midPointAssumedFlag = 1;
        } elsif ($lat1 != 39.9999 && $lon1 != -98.9999) {
            $lat = $lat1;
            $lon = $lon1;
            $pirepVars{"qcField"} += 32;
            $badLocationFlag = 1;
        } elsif ($lat2 != 39.9999 && $lon2 != -98.9999) {
            $lat = $lat2;
            $lon = $lon2;
            $pirepVars{"qcField"} += 32;
            $badLocationFlag = 1;
        }

    } elsif ( $string =~ s/\b(([0-9]{1,3}) ?([NESW]{1,3}) )([CKP]?[A-Z0-9]{3})\b// ) {
        $identifier[0] = $4;
        ($lat, $lon, $elev) = StationUtils::stationInfo_lookup ($handle, $identifier[0], $icaoId1);
        $distance = $2*$NM2KM;
        $bearing = $compassPts{$3};
        if ($lat != 39.9999 && $lon != -98.9999 && $distance > 0) {
            ($lat, $lon) = NavAids::comp_new ($lat, -1.0*$lon, $bearing, $distance);
            $lon *= -1.0;
        }

    } elsif ( $string =~ s/\b([CKP]?[A-Z0-9]{3})( ?([0-3][0-9]{2})([0-9]{3}))?\b// ) {
        $identifier[0] = $1;
        ($lat, $lon, $elev) = StationUtils::stationInfo_lookup ($handle, $identifier[0], $icaoId1);
        if (defined($2)) {
            $bearing = $3*1;
            $distance = $4*$NM2KM;
            if ($lat != 39.9999 && $lon != -98.9999 && $distance > 0) {
                ($lat, $lon) = NavAids::comp_new ($lat, -1.0*$lon, $bearing, $distance);
                $lon *= -1.0;
            }
        }

    } else {
        $lat = 39.9999;
        $lon = -98.9999;
    }

#...In case the found lat/lon differs greatly (500 km) from the leading
#   identifier, probably not good so adjust to the latter (sanity check).

    if ($identifier[2] && !$keeper) {
        ($lat1, $lon1, $elev) = StationUtils::stationInfo_lookup ($handle, $identifier[2], $icaoId1);
        unless ($lat1 == 39.9999 && $lon1 == -98.9999) {
            $dist = NavAids::gc_dist($lat, -1.0*$lon, $lat1, -1.0*$lon1);
            if ($lat==39.9999 && $lon==-98.9999 && $lat1>=-90 && $lat1<=90 && $lon1>=-180 && $lon1<=360) {
                $lat = $lat1;
                $lon = $lon1;
                $pirepVars{"qcField"} += 32;
                $badLocationFlag = 1;
            } elsif ($dist > 500 && $lat1>=-90 && $lat1<=90 && $lon1>=-180 && $lon1<=360) {
                $pirepVars{"qcField"} += 32;
                $badLocationFlag = 1;
            }
        }
    }

    print LOG "\tDistance:\t",int($distance/$NM2KM) if ($verbose && $distance);
    print LOG "\tBearing is:\t$bearing\n" if ($verbose && $distance);
    print LOG "\tIdentifier1:\t$identifier[0]" if $verbose && $identifier[0];
    print LOG "\tIdentifier2:\t$identifier[1]" if $verbose && $identifier[1];
    print LOG "\tIdentifier3:\t$identifier[2]" if $verbose && $identifier[2];
    print LOG "\n" if $verbose;

    printf LOG ("\tFinal lat/lon:\t(%8.4f, %9.4f)\n", $lat, $lon) if $verbose;

    $pirepVars{"lat"} = $lat;
    $pirepVars{"lon"} = $lon;
    $pirepVars{"elev"} = $elev*$M2FT;


}
#
#+--+------------------------------------------------------------------+
#+--+------- DECODE TM INFO -------------------------------------------+
#
#...Time should only ever be in 4 digits of the form hhmm

sub decode_tm {
    my ($string) = @_;
    my ($hhmm, $yr, $mo, $dy, $hour, $minute, $unix_time);
    use vars qw ($year $month $day $cur_time);

    $unix_time = 0;
    $hour = -1;
    $minute = 0;

    if ($string =~ s/^ ?([0-2][0-9])([0-5][0-9])\b// ) {
        $hour = $1*1 if ($1 <= 24);
        $hour = 0 if ($hour == 24);
        $minute = $2*1;
        $hhmm = sprintf("%02d%02d", $hour, $minute);
    }

    if ($hour >= 0 && $hour < 24) {
        $unix_time = &construct_timestamp($minute,$hour,$day,-8,0.5, "obsTime", " ");
        if ($unix_time > 0) {
            ($minute,$hour,$dy,$mo,$yr) = (gmtime($unix_time))[1..5];
            $yr += 1900;
            $mo += 1;
        }
    } else {
        warn "WARNING, no valid timestamp found in TM string: $string\n";
    }
    if ($unix_time > 0) {
        $pirepVars{"obsTime"} = $unix_time;
        $pirepVars{"qcField"} -= 2 if ($pirepVars{"qcField"} > 1);
        $noTimestampFlag = 0;
    } else {
        $unix_time = &construct_timestamp($minute,$hour,$day,-23,0.75, "obsTime", " ");
        ($minute,$hour,$dy,$mo,$yr) = (gmtime($unix_time))[1..5];
        $yr += 1900;
        $mo += 1;
        if ($unix_time > 0) {
            print LOG "SUSPECT timestamp (older than typical) day=$dy, time=${hhmm}z\n";
            $pirepVars{"obsTime"} = $unix_time;
        }
    }

#...Sometimes there is no break indicator from TM to FL group so
#   if $string still has more, try parsing for FL data.

    $string =~ s/^\s+//;
    $string =~ s/\s+$//;
    &decode_fl($string) if ($string ne "");

    print LOG "\tDate info:\t$yr $mo $dy $hour $minute ($unix_time)\n" if $verbose;


}

#
#+--+------------------------------------------------------------------+
#+--+------- DECODE FL INFO -------------------------------------------+
#
#...Flight level may have keywords that don't specify actual altitude (bummer)
#   or there may be a range given (120-170) or single level (080).  Levels
#   are supposed to be in hundreds of feet above MSL (not AGL!) but attempt
#   to add surface elevation of close identifier.
#   These are of no use to us:
#         "DURGD", "DURD", "DRD", "DSCNT", "DCNT", "DRGC", "APCH",
#         "DURGC", "DURC", "DRC", "ASCNT", "CLM", "CLIM", "CLIMB",
#         "DEPART", "DPRT", "DEP",
#         "BLO", "BLW", "BELOW", "ABV", "ABO", "ABOVE", "AOB",
#         "BASE", "BASES", "OTP", "ON TOP", "TPS", "TOPS",
#         "UNK", "UKN", "UNKN"

sub decode_fl {
    my ($string) = @_;
    my $fl = -1;
    my $agl = 0;
    $string =~ tr/[A-Z0-9] \-//cd;
    $string =~ s/(FL?|A)([0-6][0-9][0-9])/$2/;
    $string =~ s/\b([0-9][0-9])\b/0$1/g;        # Change 2-digit numbers to 3 with leading zero
    $string =~ s/^\s+//;
    $string =~ s/\s+$//;

    if ( $string =~ s/\b([0-6][0-9][0-9]) ?- ?([0-6][0-9][0-9])\b// ) {
        $fl = nint(($1+$2)*0.5);
        $pirepVars{"qcField"} += 4;
        $flightLevelRangeFlag = 1;
    } elsif ( $string =~ s/\b([0-6][0-9][0-9])(M)?\b// ) {
        $fl = $1*1;
        if (defined($2)) {
            $fl = nint($1*$M2FT/100);
        }
    } elsif ( $string =~ s/\b(SFC|SURFACE|SF|GND|OG|GROUND|GRND|GR)\b// ) {
        $fl = 0;
        $agl = 1;
    }
    $agl = 1 if ( $string =~ s/\bAGL\b// );

    print LOG "\tFlight Lvl:\t$fl\n" if $verbose;

    if ($fl >= 0) {
        $fl = $fl + nint($pirepVars{"elev"}/100) if ( $agl && ($pirepVars{"elev"} > 0) );
        $pirepVars{"qcField"} += 8 if $agl;
        $pirepVars{"fltLvl"} = $fl;
        $aglFlag = 1 if $agl;
    }

}

#
#+--+------------------------------------------------------------------+
#+--+------- DECODE TP INFO -------------------------------------------+
#
#...Just grab first 2-8 non-blank characters
#
sub decode_tp {
    my ($string) = @_;

    $string =~ tr/[A-Z0-9]//cd;
    $string =~ s/^\s+//;
    $string =~ s/\s+$//;
    if ( $string =~ s/^([A-Z0-9]{2,8})// ) {
        $pirepVars{"acType"} = "$1";
    }

    printf LOG ("\tAircraft type:\t%s\n", $pirepVars{"acType"}) if $verbose;

}

#
#+--+------------------------------------------------------------------+
#+--+------- DECODE SK INFO -------------------------------------------+
#
#...Sky conditions include cloud types: FEW, SCT, BKN, OVC and combine
#   with 3-digit values for cloud base. If a cloud top is known, then
#   its height is generally mentioned explicitly.  Besides these, there
#   are some special-meaning tokens: CLR, SKC, VMC, IOC, IMC, OBSC,
#   W0X0F, VVxxx, CAVU (clear above, visibility unlimited).
#
sub decode_sk {
    my ($string) = @_;
    my ($base, $top, $cvg, $n, $n2, $prev, $post);

    #...Force certain abbreviations to make parsing simpler later

    $string =~ s/0VC/OVC/g;                   # I hate when they do this
    $string =~ s/W ?\/ ?S/W-S/g;              # Change FEW/SCT to FEW-SCT; protect split coming soon
    $string =~ s/T ?\/ ?B/T-B/g;              # Change SCT/BKN to SCT-BKN
    $string =~ s/N ?\/ ?O/N-O/g;              # Change BKN/OVC to BKN-OVC
    $string =~ s/T[0O]?PS?/TOP/g;             # Force variants of TOP to match pattern needed later
    $string =~ s/\/ ?TOP/-TOP/g;              # Change /TOP to -TOP; protect split coming next
    $string =~ s/[\- ]?TOP ?UNKN?/ /g;        # This is no use to us
    $string =~ s/(CIG|CEILI?N?G?)/BASE/g;     # Unusual but simple fix here

    #...Groupings sometimes separated by , or / - split and decode separately.

    if ($string =~ s/[\/,]//) {
        $prev = $`;
        $post = $';
        &decode_sk($prev);
        &decode_sk($post);
        return;
    }

    # print LOG "parsing SKY portion: $string\n" if $verbose;

    $string = &preprocessString($string);
    $string =~ s/\b([0-9][0-9])\b/0$1/g;      # Change 2-digit numbers to 3 with leading zero
                                              # Cannot deal with alt ranges, use first one only
    $string =~ s/([0-6][0-9][0-9])-[0-6][0-9][0-9]/$1/g;
    $string =~ s/([^ \-])TOP/$1 TOP/g;        # Make it simpler to decode below
    $string =~ s/BASE?S?/BASE/g;
    $string =~ s/FEW ?- ?SCT/SCT/g;           # Collapse a few straddle categories
    $string =~ s/SCT ?- ?BKN/BKN/g;
    $string =~ s/BKN ?- ?OVC/OVC/g;
    $string =~ s/(IOC|IAO) CLO?U?DS/BKN/g;    # In and out of clouds same as BKN
    $string =~ s/(CB|T?CU) ?//g;              # Nothing to be gained from these next few
    $string =~ s/CLDS?//g;
    $string =~ s/(RGD|RAGG?E?D?) ?//;

    #...Ocassionally these weather items get thrown into here

    if ( $string =~ s/\b(HA?ZE?|FU|BR|FG|SMOKE|FOG|MIST|ASH|FV[0-9]+[KS]M)\b// ) {
        &decode_wx("$&");
    }

    for (1..2) {
        last unless (length($string) > 1);
        $base = $top = -1;
        $cvg = "";
        if ( $string =~ s/(OVC|BKN|SCT|FEW) ?([0-6][0-9][0-9])[- ]*TOP ?([0-6][0-9][0-9])// ) {
            $cvg = "$1";
            $base = $2*1;
            $top = $3*1;
        } elsif ( $string =~ s/([0-6][0-9][0-9]) ?(OVC|BKN|SCT|FEW)[- ]*TOP ?([0-6][0-9][0-9])// ) {
            $base = $1*1;
            $cvg = "$2";
            $top = $3*1;
        } elsif ( $string =~ s/(BASE|TOP) ?(OVC|BKN|SCT|FEW) ?([0-6][0-9][0-9])// ) {
            $cvg = "$2";
            if ($1 eq "BASE") {
                $base = $3*1;
            } elsif ($1 eq "TOP") {
                $top = $3*1;
            }
        } elsif ( $string =~ s/(OVC|BKN|SCT|FEW)?[- ]?(BASE|ABV|TOP|BLW)[- ]?([0-6][0-9][0-9])// ) {
            $cvg = "$1" if (defined($1));
            if ($2 eq "BASE" || $2 eq "ABV") {
                $base = $3*1;
            } elsif ($2 eq "TOP" || $2 eq "BLW") {
                $top = $3*1;
            }
        } elsif ( $string =~ s/\b([0-6][0-9][0-9])? ?(OVC|BKN|SCT|FEW) ?([0-6][0-9][0-9])?\b// ) {
            $cvg = "$2";
            if (defined($1) && defined($3)) {
                $base = $1*1;
                $top = $3*1;
            } elsif (defined($1)) {
                $base = $1*1;
            } elsif (defined($3)) {
                $base = $3*1;
            }
        } elsif ( $string =~ s/(OBSC|OBSCD|W ?[O0] ?X ?[O0] ?F|VV0[0-9][0-9])// ) {
            $cvg = "OVX";
        } elsif ( $string =~ s/C ?A ? V? O ?K// ) {
            $cvg = "CAVOK";
        } elsif ( $string =~ s/\b(CA ?VU|CLR? ?ABV?|CLEAR ABV?|CIR? ABV?|SKC|CA)\b// ) {
            $cvg = "SKC";
        } elsif ( $string =~ s/\bCLR\b// ) {
            $cvg = "CLR";
        } elsif ( $string =~ s/\b(IMC|IFR|VFR|VMC)\b// ) {
            $cvg = "$1";
        } elsif ( $string =~ s/\b([0-6][0-9][0-9])\b// ) {
            $base = $1;
        }

        for $n (1..4) {
            if ( ($pirepVars{"cldCvg$n"} eq "") && ($pirepVars{"cldBas$n"} == undef) && ($pirepVars{"cldTop$n"} == undef) ) {
                $pirepVars{"cldCvg$n"} = $cvg if $cvg;
                $pirepVars{"cldBas$n"} = $base if ($base >= 0);
                $pirepVars{"cldTop$n"} = $top if ($top >= 0);
                last;
            }
        }
    }

    $string =~ s/^\s+//;
    $string =~ s/\s+$//;
    $pirepVars{"leftOver"} .= ":SK $string" if $string;

    printf LOG ("\tSky1:\t%d %s %d\n", $pirepVars{"cldBas1"}, $pirepVars{"cldCvg1"}, $pirepVars{"cldTop1"}) if $verbose;
    printf LOG ("\tSky2:\t%d %s %d\n", $pirepVars{"cldBas2"}, $pirepVars{"cldCvg2"}, $pirepVars{"cldTop2"}) if ($verbose && $pirepVars{"cldCvg2"} ne "");
    printf LOG ("\tSky3:\t%d %s %d\n", $pirepVars{"cldBas3"}, $pirepVars{"cldCvg3"}, $pirepVars{"cldTop3"}) if ($verbose && $pirepVars{"cldCvg3"} ne "");

}

#
#+--+------------------------------------------------------------------+
#+--+------- DECODE WX INFO -------------------------------------------+
#
#...This group is supposed to have only obstructions to/and visib
#   but often has temperature or other stuff.  Most obstructions listed
#   below are rare but included anyway.

sub decode_wx {
    my ($string) = @_;
    my $wxString = "";
    my $visib = -1;
    my $cvg = "";
    my ($wxCaught, $wxPossible);

    #...------------...Allowed strings for present weather...-------

    my @WX = ('VCBLSN', 'VCBLSA', 'VCBLDU',                              # vicinity and blowing
              'VCPO', 'VCSS', 'VCDS', 'VCFG', 'VCSH', 'VCTS',            # vicinity
              'DRDU', 'DRSA', 'DRSN',                                    # low and drifting
              'BLDU', 'BLSA', 'BLPY', 'BLSN',                            # blowing
              'MIFG', 'PRFG', 'BCFG',                                    # shallow, partial, and patches
              '+SHRA', '-SHRA', 'SHRA', '+SHSN', '-SHSN', 'SHSN',        # showers of various stuff
              '+SHPE', '+SHPL', '-SHPE', '-SHPL', 'SHPE', 'SHPL', 'SHGR', 'SHGS',
              '+TSRA', '-TSRA', 'TSRA', '+TSSN', '-TSSN', 'TSSN',        # thunderstorms plus various stuff
              '+TSPE', '+TSPL', '-TSPE', '-TSPL', 'TSPE', 'TSPL', 'TSGR', 'TSGS',
              '+FZRA', '-FZRA', 'FZRA', '+FZDZ', '-FZDZ', 'FZDZ', 'FZFG',# freezing stuff
              '+FC', 'FC', 'SQ', 'VA',                                   # nasty stuff that kills/mames
              '+SS', 'SS', '+DS', 'DS', 'PO',                            # nuissance stuff obstructs vision
              'SA', 'DU', 'FU', 'HZ', 'BR', 'FG',                        # same as previous
              'TS', 'GS', 'GR',                                          # pretty cool stuff for chasers
              'IC', '+PE', '+PL', '-PE', '-PL', 'PE', 'PL',              # wintertime goodies
              '+SG', '-SG', 'SG', '+SN', '-SN', 'SN',                    # more winter fare
              '+RA', '-RA', 'RA', '+DZ', '-DZ', 'DZ');                   # drab precip types

    $string =~ s/[,\/]/ /g;
    $string =~ s/ {2,}/ /g;
    $string =~ tr/[A-Z0-9] \-+//cd;
    $string =~ s/FL([0-6][0-9][0-9])?//g;
    $string =~ s/TO?PS?//g;
    $string =~ s/UN?KN?//g;
    $string =~ s/OCNL//g;
    $string =~ s/([0-9])\+/$1/g;
    $string =~ s/(SM)\+/$1/g;
    $string =~ s/\+([0-9])/$1/g;
    $string =~ s/HA?ZE?/HZ/g;
    $string =~ s/FOG/FG/g;
    $string =~ s/\bASH\b/VA/g;
    $string =~ s/MIST/BR/g;
    $string =~ s/SMOKE/FU/g;
    $string =~ s/ {2,}/ /g;
    $string =~ s/^\s+//;
    $string =~ s/\s+$//;
    $string .= " ";

    #...Any wind info here

    if ( $string =~ s/\b([0-3][0-9]{2})(\d{2,3})KTS?// ) {
        $pirepVars{"wdir"} = $1*1;
        $pirepVars{"wspd"} = $2*1;
    }

    #...Any visibility info to be had

    if ($string =~ s/(FV|VSBY) ?([0-9]{1,2})(SM)? ?- ?([0-9]{1,3})(SM)?\b// ) {
        $visib = $2*1;
    } elsif ( $string =~ s/\b[M<](\d)\/(\d{1,2})([SK]M)// ) {
        $visib = $1 / $2 unless ($2 == 0);
        $visib *= $KM2SM if ($3 eq "KM");
    } elsif ( $string =~ s/\b(\d )?(\d)\/(\d{1,2})([SK]M)// ) {
        if ($1) {
            $visib = $1*1 + $2/$3 unless ($3 == 0);
        } else {
            $visib = $2/$3 unless ($3 == 0);
        }
        $visib *= $KM2SM if ($4 eq "KM");
    } elsif ($string =~ s/(FV|VSBY) ?([0-9]{1,3})([SK]M)?// ) {
        $visib = $2*1;
        $visib *= $KM2SM if ($3 eq "KM");
    } elsif ( $string =~ s/\bP?(\d{1,3})([SK]M)// ) {
        $visib = $1*1;
        $visib *= $KM2SM if ($2 eq "KM");
    }

    #...Check for temp info in the wrong place

    if ( $string =~ s/\b[M\-\+][0-9]{1,2}[FC]?\b// ) {
        &decode_ta("$&");
    }

    #...Possible to have certain sky conditions mixed in here

    $cvg = "";
    if ( $string =~ s/\b(CA ?VU|CLR? ?ABV?|CLEAR ABV?|CIR? ABV?|SKC|CA)\b// ) {
        $cvg = "SKC";
    } elsif ( $string =~ s/\bCLR\b// ) {
        $cvg = "CLR";
    } elsif ( $string =~ s/C ?A ? V? O ?K// ) {
        $cvg = "CAVOK";
    } elsif ( $string =~ s/\b(VFR|VMC|IMC|IFR)\b// ) {
        $cvg = "$1";
    } elsif ( $string =~ s/\b(OBSC|OBSCD|W ?[O0] ?X ?[O0] ?F|VV0[0-9][0-9])\b// ) {
        $cvg = "OVX";
    }
    if ($cvg ne "" && !($pirepVars{"cldCvg1"})) {
        $pirepVars{"cldCvg1"} = $cvg;
    } elsif ($cvg ne "" && !($pirepVars{"cldCvg2"})) {
        $pirepVars{"cldCvg2"} = $cvg;
    }

    #...Any obstructions to visibility or general weather to report?

    #...------------...Weather conditions (WMO code table 4678)...

    $string =~ s/ALQDS//;      # Eliminate false DS because some sites use TCU ALQDS (all quadrants)
    $string =~ s/TDS//;        # Eliminate false DS because Mexican sites use TDS to indicate "todos" = ALQDS
    $string =~ s/BLU\+?//;     # Eliminate false BL because Exxx and Lxxx use BLU to indicate ???
    $string =~ s/FCST//;       # Eliminate false FC from comments about FCST
    $string =~ s/GRN//;        # Eliminate false GR because Exxx use GRN to indicate ???
    $string =~ s/RERA//;       # Eliminate false RA because some sites use RERA to indicate ???

    if ($string =~ s/\bLTG(IC|CC|CG|GC)* //) {
        $wxString .= "VCTS ";
    }

    for (0..5) {
        if ( $string =~ s/ ?(VCBLSN|VCBLSA|VCBLDU|VCPO|VCSS|VCDS|VCFG|VCSH|VCTS) ?//) {
            $wxString .= "$1 ";
        } elsif ( $string =~ s/ ?(DRDU|DRSA|DRSN|BLDU|BLSA|BLPY|BLSN) ?//) {
            $wxString .= "$1 ";
        } elsif ( $string =~ s/ ?(MIFG|PRFG|BCFG) ?//) {
            $wxString .= "$1 ";
        } elsif ( $string =~ s/ ?(\+|-)?(FZFG|FZRA|FZDZ) ?//) {
            $wxString .= "$1$2 ";
        } elsif ( $string =~ s/ ?(\+|-)?(SH|TS)(RA|SN|PE|PL|GR|GS) ?//) {
            $wxString .= "$1$2$3 ";
        } elsif ( $string =~ s/ ?(\+|-|VC)?(PR|MI|BC|DR|BL|SH|TS|FZ|FZ|FZ)?(DZ|RA|SN|SG|IC|PE|PL|GR|GS|UP)?(BR|FG|FU|VA|DU|SA|HZ|PY)?(PO|SQ|FC|SS|DS)? ?// ) {
            $wxString .= "$1$2$3$4$5 ";
        } else {
            last;
        }
        $wxString =~ s/\s{1,}/ /g;              # collapse multiple spaces to a single space.
    }

    #...------------...For some reason, line above does not always catch
    #...------------...all chunks of wx.  Therefore, do again looking
    #...------------...for accepted strings.  This may change the order
    #...------------...of different wx types from original input.

    for (0..3) {
        $wxCaught = 0;
        foreach $wxPossible (@WX) {
            $wxPossible =~ s%^([\+-])%\\$1%;
            if ( $string =~ s/ ?($wxPossible) ?// ) {
                $wxString .= "$1 ";
                $wxCaught = 1;
                last;
            }
        }
        last unless $wxCaught;
        $wxString =~ s/\s{1,}/ /g;              # collapse multiple spaces to a single space.
    }

    $wxString =~ s/[\-\+][^A-Z]//g;             # A plus or minus by itself is unintended.
    $wxString =~ s/ $//;
    $wxString =~ s/^ //;

    print LOG "\tVisibility:\t$visib\n" if ($verbose && $visib>=0);
    print LOG "\tVis obstruction (WX):\t$wxString\n" if ($verbose && $wxString);

    $pirepVars{"visib"} .= $visib if ($visib >= 0);
    $pirepVars{"wxString"} .= "$wxString" if $wxString;

    $string =~ s/^\s+//;
    $string =~ s/\s+$//;
    $pirepVars{"leftOver"} .= ":WX $string" if $string;

}

#
#+--+------------------------------------------------------------------+
#+--+------- DECODE TA INFO -------------------------------------------+
#
#...Temperature is supposed to be given in Centigrade but if "F" is indicated
#   then convert to C.  Also convert likely Farenheit (large) values (>30).

sub decode_ta {
    my ($string) = @_;
    my $temp = -99;

    if ( $string =~ s/FL ?([0-6][0-9][0-9]( ?\- ?[0-6][0-9][0-9])?)// ) {
        &decode_fl($1) unless (defined($pirepVars{"fltLvl"}));
    }
    $string =~ s/MS?([0-9])/-$1/;
    $string =~ tr/[CF0-9] \-+//cd;
    $string =~ s/^\s+//;
    $string =~ s/\s+$//;

    if ( $string =~ s/([\-\+])?([0-9]{1,2})([FC])?\b// ) {
        $temp = $2*1;
        $temp *= -1.0 if (defined($1) && ($1 eq '-') );
        $temp = nint(($temp-32)*(5.0/9.0)) if (($temp > 30) || (defined($3) && ($3 eq 'F')) );
    }

    print LOG "\tTemperature:\t$temp\n" if ($verbose && $temp > -99);

    $pirepVars{"temp"} = $temp if ($temp > -99);

    $string =~ s/^\s+//;
    $string =~ s/\s+$//;
    $pirepVars{"leftOver"} .= ":TA $string" if $string;

}

#
#+--+------------------------------------------------------------------+
#+--+------- DECODE WV INFO -------------------------------------------+
#
#...Wind info must be either 5/6 digit string (060013) or ENE 13.

sub decode_wv {
    my ($string) = @_;
    my $wdir = -1;
    my $wspd = -1;

    $string =~ s/(\d)K(NO)?TS?/$1/g;
    $string =~ s/FL ?\d+//g;
    $string =~ s/\d+ ?KT ([HT][EA]?[AI]?[DL]WI?ND)//g;
    # $string =~ s/\D//g;

    if ( $string =~ s/\bE?([0-3][0-9]{2})[ \/]?(\d{2,3})\b// ) {
        $wdir = $1*1;
        $wspd = $2*1;
    } elsif ( $string =~ s/\b([NESW]{1,3}) ?(\d{2,3})\b// ) {
        $wdir = $compassPts{"$1"};
        $wspd = $2*1;
    }

    print LOG "\tWind dir/spd:\t$wdir $wspd\n" if ($verbose && $wspd >= 0);

    $pirepVars{"wdir"} = $wdir if ($wdir >= 0);
    $pirepVars{"wspd"} = $wspd if ($wspd >= 0);

    $string =~ s/^\s+//;
    $string =~ s/\s+$//;
    $pirepVars{"leftOver"} .= ":WV $string" if $string;

}

#
#+--+------------------------------------------------------------------+
#+--+------- DECODE TB INFO -------------------------------------------+
#
sub decode_tb {
    my ($string) = @_;
    my ($int_keys, $int_key, $intWord, $int, $intNum, $alt_keys);
    my ($fl1, $fl2, $prev, $post, $currFltLvl);
    my (@tokens, $token, $i, $k, $tokenCounter, $nextToken );
	my (%turbGroup, @turbGroups, $turbGroupIndex, $tbElementFoundCounter, $groupNumber);
	my ($hasType, $hasIntensity, $hasAltitude, $hasDuration );
	my ($durationMatch, $altitudeMatch, $tokenCounter, $tokenLen, $index);
	
    #...Force certain abbreviations to make parsing simpler later

    $string =~ s/\b(CONT?I?N?U?O?U?S?|FRE?QU?E?N?T?|FQT|STEA?D?Y?)\b/CONT/g;
    $string =~ s/\bINTE?R?M?I?T?T?E?N?T?\b/INT/g;
    $string =~ s/\b(OCAS?S?I?O?N?A?L?|OCN)\b/OCNL/g;
    $string =~ s/\bISOL?A?T?E?D?\b/ISOL/g;

    $string =~ s/(\b0\b|NON?E?|NIL|NEGATIVE)/NEG/g;
    # Replace SMOOTH, etc. with NEG, since there is no enum for SMTH in the Pireps database.
    #$string =~ s/(SMOOTH|SMO|SMTH?)/SMTH/g;
    $string =~ s/(SMOOTH|SMO|SMTH?)/NEG/g;
    $string =~ s/(SVR|SEVE?R?E?)/SEV/g;
    $string =~ s/EXT?R?E?M?E?/EXTM/g;
    $string =~ s/(MDT|MODE?R?A?T?E?)/MOD/g;
    $string =~ s/HEAV?Y?/HVY/g;
    $string =~ s/LI?G?H?T/LGT/g;
    
    $string =~ s/LGT[ -](ISOL|OCNL)[ -]MOD/LGT-MOD/g;
    $string =~ s/MOD[ -](ISOL|OCNL)[ -]SEV/MOD-SEV/g;
    #Include the handling of the 'TO' modifier.
    $string =~ s/LGT( ?TO ?)(ISOL|OCNL) MOD/LGT-MOD/g;
    $string =~ s/MOD( ?TO ?)(ISOL|OCNL) SEV/MOD-SEV/g;

    $string =~ s/T ?\/ ?M/T-M/g;                # Change LGT/MOD to LGT-MOD; protect split coming soon
    $string =~ s/D ?\/ ?S/D-S/g;                # Change MOD/SEV to MOD-SEV
    $string =~ s/V ?\/ ?E/V-E/g;                # Change SEV/EXTM to SEV-EXTM
    $string =~ s/([0-9]) ?\/ ?([0-9])/$1-$2/g;  # Change 040/070 to 040-070; protect split coming next
    #Insert a whitespace before any front slash, in case we have a group that is 
    #missing an identifier (eg /TB LGT CHOP/ZLC).
    $string =~ s/(\/)/ $1/g;
    $string = &preprocessString($string);
    $string =~ s/\b([0-9][0-9])\b/0$1/g;      # Change 2-digit numbers to 3 with leading zero
    $string =~ s/TURB?C? ?//g;
    $string =~ s/\bMECH ?//g;
    $string =~ s/\bCHP\b/CHOP/g;
    $string =~ s/\b(MTN? WA?VE?|MWAV)\b/MWAVE/g;
    $string =~ s/-/_/g;                         # Replace dash with underscore since "-" counts as word break
    $string =~ s/,/\./g;                         # Replace any comma with a period to avoid confusion when generating the CSV file.
    $string =~ s/ABV ([0-6][0-9][0-9])/ABV_$1/g; # Combine ABV #### with ABV_### 
    $string =~ s/BLW ([0-6][0-9][0-9])/BLW_$1/g; #Combine BLW #### with BLW_###

    #...Construct string of possible severities from @tbKeys

    $int_keys = "";
    foreach $int_key (@tbKeys) {
        ($intWord, $int, $intNum) = split (/;/, $int_key);
        $int_keys .= $intWord . "|";
    }
    $int_keys =~ s/\|$//;

    #...Various possible vertical level indicators

    $alt_keys = '(([0-6][0-9][0-9]) ?_ ?([0-6][0-9][0-9]))' . "|";
    $alt_keys .= '(SFC ?_ ?([0-6][0-9][0-9]))' . "|";
    $alt_keys .= '(([0-6][0-9][0-9]) ?_ ?SFC)' . "|";
    $alt_keys .= '(ABV_([0-6][0-9][0-9]))' . "|";
    $alt_keys .= '(BLW_([0-6][0-9][0-9]))' . "|";
    $alt_keys .= '([0-6][0-9][0-9])';

    #...Allow two distinct levels with differing TB intensity

    #ADDS-420
    #Processing of the turbulence group(s).
    #Tokenize the pre-processed string and determine which tokens are altitudes, intensities, duration, and type.
    #Then assign the tokens to the appropriate turbulence group.  For tokens with the pattern: intensity duration intensity,  the
    #duration is grouped with the second intensity and these are assigned to the second turbulence group.  If no altitude is found, apply the
    #altitude decoded from the /FL group.  If a second turbulence group is lacking an explicit type, use the type from the first
    #turbulence group.

    #...Groupings sometimes separated by whitespace or a period (actually a comma, but we replaced all commas with a period to avoid 
    #   any confusion when the CSV file is input to the database).
	@tokens = split( / |\./, $string );
	$tokenLen = scalar(@tokens);
	$tokenCounter = 0;
	$hasType      = 0;
	$hasIntensity = 0;
	$hasAltitude  = 0;
	$hasDuration  = 0;
	$index = 0;
	
	#Set aside the altitude decoded from the /FL to be used if altitudes are not
	#explicitly given in the /TB group.
	
	$currFltLvl = $pirepVars{"fltLvl"};
	$fl1 = $fl2 = -1;

    #Index used to differentiate the two allowed turbulence groups.
	$turbGroupIndex = 0;
    
    #Initialize the array hash turbGroups, which is used to hold the intensity, type, altitude, and duration tokens.
    
	for $i ( 0 .. 1 ) {
		$turbGroups[$i]{"type"}      = "";
		$turbGroups[$i]{"intensity"} = "";
		$turbGroups[$i]{"altitude"}  = "";
		$turbGroups[$i]{"duration"}  = "";
	}

    #Evaluate each token and place into the appropriate turbulence group, if possible.
	foreach $token (@tokens) {
		#Check for turbulence type
		if ( $token =~ m/\b(CHOP|CAT|MWAVE|LLWS)/ ) {
			if ( $hasType == 1 ) {
                #If a type was already found, then this type belongs to the second turbulence group.
                #Store the type in the next turbGroup array.
				$index = $turbGroupIndex + 1;
				$turbGroups[$index]{"type"} = $1;
			}
			else { 
				#First occurrence of a type token, this goes into the first turbulence group.
				$hasType = 1;
				$turbGroups[$turbGroupIndex]{"type"} = $1;
			}
		}
		#Check for turbulence intensity
		elsif ( $token =~ m/\b($int_keys)/ ) {
			if ( $hasIntensity == 1 ) {
                #If an intensity has already been found, save this intensity in the second turbulence group.
				$index = $turbGroupIndex + 1;
				$turbGroups[$index]{"intensity"} = $1;
			}
			else {
				#First occurrence of the intensity token, this goes into the first turbulence group.
				$hasIntensity = 1;
				$turbGroups[$turbGroupIndex]{"intensity"} = $1;
			}
		}
		#Check for altitude(s).
		elsif ( $token =~ m/(\b$alt_keys)/ ) {
			#Check for turbulence altitude/flight levels
			$altitudeMatch = $1;
			if ( $hasAltitude == 1 ) {
                #If an altitude has already been found, assign this to the second turbulence group.
				$index = $turbGroupIndex + 1;
				$turbGroups[$index]{"altitude"} = $altitudeMatch;
			}
			else {
				#First occurrence of an altitude token, this goes into the first turbulence group.
				$hasAltitude = 1;
				$turbGroups[$turbGroupIndex]{"altitude"} = $altitudeMatch;
			}
		}
		#Check for duration.
		elsif ( $token =~ m/(CONT|INT|OCNL|ISOL)/ ) {
			$durationMatch = $1;

			#Check for the turbulence duration
			if ( $hasDuration == 1 ) {
				#This duration token goes into the second turbulence group.
				$index = $turbGroupIndex + 1;
				$turbGroups[$index]{"duration"} = $durationMatch;
			}else {
                #Determine whether this is part of the next turbulence group by checking for any following intensity tokens.
                #If any token following this is an intensity, then this token and the remaining tokens belong to the second
                #turbulence group.
				$nextToken = $tokenCounter + 1;
				for $k ( $nextToken .. $tokenLen - 1 ) {
					if (   ( $tokens[$k] =~ m/\b($int_keys)/ )&& ( $hasIntensity == 1 ) )
					{
					    #This duration token goes into the second turbulence group.
						$index = $turbGroupIndex + 1;
						$turbGroups[$index]{"duration"} = $durationMatch;
					}
					else {
 					    #This duration token goes into the first turbulence group.
						$turbGroups[$turbGroupIndex]{"duration"} = $durationMatch;
					}
				}
				$hasDuration = 1;
			}
		}
		$tokenCounter++;
	} 
	
   
   #Assign the correct type, intensity, duration, and altitude to the turbulenceGroups array hash. Use the following rules:
   #   1. If a turbulence group is missing an altitude, use the altitude decoded from the /FL group.
   #   2. If a turbulence group is missing a duration, leave the duration undefined (i.e. DO NOT "carry over" the duration from a previous turbulence group).
   #   3. If the second turbulence group is missing a type, apply/"carry over" the first turbulence group's type.
   #Obtain the flight levels by calling parseAltitudes, and start decoding each turbulence group.
   #Check if this is an empty turbulence group before applying any of the above rules.

    $tbElementFoundCounter = 0;
	for $k ( 0 .. 1 ) {
		$groupNumber = $k + 1;
		if (   !$turbGroups[$k]{"type"} eq "" || !$turbGroups[$k]{"intensity"} eq "" || !$turbGroups[$k]{"duration"}  eq "" )
		{
			$tbElementFoundCounter++;
		}

        #Check for missing altitude if this turbulence group has at least one other entity (i.e. intensity, type, or
        #duration).
		if ( $turbGroups[$k]{"altitude"} eq "" && $tbElementFoundCounter > 0 ) {
			print LOG "Altitude missing for turb group $groupNumber, use altitude decoded from /FL group: $currFltLvl\n " if $verbose;
			#if the altitude consists of only two digits, pad with a leading zero.
			my $paddedFltLvl = $currFltLvl;
			if( $paddedFltLvl =~ m/(\d{3})/){
				$turbGroups[$k]{"altitude"} = $currFltLvl;
			}elsif( $paddedFltLvl =~ s/(\d{2})/sprintf('%03d',$1)/ge ){
				$turbGroups[$k]{"altitude"} = $paddedFltLvl;
			}
		}
		
		#Determine flight levels from the altitude.
	    ( $fl1, $fl2 ) = &parseAltitudes( $turbGroups[$k]{"altitude"}, $alt_keys );

		#Check for a missing type in the second turbulence group that isn't empty (i.e. has at least an intensity).
		if ( $turbGroups[1]{"type"} eq "" && $turbGroups[1]{"intensity"} ne "") {
			#Use the type from the first turbulence group.
			$turbGroups[1]{"type"} = $turbGroups[0]{"type"};
		}

		print LOG "Turbulence groups and their values (type, intensity, altitude, duration), index $k: \n" if $verbose;
		print LOG "Type:" , $turbGroups[$k]{"type"},      " " if $verbose;
		print LOG "Intensity:" , $turbGroups[$k]{"intensity"}, " " if $verbose;
		print LOG "Altitude: ", $turbGroups[$k]{"altitude"},  " " if $verbose;
		print LOG "Duration: ", $turbGroups[$k]{"duration"},  "\n" if $verbose;

		#Now assign the appropriate values to the pirepVars hash.
		$pirepVars{"tbInt$groupNumber"}  = $turbGroups[$k]{"intensity"} ;
		$pirepVars{"tbType$groupNumber"} = $turbGroups[$k]{"type"};
		$pirepVars{"tbFreq$groupNumber"} = $turbGroups[$k]{"duration"};
		
		
		
		if ( $fl1 >= 0 && $fl2 >= 0  ){
			$pirepVars{"tbBas$groupNumber"} = $fl1;
			$pirepVars{"tbTop$groupNumber"} = $fl2;
			print LOG "pirepVars tbBase: ", $pirepVars{"tbBas$groupNumber"}, "tbTop: ", $pirepVars{"tbTop$groupNumber"}, "\n";
		}
		elsif ( $fl1 >= 0 ) {
			$pirepVars{"tbBas$groupNumber"} = $fl1;
		}

		#Re-set the tbElementCounter for evaluating the next turbulence group.
		$tbElementFoundCounter = 0;

	}
    
    printf LOG ("\tTurbulence1:\t%d %s %d (%s)\n", $pirepVars{"tbBas1"}, $pirepVars{"tbInt1"}, $pirepVars{"tbTop1"}, $pirepVars{"tbType1"}) if ($verbose && $pirepVars{"tbInt1"} ne "");
    printf LOG ("\tTurbulence2:\t%d %s %d (%s)\n", $pirepVars{"tbBas2"}, $pirepVars{"tbInt2"}, $pirepVars{"tbTop2"}, $pirepVars{"tbType2"}) if ($verbose && $pirepVars{"tbInt2"} ne "");

}

#
#+--+------------------------------------------------------------------+
#+--+------- DECODE IC INFO -------------------------------------------+
#
sub decode_ic {
    my ($string) = @_;
    my ($int_keys, $int_key, $intWord, $int, $intNum, $alt_keys);
    my ($fl1, $fl2, $prev, $post, $currFltLvl);
    my (@tokens, $token, $i, $k, $tokenCounter, $nextToken );
	my (%icgGroup, @icgGroups, $icgGroupIndex, $icgElementFoundCounter, $groupNumber);
	my ($hasType, $hasIntensity, $hasAltitude, $hasDuration );
	my ($durationMatch, $altitudeMatch, $tokenCounter, $tokenLen, $index);

    #...Force certain abbreviations to make parsing simpler later

    $string =~ s/\b(CONT?I?N?U?O?U?S?|FRE?QU?E?N?T?|FQT|STEA?D?Y?)\b/CONT/g;
    $string =~ s/\bINTE?R?M?I?T?T?E?N?T?\b/INT/g;
    $string =~ s/\b(OCAS?S?I?O?N?A?L?|OCN)\b/OCNL/g;
    $string =~ s/\bISOL?A?T?E?D?\b/ISOL/g;

    $string =~ s/(\b0\b|NON?E?|NIL|NEGATIVE)/NEG/g;
    # Replace SMOOTH, etc. with NEG, since there is no enum for SMTH in the Pireps database.
    #$string =~ s/(SMOOTH|SMO|SMTH?)/SMTH/g;
    $string =~ s/(SMOOTH|SMO|SMTH?)/NEG/g;
    $string =~ s/(SVR|SEVE?R?E?)/SEV/g;
    $string =~ s/EXT?R?E?M?E?/EXTM/g;
    $string =~ s/(MDT|MODE?R?A?T?E?)/MOD/g;
    $string =~ s/HEAV?Y?/HVY/g;
    $string =~ s/LI?G?H?T/LGT/g;
    $string =~ s/TRA?C?E?/TRC/g;
    $string =~ s/TRC[ -](ISOL|OCNL)[ -]LGT/TRC-LGT/g;
    $string =~ s/LGT[ -](ISOL|OCNL)[ -]MOD/LGT-MOD/g;
    $string =~ s/MOD[ -](ISOL|OCNL)[ -]SEV/MOD-SEV/g;
    #Include handling of the 'TO' modifier.
    $string =~ s/TRC( ?TO ?)(ISOL|OCNL) LGT/TRC-LGT/g;
    $string =~ s/LGT( ?TO ?)(ISOL|OCNL) MOD/LGT-MOD/g;
    $string =~ s/MOD( ?TO ?)(ISOL|OCNL) SEV/MOD-SEV/g;

    $string =~ s/C ?\/ ?L/C-L/g;                # Change TRC/LGT to TRC-LGT; protect split coming soon
    $string =~ s/T ?\/ ?M/T-M/g;                # Change LGT/MOD to LGT-MOD
    $string =~ s/D ?\/ ?S/D-S/g;                # Change MOD/SEV to MOD-SEV
    $string =~ s/V ?\/ ?E/V-E/g;                # Change SEV/EXTM to SEV-EXTM
    $string =~ s/RIME? ?[\-\/] ?MI?XE?D?/MIXED/g;   # Change RIME/MIXED to MIXED
    $string =~ s/MI?XE?D? ?[\-\/] ?RIME?/MIXED/g;   # Change RIME/MIXED to MIXED
    $string =~ s/([0-9]) ?\/ ?([0-9])/$1-$2/g;  # Change 040/070 to 040-070; protect split coming next
    $string =~ s/,/\./g;                         # Replace any comma with a period to avoid confusion when generating the CSV file.
    #Insert a whitespace before any front slash, in case we have a group that is 
    #missing an identifier (eg /IC LGT MIX/ZLC).
    $string =~ s/(\/)/ $1/g;
    $string = &preprocessString($string);
    $string =~ s/\b([0-9][0-9])\b/0$1/g;        # Change 2-digit numbers to 3 with leading zero
    $string =~ s/ ?IC(IN)?G(IC)?(IP)? ?/ /g;
    $string =~ s/\b(RIM|RME)\b/RIME/g;
    $string =~ s/\bCLE?A?R\b/CLEAR/g;
    $string =~ s/\bMI?XE?D?\b/MIXED/g;
    $string =~ s/-/_/g;                         # Replace dash with underscore since "-" counts as word break

    #...Construct string of possible severities from @icKeys

    $int_keys = "";
    foreach $int_key (@icKeys) {
        ($intWord, $int, $intNum) = split (/;/, $int_key);
        $int_keys .= $intWord . "|";
    }
    $int_keys =~ s/\|$//;

    #...Various possible vertical level indicators

    $alt_keys = '(([0-6][0-9][0-9]) ?_ ?([0-6][0-9][0-9]))' . "|";
    $alt_keys .= '(SFC ?_ ?([0-6][0-9][0-9]))' . "|";
    $alt_keys .= '(([0-6][0-9][0-9]) ?_ ?SFC)' . "|";
    $alt_keys .= '(ABV_([0-6][0-9][0-9]))' . "|";
    $alt_keys .= '(BLW_([0-6][0-9][0-9]))' . "|";
    $alt_keys .= '([0-6][0-9][0-9])';

    #...Allow two distinct levels with differing IC intensity
    #ADDS-420 Although this bug addresses the incorrect assignment of flight levels to turbulence
    #groups, the same logic which caused the issue in turbulence is also inherent in the decoding of icing.
    #Tokenize the pre-processed string and determine which tokens are altitudes, intensities, duration, and type.
    #Then assign the tokens to the appropriate icing group.  For tokens with the pattern: intensity duration intensity,  the
    #duration is grouped with the second intensity and these are assigned to the second icing group.  If no altitude is found, apply the
    #altitude decoded from the /FL group.  If a second icing group is lacking an explicit type, use the type from the first
    #icing group.

	@tokens = split( / |\./, $string );
	$tokenLen = scalar(@tokens);
	$tokenCounter = 0;
	$hasType      = 0;
	$hasIntensity = 0;
	$hasAltitude  = 0;
	$hasDuration  = 0;
	$index = 0;
	
	#Set aside the altitude decoded from the /FL to be used if altitudes are not
	#explicitly given in the /IC group.
	
	$currFltLvl = $pirepVars{"fltLvl"};
	$fl1 = $fl2 = -1;

    #Index used to differentiate the two allowed icing groups.
	$icgGroupIndex = 0;
    
    #Initialize the array hash icgGroups, which is used to hold the intensity, type, altitude, and duration tokens.
    
	for $i ( 0 .. 1 ) {
		$icgGroups[$i]{"type"}      = "";
		$icgGroups[$i]{"intensity"} = "";
		$icgGroups[$i]{"altitude"}  = "";
		$icgGroups[$i]{"duration"}  = "";
	}

    #Evaluate each token and place into the appropriate turbulence group, if possible.
	foreach $token (@tokens) {
		#Check for icing type
		if ( $token =~ m/\b(RIME|CLEAR|MIXED)\b/ ) {
			if ( $hasType == 1 ) {
                #If a type was already found, then this type belongs to the second icing group.
                #Store the type in the next icgGroup array.
				$index = $icgGroupIndex + 1;
				$icgGroups[$index]{"type"} = $1;
			}
			else {
				#First occurrence of a type token, this goes into the first icing group.
				$hasType = 1;
				$icgGroups[$icgGroupIndex]{"type"} = $1;
			}
		}
		#Check for icing intensity
		elsif ( $token =~ m/\b($int_keys)/ ) {
			if ( $hasIntensity == 1 ) {
                #If an intensity has already been found, save this intensity in the second icing group.
				$index = $icgGroupIndex + 1;
				$icgGroups[$index]{"intensity"} = $1;
			}
			else {
				#First occurrence of the intensity token, this goes into the first icing group.
				$hasIntensity = 1;
				$icgGroups[$icgGroupIndex]{"intensity"} = $1;
			}
		}
		#Check for altitude(s).
		elsif ( $token =~ m/(\b$alt_keys)/ ) {
			#Check for icing altitude/flight levels
			$altitudeMatch = $1;
			if ( $hasAltitude == 1 ) {
                #If an altitude has already been found, assign this to the second icing group.
				$index = $icgGroupIndex + 1;
				$icgGroups[$index]{"altitude"} = $altitudeMatch;
			}
			else {
				#First occurrence of an altitude token, this goes into the first icing group.
				$hasAltitude = 1;
				$icgGroups[$icgGroupIndex]{"altitude"} = $altitudeMatch;
			}
		}
		#Check for duration.
		elsif ( $token =~ m/(CONT|INT|OCNL|ISOL)/ ) {
			$durationMatch = $1;

			#Check for the icing duration
			if ( $hasDuration == 1 ) {
				#This duration token goes into the second icing group.
				$index = $icgGroupIndex + 1;
				$icgGroups[$index]{"duration"} = $durationMatch;
			}else {
                #Determine whether this is part of the next icing group by checking for any following intensity tokens.
                #If any token following this is an intensity, then this token and the remaining tokens belong to the second
                #icing group.
				$nextToken = $tokenCounter + 1;
				for $k ( $nextToken .. $tokenLen - 1 ) {
					if (   ( $tokens[$k] =~ m/\b($int_keys)/ )&& ( $hasIntensity == 1 ) )
					{
					    #This duration token goes into the second icing group.
						$index = $icgGroupIndex + 1;
						$icgGroups[$index]{"duration"} = $durationMatch;
					}
					else {
 					    #This duration token goes into the first icing group.
						$icgGroups[$icgGroupIndex]{"duration"} = $durationMatch;
					}
				}
				$hasDuration = 1;
			}
		}
		$tokenCounter++;
	}

   #Assign the correct type, intensity, duration, and altitude to the icgGroups array hash. Use the following rules:
   #   1. If an icing group is missing an altitude, use the altitude decoded from the /FL group.
   #   2. If an icing group is missing a duration, leave the duration undefined (i.e. DO NOT "carry over" the duration from a previous icing group).
   #   3. If the second icing group is missing a type, apply/"carry over" the first icing group's type (only if it isn't an emtpy group, ie. there is at least an intensity).
   #Obtain the flight levels by calling parseAltitudes, and start decoding each icing group.
   #Check if this is an empty icing group before applying any of the above rules.

    $icgElementFoundCounter = 0;
	for $k ( 0 .. 1 ) {
		$groupNumber = $k + 1;
		if (   !$icgGroups[$k]{"type"} eq "" || !$icgGroups[$k]{"intensity"} eq "" || !$icgGroups[$k]{"duration"}  eq "" )
		{
			$icgElementFoundCounter++;
		}

        #Check for missing altitude if this icing group has at least one other entity (i.e. intensity, type, or
        #duration).
			if ( ($icgGroups[1]{"type"} eq "" &&  $icgGroups[1]{"intensity"} ne "") ||($icgGroups[1]{"type"} eq "" && $icgGroups[1]{"duration"} ne "")){
			print LOG "Altitude missing for icing group $groupNumber, use altitude decoded from /FL group: $currFltLvl\n " if $verbose;
			#if the altitude consists of only two digits, pad with a leading zero.
			my $paddedFltLvl = $currFltLvl;
			if( $paddedFltLvl =~ m/(\d{3})/){
				$icgGroups[$k]{"altitude"} = $currFltLvl;
			}elsif( $paddedFltLvl =~ s/(\d{2})/sprintf('%03d',$1)/ge ){
				$icgGroups[$k]{"altitude"} = $paddedFltLvl;
			}
		}
		
		#Determine flight levels from the altitude.
	    ( $fl1, $fl2 ) = &parseAltitudes( $icgGroups[$k]{"altitude"}, $alt_keys );

		#Check for a missing type in the second icing group where this isn't an empty group (i.e. there is at least an intensity).
		if ( $icgGroups[1]{"type"} eq "" && $icgGroups[1]{"intensity"} ne "") {
			#Use the type from the first icing group.
			$icgGroups[1]{"type"} = $icgGroups[0]{"type"};
		}

		print LOG "Icing groups and their values (type, intensity, altitude, duration), index $k: \n" if $verbose;
		print LOG $icgGroups[$k]{"type"},      " " if $verbose;
		print LOG $icgGroups[$k]{"intensity"}, " " if $verbose;
		print LOG $icgGroups[$k]{"altitude"},  " " if $verbose;
		print LOG $icgGroups[$k]{"duration"},  "\n" if $verbose;

		#Now assign the appropriate values to the pirepVars hash.
		$pirepVars{"icgInt$groupNumber"}  = $icgGroups[$k]{"intensity"};
		$pirepVars{"icgType$groupNumber"} = $icgGroups[$k]{"type"};
		$pirepVars{"icgFreq$groupNumber"} = $icgGroups[$k]{"duration"};
		
		if ( $fl1 >= 0 && $fl2 >= 0 ) {
			$pirepVars{"icgBas$groupNumber"} = $fl1;
			$pirepVars{"icgTop$groupNumber"} = $fl2;
			print LOG "pirepVars icBase: ", $pirepVars{"icgBas$groupNumber"}, " tbTop: ", $pirepVars{"icgTop$groupNumber"}, "\n";
		}
		elsif ( $fl1 >= 0 ) {
			$pirepVars{"icgBas$groupNumber"} = $fl1;
		}

		#Re-set the icgElementCounter for evaluating the next icing group.
		$icgElementFoundCounter = 0;

	}
       

    printf LOG ("\tIcing1:\t%d %s %d (%s)\n", $pirepVars{"icgBas1"}, $pirepVars{"icgInt1"}, $pirepVars{"icgTop1"}, $pirepVars{"icgType1"}) if ($verbose && $pirepVars{"icgInt1"} ne "");
    printf LOG ("\tIcing2:\t%d %s %d (%s)\n", $pirepVars{"icgBas2"}, $pirepVars{"icgInt2"}, $pirepVars{"icgTop2"}, $pirepVars{"icgType2"}) if ($verbose && $pirepVars{"icgInt2"} ne "");

}

#
#+--+------------------------------------------------------------------+
#+--+------- DECODE RM INFO -------------------------------------------+
#
sub decode_rm {
    my ($string) = @_;
    
    $string =~ s/\,/./;                         # Replace any comma with a period.  Additional commas cause problems when the csv is read into the database.

    $string = &preprocessString($string);

    #...As a last resort, use FL info found here if none known already.

    if ( $string =~ s/\b(FL)?([0-6][0-9][0-9](\-[0-6][0-9][0-9])?)\b// ) {
        &decode_fl($2) unless (defined($pirepVars{"fltLvl"}));
    }

    #...If on final approach, assume close to ground and only use if fltLvl not known.

    if ( $string =~ s/\b(FAP|FINAL|FNL APRCH)\b// ) {
        &decode_fl("001 AGL") unless (defined($pirepVars{"fltLvl"}));
    }

    #...Any visibility info stuck in remarks?

    if ($string =~ s/(FV|VSBY) ?[0-9]+([KS]M)?// ) {
        &decode_wx("$&") unless (defined($pirepVars{"visib"}));
    }

    #...Any additional sky attributes?

    if ( $string =~ s/\b(CA ?VU|CLR? ?ABV?|CLEAR ABV?|CIR? ABV?|CA)\b// ) {
        &decode_sk("$&");
    }

    #...Fill in icing type in case we have icing intensity but not type from IC group.

    if ( $string =~ s/\b(RIME|CLEAR|MIXED)\b// ) {
        $pirepVars{"icgType1"} = "$1" if ($pirepVars{"icgType1"} eq "" && $pirepVars{"icgInt1"} ne "");
    }

    #...Fill in turb type in case we have icing intensity but not type from TB group.

    if ( $string =~ s/\b(LLWS|CAT|CHOP)\b// ) {
        $pirepVars{"tbType1"} = "$1" unless $pirepVars{"tbType1"};
    } elsif ( $string =~ s/\bMTN? ?WA?VE?\b// ) {
        $pirepVars{"tbType1"} = "$1" unless $pirepVars{"tbType1"};
    }

}

#
#+--+------------------------------------------------------------------+
#+--+------- ASSUMPTIONS ----------------------------------------------+
#
sub assumptions {

    my $topSky = 600;

    #...Do whatever we can to find flight level, set qcField as indicator.
    #...If no known flight level, take alt from SK, then IC, then TB group.

    unless (defined($pirepVars{"fltLvl"})) {

        if (defined($pirepVars{"cldBas1"}) && defined($pirepVars{"cldTop1"}) && $pirepVars{"cldTop1"} < $topSky) {
            $pirepVars{"fltLvl"} = nint( ($pirepVars{"cldBas1"} + $pirepVars{"cldTop1"})*0.5);
            $pirepVars{"qcField"} += 16;
            $noFlightLevelFlag = 1;
        } elsif (defined($pirepVars{"cldBas1"})) {
            $pirepVars{"fltLvl"} = $pirepVars{"cldBas1"};
            $pirepVars{"qcField"} += 16;
            $noFlightLevelFlag = 1;
        } elsif (defined($pirepVars{"cldTop1"}) && $pirepVars{"cldTop1"} < $topSky) {
            $pirepVars{"fltLvl"} = $pirepVars{"cldTop1"};
            $pirepVars{"qcField"} += 16;
            $noFlightLevelFlag = 1;
        } elsif (defined($pirepVars{"cldBas2"}) && defined($pirepVars{"cldTop2"}) && $pirepVars{"cldTop2"} < $topSky) {
            $pirepVars{"fltLvl"} = nint( ($pirepVars{"cldBas2"} + $pirepVars{"cldTop2"})*0.5);
            $pirepVars{"qcField"} += 16;
            $noFlightLevelFlag = 1;
        } elsif (defined($pirepVars{"cldBas2"})) {
            $pirepVars{"fltLvl"} = $pirepVars{"cldBas2"};
            $pirepVars{"qcField"} += 16;
            $noFlightLevelFlag = 1;
        } elsif (defined($pirepVars{"cldTop2"}) && $pirepVars{"cldTop2"} < $topSky) {
            $pirepVars{"fltLvl"} = $pirepVars{"cldTop2"};
            $pirepVars{"qcField"} += 16;
            $noFlightLevelFlag = 1;
        } elsif (defined($pirepVars{"icgBas1"}) && defined($pirepVars{"icgTop1"}) && $pirepVars{"icgTop1"} < $topSky) {
            $pirepVars{"fltLvl"} = nint( ($pirepVars{"icgBas1"} + $pirepVars{"icgTop1"})*0.5);
            $pirepVars{"qcField"} += 16;
            $noFlightLevelFlag = 1;
        } elsif (defined($pirepVars{"icgBas1"})) {
            $pirepVars{"fltLvl"} = $pirepVars{"icgBas1"};
            $pirepVars{"qcField"} += 16;
            $noFlightLevelFlag = 1;
        } elsif (defined($pirepVars{"icgTop1"}) && $pirepVars{"icgTop1"}<$topSky) {
            $pirepVars{"fltLvl"} = $pirepVars{"icgTop1"};
            $pirepVars{"qcField"} += 16;
            $noFlightLevelFlag = 1;
        } elsif (defined($pirepVars{"icgBas2"}) && defined($pirepVars{"icgTop2"}) && $pirepVars{"icgTop2"} < $topSky) {
            $pirepVars{"fltLvl"} = nint( ($pirepVars{"icgBas2"} + $pirepVars{"icgTop2"})*0.5);
            $pirepVars{"qcField"} += 16;
            $noFlightLevelFlag = 1;
        } elsif (defined($pirepVars{"icgBas2"})) {
            $pirepVars{"fltLvl"} = $pirepVars{"icgBas2"};
            $pirepVars{"qcField"} += 16;
            $noFlightLevelFlag = 1;
        } elsif (defined($pirepVars{"icgTop2"}) && $pirepVars{"icgTop2"}<$topSky) {
            $pirepVars{"fltLvl"} = $pirepVars{"icgTop2"};
            $pirepVars{"qcField"} += 16;
            $noFlightLevelFlag = 1;
        } elsif (defined($pirepVars{"tbBas1"}) && defined($pirepVars{"tbTop1"})) {
            $pirepVars{"fltLvl"} = nint( ($pirepVars{"tbBas1"} + $pirepVars{"tbTop1"})*0.5);
            $pirepVars{"qcField"} += 16;
            $noFlightLevelFlag = 1;
        } elsif (defined($pirepVars{"tbBas1"})) {
            $pirepVars{"fltLvl"} = $pirepVars{"tbBas1"};
            $pirepVars{"qcField"} += 16;
            $noFlightLevelFlag = 1;
        } elsif (defined($pirepVars{"tbTop1"})) {
            $pirepVars{"fltLvl"} = $pirepVars{"tbTop1"};
            $pirepVars{"qcField"} += 16;
            $noFlightLevelFlag = 1;
        } elsif (defined($pirepVars{"tbBas2"}) && defined($pirepVars{"tbTop2"})) {
            $pirepVars{"fltLvl"} = nint( ($pirepVars{"tbBas2"} + $pirepVars{"tbTop2"})*0.5);
            $pirepVars{"qcField"} += 16;
            $noFlightLevelFlag = 1;
        } elsif (defined($pirepVars{"tbBas2"})) {
            $pirepVars{"fltLvl"} = $pirepVars{"tbBas2"};
            $pirepVars{"qcField"} += 16;
            $noFlightLevelFlag = 1;
        } elsif (defined($pirepVars{"tbTop2"})) {
            $pirepVars{"fltLvl"} = $pirepVars{"tbTop2"};
            $pirepVars{"qcField"} += 16;
            $noFlightLevelFlag = 1;
        }
    }

    #...If we still do not have FL but found LLWS, assume arbitrary low altitude

    if ($pirepVars{"tbType1"} eq "LLWS" && !defined($pirepVars{"fltLvl"})) {
        &decode_fl("001 AGL");
    }

    #...If we have a clear sky indicator, set bases and top to match.

    if ($pirepVars{"cldCvg1"} eq "SKC") {
        $pirepVars{"cldBas1"} = $pirepVars{"fltLvl"};
        $pirepVars{"cldTop1"} = $topSky;
    } elsif ($pirepVars{"cldCvg2"} eq "SKC") {
        $pirepVars{"cldBas2"} = $pirepVars{"cldTop1"};
        $pirepVars{"cldTop2"} = $topSky;
    } elsif ($pirepVars{"cldCvg3"} eq "SKC") {
        $pirepVars{"cldBas3"} = $pirepVars{"cldTop2"};
        $pirepVars{"cldTop3"} = $topSky;
    } elsif ($pirepVars{"cldCvg4"} eq "SKC") {
        $pirepVars{"cldBas4"} = $pirepVars{"cldTop3"};
        $pirepVars{"cldTop4"} = $topSky;
    }


    #...If we have a clear sky indicator, set icing to "NEGclr" type.

    if ($pirepVars{"cldCvg1"} eq "SKC" && !($pirepVars{"icgInt2"})) {
        $pirepVars{"icgInt2"} = "NEGclr";
        $pirepVars{"icgBas2"} = $pirepVars{"cldBas1"};
        $pirepVars{"icgTop2"} = $pirepVars{"cldTop1"};
    } elsif ($pirepVars{"cldCvg2"} eq "SKC" && !($pirepVars{"icgInt2"})) {
        $pirepVars{"icgInt2"} = "NEGclr";
        $pirepVars{"icgBas2"} = $pirepVars{"cldBas2"};
        $pirepVars{"icgTop2"} = $pirepVars{"cldTop2"};
    } elsif ($pirepVars{"cldCvg3"} eq "SKC" && !($pirepVars{"icgInt2"})) {
        $pirepVars{"icgInt2"} = "NEGclr";
        $pirepVars{"icgBas2"} = $pirepVars{"cldBas3"};
        $pirepVars{"icgTop2"} = $pirepVars{"cldTop3"};
    } elsif ($pirepVars{"cldCvg4"} eq "SKC" && !($pirepVars{"icgInt2"})) {
        $pirepVars{"icgInt2"} = "NEGclr";
        $pirepVars{"icgBas2"} = $pirepVars{"cldBas4"};
        $pirepVars{"icgTop2"} = $pirepVars{"cldTop4"};
    } elsif ( ($pirepVars{"cldCvg1"} eq "CLR" || $pirepVars{"cldCvg1"} eq "VFR" || $pirepVars{"cldCvg1"} eq "VMC" || $pirepVars{"cldCvg1"} eq "CAVOK") && !($pirepVars{"icgInt2"}) ) {
        $pirepVars{"icgInt2"} = "NEGclr";
        $pirepVars{"icgBas2"} = $pirepVars{"cldBas1"};
        $pirepVars{"icgTop2"} = $pirepVars{"cldTop1"};
    } elsif ($pirepVars{"visib"} >= 6 && !($pirepVars{"icgInt2"})) {
        $pirepVars{"icgInt2"} = "NEGclr";
        $pirepVars{"icgBas2"} = $pirepVars{"fltLvl"};
    }

    return;

}

#
#+--+------------------------------------------------------------------+
#
sub preprocessString {
    my ($string) = @_;

    #...Alter some things that just confound parsing.

#   $string =~ s/\b([0-9][0-9])\b/0$1/g;        # Change 2-digit numbers to 3 with leading zero
                                                # actually this is not a good thing to do.
    $string =~ s/([0-6][0-9][0-9])E/$1/g;       # Drop letter E indicating "estimated"
    $string =~ s/\bE ?([0-6][0-9][0-9])/$1/g;

    $string =~ s/ ?EXCE?P?T? ?/ /g;             # Get rid of troublesome EXCEPT
    $string =~ s/ ?EXTREMELY ?/ /g;             # Get rid of this troublemaker

    $string =~ s/GR?O?U?ND/SFC/g;               # Same thing but parsed more easily
    $string =~ s/ TO /-/g;                      # ditto
    $string =~ s/(FL|[^C]AT|AOB|OTP|FM)//g;     # Rest of these things just get in the way
    $string =~ s/UN?KN?//g;                     # and do not add valuable info
    $string =~ s/BTW?N ?//g;
    $string =~ s/(BELOW|BLO|BLW)/BLW/g;
    $string =~ s/(ABOVE|ABOV?)/ABV/g;
    $string =~ s/([0-9]) ?[AM]SL/$1/g;
    $string =~ s/(DURG?[CD]|[AD]S?CNT) ?//g;
    $string =~ s/(CLI?MB?|DE?PA?R?T) ?//g;
    $string =~ s/XXX//g;

    $string =~ tr/[A-Z0-9 \-]//cd;              # Delete all chars except A-Z0-9, space and dash
    $string =~ s/ {2,}/ /g;                     # Collapse multi-spaces into single
    $string =~ s/^\s+//;                        # Strip leading whitespace
    $string =~ s/\s+$//;                        # Strip trailing whitespace
   

    return $string;
}

#
#+--+------------------------------------------------------------------+
#
sub parseAltitudes {
    my ($string, $alt_keys) = @_;
    my ($alt1, $alt2);

#   $alt_keys = '(([0-6][0-9][0-9]) ?_ ?([0-6][0-9][0-9]))' . "|";      # $1, $2, $3,
#   $alt_keys .= '(SFC ?_ ?([0-6][0-9][0-9]))' . "|";                   # $4, $5,
#   $alt_keys .= '(([0-6][0-9][0-9]) ?_ ?SFC)' . "|";                   # $6, $7,
#   $alt_keys .= '(ABV_([0-6][0-9][0-9]))' . "|";                       # $8, $9,
#   $alt_keys .= '(BLW_([0-6][0-9][0-9]))' . "|";                       # $10, $11,
#   $alt_keys .= '([0-6][0-9][0-9])';                                   # $12

    $string =~ m/$alt_keys/;

    #Revert any ABV_XXX and BLW_XXX to ABV XXX and BLW XXX, respectively.
	$string =~ s/(ABV_([0-6][0-9][0-9]))/ABV $2/g;
	$string =~ s/(BLW_([0-6][0-9][0-9]))/BLW $2/g;
	
    if (defined($1)) {
        $alt1 = &min($2*1, $3*1);
        $alt2 = &max($2*1, $3*1);
    } elsif (defined($4)) {
        $alt1 = 0;
        $alt2 = $5*1;
    } elsif (defined($6)) {
        $alt1 = 0;
        $alt2 = $7*1;
    } elsif (defined($8)) {
        $alt1 = $9*1;
        $alt2 = $pirepVars{"fltLvl"} if ($pirepVars{"fltLvl"} > $alt1);
    } elsif (defined($10)) {
        $alt2 = $11*1;
        $alt1 = $pirepVars{"fltLvl"} if ($pirepVars{"fltLvl"} < $alt2);
    } elsif (defined($12)) {
        $alt1 = $12*1;
        $alt2 = -1;
    }
    return ($alt1, $alt2);
}

#
#+--+------------------------------------------------------------------+
#
sub decode_airep {
    my ($line) = @_;
    my ($ld, $tp, $ov, $tm, $fl, $ta, $wv);
    my (@nwaTbCodes);
    @nwaTbCodes = ( "NEG", "OCNL LGT", "LGT", "LGT-MOD", "MOD", "MOD-SEV", "SEV", "EXTM");

    print LOG "Inside decode_airep, Decoding AIREP: $line\n";
    $line =~ s/^\n? ?ARP //;
    $ld = "";

#...Get ac-flightNumber, lat, lon, time, fl, temp, and wind info

    if ( $line =~ s/([A-Z0-9]{2,7}) (\d{2,5}[NS] ?\d{2,5}[WE]?) (\d{4}) F?(\d{3})//) {
        $tp = $1;
        $ov = $2;
        $tm = $3;
        $fl = $4;
        &decode_ov($ld, $ov) if ($ld || $ov);
        &decode_tm($tm);
        &decode_fl($fl);
        &decode_tp($tp);

        if ( $line =~ s/ ([MS]{0,2}[0-9\/X]{0,2} )?([0-9\/X]{3}[ \/]?[0-9\/X]{2,3})(KT)?// ) {
            $ta = $1 if (defined($1));
            $wv = $2;
            $wv =~ s/\D//g;        #...Take out any non-digits from wind info
            &decode_ta($ta);
            &decode_wv($wv);
        }

        if ( $line =~ s/TB // ) {
            &decode_tb($line) if (length($line) > 0);
        } elsif ( $line =~ s/(TB )?CODE// ) {
            $line =~ s/ZERO/0/;
            $line =~ s/ONE/1/;
            $line =~ s/TWO/2/;
            $line =~ s/THREE/3/;
            $line =~ s/FOUR/4/;
            $line =~ s/FIVE/5/;
            $line =~ s/SIX/6/;
            $line =~ s/SEVEN/7/;
            $line =~ s/EIGHT/8/;
            $line =~ s/NINE/9/;
            if ($tp =~ m/(NWA|UAL|JAL)/) {
                $line =~ s/ ([1-7])\b/$nwaTbCodes[$1]/;
            }
            &decode_tb($line) if (length($line) > 0);
        }

        return 1;

    } else {
        warn "WARNING, format of AIREP not a match, cannot decode:$line\n";
        return 0;
    }

}

#
#+--+------------------------------------------------------------------+
#
sub decode_amdar {
    my ($line) = @_;
    my ($phase, $tp, $ov, $tm, $fl, $ta, $td, $rh, $wv, $ude);
    my (@tbCodes);
    @tbCodes = ( "NEG", "LGT", "MOD", "SEV");

    #...Get phase of flight (level, unstable, ascending, descending).

    $line =~ s/(LV.|UNS|ASC|DES|\/\/\/) //;
    $phase = $1 if (defined($1));

    #...Get ac-flightNumber, lat, lon, time, fl

    if ( $line =~ s/\b([A-Z]{2,3}[0-9]{3,4}([A-Z]{2,3})?) +(\d{4}[NS] ?\d{5}[WE]?) +([0-3][0-9])?([0-2][0-9][0-5][0-9]) +([AF])([0-6][0-9][0-9])// ) {
        $tp = $1;
        $ov = $3;
        $tm = $5;
        $fl = $7*1;
        $fl *= -1 if ($6 eq "A");
        $pirepVars{"fltLvl"} = $fl;
        &decode_ov("", $ov);
        &decode_tm($tm);
        $pirepVars{"acType"} = $tp;

        #...Get temperature and wind info if there

        if ( $line =~ s/\b([MP]S[0-9\/X]{3}) +(([MP]S[0-9\/X]{3})|([0-9\/X]{3} ))?([0-9\/X]{3}[ \/]?[0-9\/X]{2,3})(KT)?// ) {
            $ta = $1;
            $td = $2 if defined($2);  #...Dewpoint or humidity possible here, but skip for now.
            $wv = $5;
            if ($ta =~ /([MP])S([0-9]{3})/) {
                $pirepVars{"temp"} = nint($2/10);
                $pirepVars{"temp"} *= -1 if ($1 eq "M");
            }
            $wv =~ s/\D//g;           #...Take out any non-digits from wind info
            &decode_wv($wv) if $wv;
        }

        #...Get turbulence info if there

        if ( $line =~ s/\bTB([0-3\/])\b// ) {
            &decode_tb("$tbCodes[$1]") unless ($1 eq "/");
        }

        #...Bypass but strip out the navagation system/type/temp precision
        #...Code tables 3866, 3867, 3868

        $line =~ s/\bS([0-1][0-5][0-1])\b//;

        #...Section 3 (indicated by "333")
        #...Equivalent vertical gust (Ude) (tengths meters per second)

        $line =~ s/\b333 +([AF][0-9\/]{3} +)?VG([0-9][0-9][0-9])\b//;
        $ude = $2*1;
        $pirepVars{"vertGust"} = $ude;

        return 1;

    } else {
        warn "WARNING, format of AMDAR not a match, cannot decode:$line\n";
        return 0;
    }


}

#........................................................................#

sub initialize {
    my ($icaoId1, $bulletin_time, $pirep) = @_;

    $pirepVars{"obsTime"}  = $bulletin_time;
    $pirepVars{"qcField"}  = 2;
    $pirepVars{"icaoId"}   = $icaoId1;
    $pirepVars{"acType"}   = "";
    $pirepVars{"lat"}      = -99.99;
    $pirepVars{"lon"}      = -999.99;
    $pirepVars{"elev"}     = 0;
    $pirepVars{"fltLvl"}   = undef;
    $pirepVars{"cldCvg1"}  = "";
    $pirepVars{"cldCvg2"}  = "";
    $pirepVars{"cldCvg3"}  = "";
    $pirepVars{"cldCvg4"}  = "";
    $pirepVars{"cldBas1"}  = undef;
    $pirepVars{"cldBas2"}  = undef;
    $pirepVars{"cldBas3"}  = undef;
    $pirepVars{"cldBas4"}  = undef;
    $pirepVars{"cldTop1"}  = undef;
    $pirepVars{"cldTop2"}  = undef;
    $pirepVars{"cldTop3"}  = undef;
    $pirepVars{"cldTop4"}  = undef;
    $pirepVars{"visib"}    = undef;
    $pirepVars{"wxString"} = "";
    $pirepVars{"temp"}     = undef;
    $pirepVars{"wdir"}     = undef;
    $pirepVars{"wspd"}     = undef;
    $pirepVars{"icgBas1"}  = undef;
    $pirepVars{"icgTop1"}  = undef;
    $pirepVars{"icgInt1"}  = "";
    $pirepVars{"icgType1"} = "";
    $pirepVars{"icgBas2"}  = undef;
    $pirepVars{"icgTop2"}  = undef;
    $pirepVars{"icgInt2"}  = "";
    $pirepVars{"icgType2"} = "";
    $pirepVars{"tbBas1"}   = undef;
    $pirepVars{"tbTop1"}   = undef;
    $pirepVars{"tbInt1"}   = "";
    $pirepVars{"tbType1"}  = "";
    $pirepVars{"tbFreq1"}  = "";
    $pirepVars{"tbBas2"}   = undef;
    $pirepVars{"tbTop2"}   = undef;
    $pirepVars{"tbInt2"}   = "";
    $pirepVars{"tbType2"}  = "";
    $pirepVars{"tbFreq2"}  = "";
    $pirepVars{"vertGust"} = undef;
    $pirepVars{"rawOb"}    = "$pirep";
    $pirepVars{"reportType"} = "PIREP";
    $pirepVars{"leftOver"} = "";

    return %pirepVars;
}

#........................................................................#

sub printOutput {
    my ($headerPrint) = @_;
    my ($unix_time, $lat, $lon, $fl_level, $sk_bas1, $sk_cvg1, $sk_top1, $sk_bas2, $sk_cvg2, $sk_top2, $visib, $obstruction, $temp, $wdir, $wspd, $ic_bas1, $ic_top1, $ic_int1, $ic_typ1, $ic_bas2, $ic_top2, $ic_int2, $ic_typ2, $tb_bas1, $tb_top1, $tb_frq1, $tb_int1, $tb_typ1, $tb_bas2, $tb_top2, $tb_frq2, $tb_int2, $tb_typ2, $airc_type, $full_pirep);
    my (%old_cldCvg, %old_icgInt, %old_icgTyp, %old_tbInt, %old_tbTyp, %old_tbFrq);

    %old_cldCvg = (
                   "VMC", -1,
                   "VFR", -1,
                   "CAVOK", -1,
                   "CLR", -1,
                   "SKC", -2,
                   "FEW", 1,
                   "SCT", 2,
                   "BKN", 5,
                   "OVC", 8,
                   "OVX", 9,
                   "IMC", 9,
                   "IFR", 9,
                  );

    %old_icgInt = (
                   "NEG", -1,
                   "NEGclr", -2,
                   "TRC", 1,
                   "TRC-LGT", 2,
                   "LGT", 3,
                   "LGT-MOD", 4,
                   "MOD", 5,
                   "MOD-SEV", 6,
                   "HVY", 6,
                   "SEV", 7,
                  );

    %old_icgTyp = ("RIME", 1,  "CLEAR", 2,  "MIXED", 3);

    %old_tbInt =  (
                   "NEG", -1,
                   "SMTH-LGT", 1,
                   "LGT", 2,
                   "LGT-MOD", 3,
                   "MOD", 4,
                   "MOD-SEV", 5,
                   "SEV", 6,
                   "SEV-EXTM", 7,
                   "EXTM", 8,
                  );

    %old_tbTyp =  ("CAT", 2,  "CHOP", 1,   "LLWS", 3,  "MWAVE", 4);

    %old_tbFrq =  ("ISOL", 1,  "OCNL", 1,   "INT", 2,  "CONT", 3);

    if ($headerPrint) {
        select (OUTFILE);
        write OUTFILE_HEAD;
        select (LOG);
    } else {
        $full_pirep = $pirepVars{"rawOb"};
        $unix_time = $pirepVars{"obsTime"};
        $airc_type = ($pirepVars{"acType"})? $pirepVars{"acType"} : "UNKN";
        $lat = $pirepVars{"lat"};
        $lon = $pirepVars{"lon"};
        $fl_level = defined($pirepVars{"fltLvl"})? $pirepVars{"fltLvl"} : -9;
        $sk_bas1 = defined($pirepVars{"cldBas1"})? $pirepVars{"cldBas1"} : -9;
        $sk_cvg1 = ($pirepVars{"cldCvg1"})? $old_cldCvg{$pirepVars{"cldCvg1"}} : -9;
        $sk_top1 = defined($pirepVars{"cldTop1"})? $pirepVars{"cldTop1"} : -9;
        $sk_bas2 = defined($pirepVars{"cldBas2"})? $pirepVars{"cldBas2"} : -9;
        $sk_cvg2 = ($pirepVars{"cldCvg2"})? $old_cldCvg{$pirepVars{"cldCvg2"}} : -9;
        $sk_top2 = defined($pirepVars{"cldTop2"})? $pirepVars{"cldTop2"} : -9;
        $visib = defined($pirepVars{"visib"})? $pirepVars{"visib"} : -9;
        $obstruction = ($pirepVars{"wxString"})? 1 : -9;
        $temp = defined($pirepVars{"temp"})? $pirepVars{"temp"} : -99;
        $wdir = defined($pirepVars{"wdir"})? $pirepVars{"wdir"} : -9;
        $wspd = defined($pirepVars{"wspd"})? $pirepVars{"wspd"} : -9;
        $ic_bas1 = defined($pirepVars{"icgBas1"})? $pirepVars{"icgBas1"} : -9;
        $ic_top1 = defined($pirepVars{"icgTop1"})? $pirepVars{"icgTop1"} : -9;
        $ic_int1 = ($pirepVars{"icgInt1"})? $old_icgInt{$pirepVars{"icgInt1"}} : -9;
        $ic_typ1 = ($pirepVars{"icgType1"})? $old_icgTyp{$pirepVars{"icgType1"}} : -9;
        $ic_bas2 = defined($pirepVars{"icgBas2"})? $pirepVars{"icgBas2"} : -9;
        $ic_top2 = defined($pirepVars{"icgTop2"})? $pirepVars{"icgTop2"} : -9;
        $ic_int2 = ($pirepVars{"icgInt2"})? $old_icgInt{$pirepVars{"icgInt2"}} : -9;
        $ic_typ2 = ($pirepVars{"icgType2"})? $old_icgTyp{$pirepVars{"icgType2"}} : -9;
        $tb_bas1 = defined($pirepVars{"tbBas1"})? $pirepVars{"tbBas1"} : -9;
        $tb_top1 = defined($pirepVars{"tbTop1"})? $pirepVars{"tbTop1"} : -9;
        $tb_int1 = ($pirepVars{"tbInt1"})? $old_tbInt{$pirepVars{"tbInt1"}} : -9;
        $tb_typ1 = ($pirepVars{"tbType1"})? $old_tbTyp{$pirepVars{"tbType1"}} : -9;
        $tb_frq1 = ($pirepVars{"tbFreq1"})? $old_tbFrq{$pirepVars{"tbFreq1"}} : -9;
        $tb_bas2 = defined($pirepVars{"tbBas2"})? $pirepVars{"tbBas2"} : -9;
        $tb_top2 = defined($pirepVars{"tbTop2"})? $pirepVars{"tbTop2"} : -9;
        $tb_int2 = ($pirepVars{"tbInt2"})? $old_tbInt{$pirepVars{"tbInt2"}} : -9;
        $tb_typ2 = ($pirepVars{"tbType2"})? $old_tbTyp{$pirepVars{"tbType2"}} : -9;
        $tb_frq2 = ($pirepVars{"tbFreq2"})? $old_tbFrq{$pirepVars{"tbFreq2"}} : -9;
        write OUTFILE;
    }

    #
    #...Formatted output - the header is currently commented out in the code
    #...but the line below represents the correct columns of information.
    #...Output formatted digital data is 137 chars wide and the full raw
    #...PIREP begins at character number 138 and is variable length.
    #...Example input and output are provided below.
    #

format OUTFILE_HEAD =
#UNIX_TIME|      |       |FL |   SKY1   |   SKY2   |    WEATHER      |   ICING 1   |   ICING 2   |     TURB 1     |     TURB 2     |AIRC| FULL RAW
#  UTC    |  LAT |  LON  |LVL|BAS CG TOP|BAS CG TOP|VS OB TMP WDR WSP|BAS TOP IN TY|BAS TOP IN TY|BAS TOP FR IN TY|BAS TOP FR IN TY|TYPE|  PIREP
#_________|______|_______|___|___|__|___|___|__|___|__|__|___|___|___|___|___|__|__|___|___|__|__|___|___|__|__|__|___|___|__|__|__|____|__________
#23456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567
.

format OUTFILE =
@######### @##.## @###.## @## @## @# @## @## @# @## @# @# @## @## @## @## @## @# @# @## @## @# @# @## @## @# @# @# @## @## @# @# @# @<<< @*
$unix_time, $lat, $lon, $fl_level, $sk_bas1, $sk_cvg1, $sk_top1, $sk_bas2, $sk_cvg2, $sk_top2, $visib, $obstruction, $temp, $wdir, $wspd, $ic_bas1, $ic_top1, $ic_int1, $ic_typ1, $ic_bas2, $ic_top2, $ic_int2, $ic_typ2, $tb_bas1, $tb_top1, $tb_frq1, $tb_int1, $tb_typ1, $tb_bas2, $tb_top2, $tb_frq2, $tb_int2, $tb_typ2, $airc_type, $full_pirep
.

#
#...Example of a raw PIREP (without the leading "#"):
#
#MFR UA /OV MFR/TM 1755/FL110/TP C210/SK 070OVC-TOPS090/CA/TA M12/WV 250034KT/WX FV99SM/TB NEG 070-110/IC LGT RIME 070-090

#...Corresponding decoded version of the above (excluding the leading "#"):
#
# 921261300  42.37 -122.87 110  70  8  90  90  0 600 99  1 -12 250  34  70  90  3  1  90 600 -1 -9  70 110 -9 -1 -9  -9 -9 -9 -9 -9 C210 MFR UA /OV MFR/TM 1755/FL110/TP C210/SK 070OVC-TOPS090/CA/TA M12/WV 250034KT/WX FV99SM/TB NEG 070-110/IC LGT RIME 070-090

    return;

}


#........................................................................#

sub construct_timestamp {
    my ($imin, $ihour, $iday, $t1, $t2, $timeStr, $icaoId) = @_;
    use vars qw ($year $month $day $hour $min $sec);
    my ($try1, $try2, $oldest, $futurist, $timestamp, $yr, $mo, $dy);

    $try1 = &timegm($sec,$min,$hour,$day,$month-1,$year);
    $oldest = $try1 + $t1*3600;
    $futurist = $try1 + $t2*3600;

    $timestamp = &my_timegm(0,$imin,$ihour,$iday,$month-1,$year);

    if ($timestamp < 0) {
        if ($month > 1) {
            $timestamp = &my_timegm(0,$imin,$ihour,$iday,$month-2,$year);
        } else {
            $timestamp = &my_timegm(0,$imin,$ihour,$iday,12-1,$year-1);
        }
    }

    if ($timestamp < $oldest) {
        if ($iday*1 == 1) {
            $mo = $month + 1;
            $yr = $year;
            if ($mo > 12) {
                $mo = 1;
                $yr += 1;
            }
            $timestamp = &my_timegm(0,$imin,$ihour,$iday,$mo-1,$yr);
            return 0 if ($timestamp < $oldest || $timestamp > $futurist);
        } else {
            $timestamp += 3600*24;
            if ($timestamp >= $oldest && $timestamp <= $futurist) {
                print LOG "AUTO, apparent timestamp typo (yesterday) within $timeStr [$icaoId], adjust (+1 day)\n";
            } else {
                $timestamp -= 3600*48;
                if ($timestamp >= $oldest && $timestamp <= $futurist) {
                    print LOG "AUTO, apparent timestamp typo (tomorrow) within $timeStr [$icaoId] adjust (-1 day)\n";
                } else {
                    return 0;
                }
            }
        }

    } elsif ($timestamp > $futurist) {
        $try2 = $timestamp - 3600*24;
        if ($try2 >= $oldest && $try2 <= $futurist) {
            print LOG "AUTO, apparent timestamp typo (tomorrow) within $timeStr [$icaoId] adjust (-1 day)\n";
            $timestamp = $try2;
        } elsif ($day*1 == 1) {
            $mo = $month - 1;
            $yr = $year;
            if ($mo < 1) {
                $mo = 12;
                $yr -= 1;
            }
            $timestamp = &my_timegm(0,$imin,$ihour,$iday,$mo-1,$yr);
            return 0 if ($timestamp < $oldest || $timestamp > $futurist);
        } else {
            $timestamp += 3600*24;
            if ($timestamp >= $oldest && $timestamp <= $futurist) {
                print LOG "AUTO, apparent timestamp typo (yesterday) within $timeStr [$icaoId] adjust (+1 day)\n";
            } else {
                $timestamp -= 3600*48;
                if ($timestamp >= $oldest && $timestamp <= $futurist) {
                    print LOG "AUTO, apparent timestamp typo (tomorrow) within $timeStr [$icaoId] adjust (-1 day)\n";
                } else {
                    return 0;
                }
            }
        }

    }

    return $timestamp;
}

#........................................................................#

sub my_timegm {
    my ($sec, $min, $hour, $day, $month, $year) = @_;
    my ($utime, $d, $m, $y);

    eval { $utime = &timegm($sec,$min,$hour,$day,$month,$year); };

    $utime = -1 unless ($@ eq "");

    ($d, $m, $y) = (gmtime($utime))[3..5];
    $utime = -1 if ($d != $day || $m != $month || $y+1900 != $year);

    return $utime;
}

#........................................................................#

sub atexit {

    my ($sig, $handle ) = @_ ;
    use vars qw ($prog $timeout);

    if ( $sig eq "eof" ) {
        print STDERR "EOF on STDIN -- awaiting more data\n" ;
    } elsif ( $sig eq "timeout" ) {
        print STDERR ("$prog shutting down, timeout = ", $timeout/60, " minutes\n");
    } elsif ( defined($sig) ) {
        print STDERR "\nCaught SIG$sig --shutting down\n\n" ;
    }

    close (STDOUT);
    close (STDERR);
    close (LOG);
    &Navaid::shutdown($handle);


    exit (0);
}

#........................................................................#

sub bynumber { $a <=> $b; }
sub max { my($a,$b) = @_; (($a <=> $b) > 0)? $a: $b; }
sub min { my($a,$b) = @_; (($a <=> $b) < 0)? $a: $b; }
sub nint { my ($z) = @_; return ($z>0)? int($z+0.5) : int($z-0.5); }

#........................................................................#
sub writePirep2Csv{

my ($obsTime, $icaoId, $acType, $lat, $lon, $fltLvl, $cldBas1, $cldTop1, $cldCvg1, $cldBas2, $cldTop2, $cldCvg2,
    $visib, $wxString, $temp, $wdir, $wspd, $icgBas1, $icgTop1, $icgInt1, $icgType1, $icgBas2, $icgTop2, 
    $icgInt2, $icgType2, $tbBas1, $tbTop1, $tbInt1, $tbType1, $tbFreq1, $tbBas2, $tbTop2, $tbInt2, $tbType2, $tbFreq2, 
    $vertGust, $pirepType, $rawOb, $qcField);

my ($headerDescriptor, $counter, $element, $numberOfPirepVariables );
my @qcConfidenceArray;

   #Check for and set undefined values to NULL, where applicable
   $headerDescriptor = "PIREP";
   $obsTime = $pirepVars{"obsTime"};
   $icaoId = $pirepVars{"icaoId"} ? $pirepVars{"icaoId"} : "";
   $acType = $pirepVars{"acType"} ? $pirepVars{"acType"} : "";
   $lat = $pirepVars{"lat"};
   $lon = $pirepVars{"lon"};
   $fltLvl = defined($pirepVars{"fltLvl"}) ? $pirepVars{"fltLvl"} : 0;
   $cldBas1 = defined($pirepVars{"cldBas1"}) ? $pirepVars{"cldBas1"} : "";
   $cldCvg1 = $pirepVars{"cldCvg1"};
   $cldTop1 = defined($pirepVars{"cldTop1"}) ? $pirepVars{"cldTop1"} : "";
   $cldBas2 = defined($pirepVars{"cldBas2"}) ? $pirepVars{"cldBas2"} : "";
   $cldCvg2 = $pirepVars{"cldCvg2"};
   $cldTop2 = defined($pirepVars{"cldTop2"}) ? $pirepVars{"cldTop2"} : "";
   $visib = defined($pirepVars{"visib"}) ? $pirepVars{"visib"} : "";
   $wxString = $pirepVars{"wxString"};
   $temp = defined($pirepVars{"temp"}) ? $pirepVars{"temp"} : "";
   $wdir = defined($pirepVars{"wdir"}) ? $pirepVars{"wdir"} : "";
   $wspd = defined($pirepVars{"wspd"}) ? $pirepVars{"wspd"} : "";
   $icgBas1 = defined($pirepVars{"icgBas1"}) ? $pirepVars{"icgBas1"} : "";
   $icgTop1 = defined($pirepVars{"icgTop1"}) ? $pirepVars{"icgTop1"} : "";
   $icgInt1 = $pirepVars{"icgInt1"};
   $icgType1 = $pirepVars{"icgType1"};
   $icgBas2 = defined($pirepVars{"icgBas2"}) ? $pirepVars{"icgBas2"} : "";
   $icgTop2 = defined($pirepVars{"icgTop2"}) ? $pirepVars{"icgTop2"} : "";
   $icgInt2 = $pirepVars{"icgInt2"};
   $icgType2 = $pirepVars{"icgType2"};
   $tbBas1 = defined($pirepVars{"tbBas1"}) ? $pirepVars{"tbBas1"} : "";
   $tbTop1 = defined($pirepVars{"tbTop1"}) ? $pirepVars{"tbTop1"} : "";
   $tbInt1 = $pirepVars{"tbInt1"};
   
   $tbType1 = $pirepVars{"tbType1"};
   $tbFreq1 = $pirepVars{"tbFreq1"};
   $tbBas2 = defined($pirepVars{"tbBas2"}) ? $pirepVars{"tbBas2"} : "";
   $tbTop2 = defined($pirepVars{"tbTop2"}) ? $pirepVars{"tbTop2"} : "";
   $tbInt2 = $pirepVars{"tbInt2"};
   $tbType2 = $pirepVars{"tbType2"};
   $tbFreq2 = $pirepVars{"tbFreq2"};
   $vertGust = ($pirepVars{"vertGust"}) ? $pirepVars{"vertGust"} : "";
   $rawOb = $pirepVars{"rawOb"};
   #Replace any commas in the rawOb with a '.'.
   $rawOb =~ s/,/./g;
   $pirepType = $pirepVars{"reportType"};
   $qcField = defined ($pirepVars{"qcField"}) ? $pirepVars{"qcField"} : "0";

   #Remove the quotation marks from wxString, icgInt, icgType, tbInt, tbType, tbFreq
   $wxString =~ s/"//g;
   $icgInt1 =~ s/"//g;
   $icgInt2 =~ s/"//g;
   $icgType1 =~ s/"//g;
   $icgType2 =~ s/"//g;
   $tbInt1 =~ s/"//g;
   $tbInt2 =~ s/"//g;
   $tbType1 =~ s/"//g;
   $tbType2 =~ s/"//g;
   $tbFreq1 =~ s/"//g;
   $tbFreq2 =~ s/"//g;
   
   #Replace any underscores with dashes, so they are consistent with the enums in the Pireps database table.
   $tbInt1 =~ s/_/-/g;
   $tbInt2 =~ s/_/-/g;
   $icgInt1 =~ s/_/-/g;
   $icgInt2 =~ s/_/-/g;
   
  
   #Remove leading whitespace(s) from obsTime and acType
   $obsTime =~ s/^\s+//;
   $acType =~ s/^\s+//;
   
   
   
   if ($lat == -99.99){
       $lat = "";
   }
   if ($lon == -999.99) {
       $lon = "";
   }

   
  my @pirepVarArray = ($headerDescriptor, $obsTime, $icaoId, $acType, $lat, $lon, $fltLvl, $cldBas1, $cldTop1, $cldCvg1, $cldBas2, $cldTop2, $cldCvg2, $visib, $wxString, $temp, $wdir, $wspd, $icgBas1, $icgTop1, $icgInt1, $icgType1, $icgBas2, $icgTop2, $icgInt2, $icgType2, $tbBas1, $tbTop1, $tbInt1, $tbType1, $tbFreq1, $tbBas2, $tbTop2, $tbInt2, $tbType2, $tbFreq2, $vertGust, $rawOb, $pirepType);
  $numberOfPirepVariables = @pirepVarArray - 1;
  my $pirepCsv = join(",",@pirepVarArray); 
  print STDOUT "$pirepCsv\n";

  #Obtain the qc/confidence values if they exist for this particular PIREP report
  if( $qcField > 0 ){
    @qcConfidenceArray = &initConfidenceQcValues($numberOfPirepVariables);
    &getQcConfidenceValues($qcField,@qcConfidenceArray);
  }
}

#........................................................................#
sub initConfidenceQcValues{
   my ($numVars) = @_;
   my @qcConfArray;
   my $qcHeaderDescriptor = "CONFIDENCE";
   #Initialize qc array with only the qcHeaderDescriptor
   my $i;
   $qcConfArray[0] = $qcHeaderDescriptor;
   for ( $i = 1; $i < $numVars ; $i++ ){
      $qcConfArray[$i] = " "; 
   }
   $qcConfArray[$numVars] = "";
   
   return @qcConfArray;
}

#........................................................................#

sub getQcConfidenceValues{
   my($qcField,@qcArray) = @_;
   my ($latLonQcValue, $elevQcValue, $timeQcValue);
   #Values associated with each qc field
   my $badLocationValue = 32;
   my $noFlightLevelValue = 16;
   my $aglValue = 8;
   my $flightLevelRangeValue = 4;
   my $noTimestampValue = 2; 
   my $midPointAssumedValue = 1;
   #QC values affect three groups: lat/lon, elevation and time
   $latLonQcValue = 0;
   $elevQcValue = 0;
   $timeQcValue = 0;

   #Assign the qc value to the correct field
   #Midpoint assumed pertains to the lat and lon field
   #No time stamp pertains to the obsTime
   #Flight level Range, AGL and No flight level pertatin to:
   #  fltLvl, cldbas1,cldbas2, cldTop1, cldTop2, icgBas1, icgBas2, icgTop1, icgTop2, tbBas1, tbBas2, tbTop1, tbTop2
   #Bad location pertains to lat and lon
   #For fields which can comprise of more than one qc value, sum up all the values
    
   if ( $badLocationFlag) {
       $latLonQcValue += $badLocationValue;
       #print LOG "latlon qc value (after bad location added) = $latLonQcValue\n" if $verbose;
   }
         
   if ( $midPointAssumedFlag ){
      $latLonQcValue += $midPointAssumedValue;
      #print LOG "latlon qc value (after midpoint assumed added) = $latLonQcValue\n" if $verbose;
   }

   if ( $flightLevelRangeFlag ){
       $elevQcValue += $flightLevelRangeValue;
       #print LOG "elev qc value (after adding flight level range): $elevQcValue\n" if $verbose;
   }
 
   if ( $aglFlag ) {
       $elevQcValue += $aglValue;
       #print LOG "elev qc value (after adding agl): $elevQcValue\n" if $verbose;
   }
       
   if ( $noFlightLevelFlag ){
       $elevQcValue += $noFlightLevelValue;
       #print LOG "elev qc value (after adding no flight level): $elevQcValue\n" if $verbose;
   } 
        
   if ( $noTimestampFlag ) {
       $timeQcValue += $noTimestampValue;
       #print LOG "time qc value: $timeQcValue\n" if $verbose;
   }
 
   #Apply these values to the appropriate PIREP fields
   #NOTE:
   #See fill_PirepsTable() in pirepCsv2Mysql.pl for the indexes corresponding to the field of interest
   #

   if ( $latLonQcValue > 0 ){
       #lat
       $qcArray[4] = $latLonQcValue;

       #lon
       $qcArray[5] = $latLonQcValue;
   }
          
   if( $elevQcValue > 0 ) {
       #fltLvl
       $qcArray[6] = $elevQcValue;

       #cldBas1
       $qcArray[7] = $elevQcValue;

       #cldTop1
       $qcArray[8] = $elevQcValue;

       #cldBas2
       $qcArray[10] = $elevQcValue;

       #cldTop2
       $qcArray[11] = $elevQcValue;

       #icgBas1
       $qcArray[18] = $elevQcValue;

       #icgTop1
       $qcArray[19] = $elevQcValue;

       #icgBas2
       $qcArray[22] = $elevQcValue;

       #icgTop2
       $qcArray[23] = $elevQcValue;

       #tbBas1
       $qcArray[26] = $elevQcValue;

       #tbTop1
       $qcArray[27] = $elevQcValue;

       #tbBas2
       $qcArray[31] = $elevQcValue;

       #tbTop2
       $qcArray[32] = $elevQcValue;
   }
     
       if( $timeQcValue > 0 ) {
          #obsTime
          $qcArray[1] = $timeQcValue;
       }
        
 my $confidenceQcCsv = join(",", @qcArray);
   #print LOG "\n********Confidence/QA************\n\n"; 
  # print LOG  "$confidenceQcCsv\n"; 
   #print STDOUT "$confidenceQcCsv\n";
}
