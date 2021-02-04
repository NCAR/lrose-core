#! /usr/bin/perl
#
#  This is an adaptation of the Unidata (www.unidata.ucar.edu)
#  Perl script to decode routine aviation weather report (METAR) 
#  data and create a NetCDF file.  The decoder has undergone
#  extensive modification and improvement particularly for
#  international METAR reports.  This code no longer creates
#  NetCDF formatted files.
#
#  The following entry in the LDM 'pqact.conf' file is used to
#  make this run:
#  WMO	^S(A....|P....) .... ([0-3][0-9])([0-2][0-9])	PIPE parseMetar.pl 	
#
#  Author:  Greg Thompson
#  based on original code by Robb Kambic (Unidata).
#
use Getopt::Std;
use Env qw(ADDSHOME);                    #...Get ADDS project dir
use MetarUtils;
no locale;

($prog = $0) =~ s%.*/%%;                 # Determine program basename.
umask 002;                               # Set file permissions.
$cur_time = time();                      # Get current time.
$timeout = 300;                          # Program will exit if stagnant this long.


#...------------...Usage Info...--------------------------------

$opt_v = $opt_h = 0;
$usage = <<EOF;
Usage: $prog [-hv] [-t yyyymmddhh] [-L log dir]
  -h  Help:     Print out this usage information.
  -v  Verbose:  Print out verbose debug information (CAUTION).
  -t  yyyymmddhh: adjust the year, month, and day otherwise current date.
  -L  log file dir: Specify where the log file lives.
  expects standard input to be parsed so redirect a file to STDIN if need be.
EOF

#...------------...Sanity check the options...------------------

&getopts('vh:t:L:') || die $usage;
die $usage if $opt_h;
$verbose = 1 if $opt_v;
if ($opt_t) {
    $YYYYMMDDHH = $opt_t;
    ($year, $month, $day, $hour) = $YYYYMMDDHH =~ /([12][90][0-9][0-9])([01][0-9])([0-3][0-9])([0-2][0-9])/;
    print "Expecting archive METARs for year=$year, month=$month, day=$day, and hour=$hour\n" if $verbose;
    if ($year<1900 || $year>2099 || $month<1 || $month>12 || $day<1 || $day>31 || $hour>23) {
        print STDERR "Incorrect date format ($YYYYMMDDHH)\n";
        die "Please try again\n\n";
    }
} else {
    $YYYYMMDDHH = 0;
}

#...------------...Redirect LOG/STDERROR...------------------


if ($opt_L) {
    if ($opt_L == "/dev/null"){
	open (LOG, ">/dev/null") or die "Can't open /dev/null: $!";
	open (STDERR, ">&LOG" ) or die "could not dup stderr: $!\n";
    }else{
	open (LOG, ">>$opt_L/parseMetarLog_$year$month$day$hour.log" ) or die "could not open $opt_L/parseMetarLog.$year$month$day$hour.log: $!\n";
	print LOG "log dir: $opt_L\n";
	select( LOG ); $| = 1;
	open (STDERR, ">&LOG" ) or die "could not dup stdout: $!\n";
	select( STDERR ); $| = 1;
    }
}
select( STDOUT ); $| = 1;

#...------------...Set interrupt handler...---------------------

$SIG{ 'INT' }   = 'atexit';
$SIG{ 'KILL' }  = 'atexit';
$SIG{ 'TERM' }  = 'atexit';
$SIG{ 'QUIT' }  = 'atexit';

#...------------...Constants...---------------------------------

$UNRESTRICTED_VIS = 10;
$UNRESTRICTED_VIS_M = 9999;
$KM2SM = 0.6214;
$M2SM = $KM2SM*0.001;
$FT2SM = 1.0/5280.;
$MPS2KT = 1.94;
$MPH2KT = 60.0/69.0;
$KPH2KT = (1000.0/3600.0)*$MPS2KT;
$INCHES2MB = 1016.0/30.0;             # 30.00 inches of mercury equals 1016.0 mb

#...------------...Allowed strings for present weather...-------

@WX = ('VCBLSN', 'VCBLSA', 'VCBLDU',                              # vicinity and blowing
       'VCPO', 'VCSS', 'VCDS', 'VCFG', 'VCSH', 'VCTS',            # vicinity
       'DRDU', 'DRSA', 'DRSN',                                    # low and drifting
       'BLDU', 'BLSA', 'BLPY', 'BLSN',                            # blowing
       'MIFG', 'PRFG', 'BCFG',                                    # shallow, partial, and patches
       '\Q+SHRA\E', '-SHRA', 'SHRA', '+SHSN', '-SHSN', 'SHSN',        # showers of various stuff
       '+SHPE', '+SHPL', '-SHPE', '-SHPL', 'SHPE', 'SHPL', 'SHGR', 'SHGS',
       '+TSRA', '-TSRA', 'TSRA', '+TSSN', '-TSSN', 'TSSN',        # thunderstorms plus various stuff
       '+TSPE', '+TSPL', '-TSPE', '-TSPL', 'TSPE', 'TSPL', 'TSGR', 'TSGS',
       '+FZRA', '-FZRA', 'FZRA', '+FZDZ', '-FZDZ', 'FZDZ', 'FZFG',# freezing stuff
       '+FC', 'FC', 'SQ', 'VA',                                   # nasty stuff that kills/maims
       '+SS', 'SS', '+DS', 'DS', 'PO',                            # nuisance stuff obstructs vision
       'SA', 'DU', 'FU', 'HZ', 'BR', 'FG',                        # same as previous
       'TS', 'GS', 'GR',                                          # pretty cool stuff for chasers
       'IC', '+PE', '+PL', '-PE', '-PL', 'PE', 'PL',              # wintertime goodies
       '+SG', '-SG', 'SG', '+SN', '-SN', 'SN',                    # more winter fare
       '+RA', '-RA', 'RA', '+DZ', '-DZ', 'DZ',                    # drab precip types
       'UP'                                                       # as dreaded as Darth Vader
      );


#...------------...Begin parsing file breaking on cntrl C...----

$/ = "\cC" ;

START:

    #...------------...Open standard input for reading data...-----------------
    print "Opening standard input ... \n" if $verbose;
    open (STDIN, '-');
    binmode (STDIN);

    #Header for variables with units in parenthesis
    print STDOUT "headerDescriptor, icaoId, observationTime, timeReportReceived, surfaceTemperature(C), dewpointTemperature(C), windDirection(deg), windSpeed(kts), windGust(kts), horizontalVisibility(mi), altimeter(in), seaLevelPressure(hPa), qcField, presentWeather, cloudCoverage1, cloudCoverage2, cloudCoverage3, cloudCoverage4, cloudCoverage5, cloudCoverage6, cloudBase1(ft AGL), cloudBase2(ft AGL), cloudBase3(ft AGL), cloudBase4(ft AGL), cloudBase5(ft AGL), cloudBase6(ft AGL), pressureTendency(1/10 hPa), maximumT(C), minmumT(C), maximumTemperature24hr(C), minimumTemperature24hr(C), precipitation(in), precipitation3hr(in), precipitation6hr(in), precipitation24hr(in), snow(in), verticalVisibility(ft), ceilingLow(ft AGL), ceilingHigh(ft AGL), metarType, rawText, possibleNil\n ";
while ( 1 ) {

    #...------------...Set current time info...-----------------

    if ($YYYYMMDDHH) {
        $cur_time = &timegm(59,59,$hour,$day,$month-1,$year);
        die $usage if ($cur_time < 0);
        $max_age = 24*3600 + 15*60;
    } else {
        $cur_time = time();
        $max_age = 11*3600 + 15*60;
    }
    $oldest_allowed = $cur_time - $max_age;
    ($sec,$min,$hour,$day,$month,$year) = (gmtime($cur_time))[0..5];
    $year += 1900;
    $month += 1;
    $timeString = sprintf("%04d-%02d-%02d %02d:%02d:%02d", $year,$month,$day,$hour,$min,$sec);

    $tday = 0;
    $thour = -1;
    $tmin = -1;

    #...------------...Keep STDIN attached unless timeout...----

    vec($rin,fileno(STDIN),1) = 1;
    $nfound = select ($rin, undef, undef, $timeout );
   
    &atexit("timeout") if ( ! $nfound );
    &atexit("eof") if ( eof(STDIN));

    $_ = <STDIN>;


    #...------------...Process header info first...-------------
    #...------------...headers can precede one to many obs...---

    print "Begin processing ... \n" if $verbose;
    s/\cM{1,}//g;             # Swap newlines for control-Ms.
    tr/\n -~//cd;             # Strip out any control-chars.
    #Latest update from Greg on 10/25/2012
    #replaces: tr/\'\"\*\\//d;           # Strip out few nasty chars which do not belong in METARs.
    tr/\`\'\"\*\\\[\]\@\^\~\%\(\)\{\}\?\|\<\>//d;         # Strip out nasty chars that do not belong in METARs.
    s/\n[0-9]{3} ?\n//;       # No need for 3 digit number scheme.
    s/DUPE\n//;               # Why is this here.
                              # Dates in headers look like: "SAUS80 KWBC 250400".
    s/S[APX]\w{2}(\d{1,2})? \w{4} ([0-3][0-9])([0-2][0-9])([0-5][0-9]).*\n//;

    $tday = $2*1 if ( defined($2) && ($2 <= 31) );
    $thour = $3*1 if ( defined($3) && ($3 <= 24) );
    $thour = 23 if ($thour == 24);
    $tmin = $4*1 if ( defined($4) && ($4 <= 59) );
    $tmin = 0 unless ($tmin >= 0);
                              # Skip chunks of header that look like MTRxxx and MTTxxx.
    s/MT[RT][A-Z0-9]{3} ?\n//;

    $hrep_type = ( s/(METAR|SPECI)(\s*\d{4,6}Z?)?\n// )? "$1" : "METAR";

    #...------------...Separate bulletins into reports...-------

    s/\n[ ]{4,}(.*)/ $1/g;             # re-attach continued lines indicated by 4+ spaces.
    s/\s{1,}/ /g;                      # collapse multiple whitespace chars (incl newlines) to a single space.
    s/ ?= ?\n?/\n/g;                   # replace equal signs with newlines.

    @reports = split ( /\n/ );         # now split em on newlines.

    #...------------...Process each individual ob now...--------

REPORT:
    for ( @reports ) {

        #...------------...Set defaults...----------------------
    
        %metarVars =  (
                      "icaoId"   => "",
                      "obsTime"  => $cur_time,
                      "reportTime"  => $timeString,
                      "temp"     => undef,
                      "dewp"     => undef,
                      "wdir"     => undef,
                      "wspd"     => undef,
                      "wgst"     => undef,
                      "visib"    => undef,
                      "altim"    => undef,
                      "slp"      => undef,
                      "qcField"  => 0,
                      "wxString" => "",
                      "cldCvg1"  => "",
                      "cldCvg2"  => "",
                      "cldCvg3"  => "",
                      "cldCvg4"  => "",
                      "cldCvg5"  => "",
                      "cldCvg6"  => "",
                      "cldBas1"  => undef,
                      "cldBas2"  => undef,
                      "cldBas3"  => undef,
                      "cldBas4"  => undef,
                      "cldBas5"  => undef,
                      "cldBas6"  => undef,
                      "presTend" => undef,
                      "maxT"     => undef,
                      "minT"     => undef,
                      "maxT24"   => undef,
                      "minT24"   => undef,
                      "precip"   => undef,
                      "pcp3hr"   => undef,
                      "pcp6hr"   => undef,
                      "pcp24hr"  => undef,
                      "snow"     => undef,
                      "vertVis"  => undef,
                      "ceilHi"  => "",
                      "ceilLow"  => "",
                      "metarType" => $hrep_type,
                      "rawOb"    => "",
                      "remarks"  => undef,
                      "possibleNil" => undef
                     );
    
        next if ( /^\s*\n/ ) ;   # nevermind if blank line.
        s/^ //;                  # get rid of a leading space.
        # Deal with error of report having both METAR & SPECI. Drop METAR designator
        s/METAR ([A-HK-PR-WYZ][A-Z0-9]{3}) SPECI (\d{6}Z .+)/SPECI $1 $2/;
        # Deal with error of report having COR before the Site ID and time. Move it.
        s/METAR COR ([A-HK-PR-WYZ][A-Z0-9]{3}) (\d{6}Z) (.+)/METAR $1 $2 COR $3/;
        # Get the report type from the individual report, in case it wasn't in the header.
        if ( s#^ ?(METAR|SPECI|TESTM|TESTS)\s+## ) {
          $rep_type = $1; 
          $metarVars{"metarType"} = $rep_type;
        }
   
        #...------------...Ensure we didnt goof and append improperly...
  
        if( /.( ?(METAR|SPECI|TESTM|TESTS)?\s+?[A-HK-PR-WYZ][A-Z0-9]{3} \d{6}Z .+)/ ) {
            if (! ($1 =~ /NEXT \d{6}Z/) ) {
                $tmp = $1;
                s/(.)($tmp)/$1/;
                print STDERR "Appended improperly?\t:$_:$tmp:\n";
            }
        }

        #...------------...Finally, we reach the good stuff...------
        #...------------...save chunks into special vars...---------
        #BUGFIX: https://wiki.ucar.edu/display/ralifi/General+Task+List
        #METAR decoding failing on bad METARs
        #Work-around from using Tiny::Try, which cannot be installed
        #by AWC.
        eval{ 
            $metarVars{"rawOb"} = "$_";
            $report = $_;
            $remarks = '';
            if ( s#( RMK| REMARKS)(.*)?## ) {
                $report = $_;
                $remarks =  $2 . $remarks unless( ! $2 || $2 =~ /NIL/ );
            } 
            
            if ( s#( TEMPO| FM[0-2][0-9][0-5][0-9]| BECMG)(.*)?## ) {
                $report = $_;
                $remarks =  $2 . $remarks unless( ! $2 || $2 =~ /NIL/ );
            } 
        
            if ( s#( INTER)(.*)?## ) { 
                $report = $_;
                $remarks =  $2 . $remarks unless( ! $2 || $2 =~ /NIL/ );
            }


            #...------------...Perhaps RMK indication missing...--------
            if ($remarks eq '') {
                if ( m/ Q([01]\d{3}(\.[0-9])?)\s?(.*)?/ || m/ A([23]\d{3})\s?(.*)?/ ) {
                    $report = $_;
                    $remarks =  $2 . $remarks unless( ! $2 || $2 =~ /NIL/ );
                }
            }

            if ($remarks eq '') {
                undef ($remarks);
            }else {
                # Replace '+' with '\+' to prevent premature
                # exit of script (added by Greg Thompson on 7/27/2013) 
                $remarks =~ s/\+/\\+/g;
                $report =~ s#$remarks##;
            }

            #...------------...Skip what dont look like METARs...-------
            #...------------...including old SAO format data...---------

            if (! $report || (length($report) < 6) || ($report =~ s/^[A-Z0-9]{3,4} SA//) ) {
                undef ($report);
                undef ($remarks);
                undef ($metarVars);
                next;
            }

            #...------------...Original body and remarks...
            print LOG ("\n", $metarVars{"rawOb"}, " (", $metarVars{"metarType"}, ")\n") if $verbose;
    
            #...------------...Decode main body contents first...-------
    
            $return_body = &decodeMETARbody($report);
    
            $keep = 1;
            $possibleNil = 0;
            if ($return_body eq "undefined" ) {
                undef ($report);
                undef ($remarks);
                undef ($metarVars);
                next;
            } elsif ($return_body == 2 ) {
                #set the possibleNil flag -this NIL METAR needs to be 
                #checked prior to inserting into the database
                $possibleNil = 1;
            } elsif ($return_body != 1 && $return_body != 2) {
                $remarks = " $return_body" . $remarks;
            }
           
            $metarVars{"possibleNil"} = $possibleNil;
            if ($keep == 1) {
                #...------------...Decode tougher remarks now...------------

                if (length($remarks) > 2 ) {
                    $metarVars{"remarks"} = $remarks;
                    %metarVars = MetarUtils::decodeMETARremarks(%metarVars );
                    &printvars();
                }

                #...------------...Do some friendly printing...-------------

                #&printvars() if $verbose;
    
            }

            #...------------...Wipe away variable contents for next ob...

            undef ($report);
            undef ($remarks);
            undef ($metarVars);
            if (length($tmp) > 6) {
                $leftOver .= "$tmp\n";
                print STDERR "Attempt re-decode\t:$tmp\n";
            }
            undef ($tmp);
            &writeMetar2Csv();

          
            #END of exception handling using EVAL 
            };

            print STDOUT "\n";
            # END BUG FIX METAR decoding failing on bad METARs
            if($@) {
                print "EVAL Exception handling. Error encountered during parsing ... \n" if $verbose;
                goto REPORT;
            };
    
        }            # end of FOR (@reports) loop.

        undef ($tmin);
        undef ($thour);
        undef ($tday);
        undef ($hrep_type);
        undef ($rep_type);
        undef (@reports);
        undef ($possibleNil);

        #...------------...Safety net for leftOver...---------------

        if ($leftOver) {
            $leftOver =~ s/\n$//;
            @reports = split(/\n/, $leftOver);
            undef ($leftOver);
            goto REPORT;
        }

    
            #print newline to delineate the end of one "batch" of METAR data from another
    
}                # end of infinite WHILE (1) loop.
   

exit (0);        # how on earth did we reach this line.

#........................................................................#
#........................................................................#

sub decodeMETARbody {

    ( $_ ) = @_;
    local ($stn_name,
           $ob_year, $ob_month, $ob_day, $ob_hour, $ob_min, $omin, $ohour, $oday, $omonth, $oyear, 
           $obsTime, $extraTime, $try_time1, $try_time2,
           $COR, $AUTO, $NOSIG, $RVRNO, $CAVOK, $TEMPO,
           $vis, $visVert, $visRVR, $wspd, $wdir, $wvrb, $wgst, $wunits, $wdir1, $wdir2,
           $altim, $temp, $dewp, $wxString, $wxCaught, $wxPossible,
           $clouds_cb, @cloud_types, @cloud_hts,
           $i
          );

    $ob_day = 0;
    $ob_hour = -1;
    $ob_min  = -1;

    $_ .= " ";      #...Add a trailing space for simpler matching throughout.

    #...------------...Without icaoId cannot process...
    #...------------...Station must begin with any LETTER except I, J, Q, or X

    $stn_name = $1 if ( s/^([A-HK-PR-WYZ][A-Z0-9]{3}) // );
    return ("undefined") unless ($stn_name);

    #...------------...Try to decipher time and assign obsTime...--------

    if ( s/([0-3][0-9])?([0-2][0-9])([0-5][0-9])Z? // ) {
        $ob_day  = $1*1 if ( defined($1) && ($1 <= 31) );
        $ob_hour = $2*1 if ( defined($2) && ($2 < 24) );
        $ob_min  = $3*1 if ( defined($3) && ($3 <= 59) );
    }

    #...------------...If regular pattern not useful, use time from header

    $ob_day = $tday if ( ($ob_day == 0 || $ob_day > 31) && ($tday > 0) );
    $ob_day = $day if ($ob_day == 0);
    $ob_hour = $thour if ( ($ob_hour == -1 || $ob_hour > 23) && ($thour >= 0) );
    $ob_min = $tmin if ( ($ob_min == -1 || $ob_min > 59) && ($tmin >= 0) );

    if ( $ob_day<1 || $ob_day>31 || $ob_hour<0 || $ob_hour>23 || $ob_min<0 || $ob_min>59) {
        print STDERR ("SKIPPING - no discernable time. Perhaps not a METAR:\n",
               $metarVars{"rawOb"}, "\n");
        return ("undefined");
    }

    #...------------...Attempt to assign a useful obsTime...-------------

    $try_time1 = &my_timegm(0,$ob_min,$ob_hour,$ob_day,$month-1,$year);

    if ( ($try_time1 < $cur_time+3600) && ($try_time1 > $oldest_allowed) ) {
        $obsTime = $try_time1;

    } elsif ( $try_time1 > $cur_time+3600 || $try_time1 < 0 ) {
        $try_time2 = $try_time1 - 3600*24;
        if ( ($try_time2 < $cur_time+3600) && ($try_time2 > $oldest_allowed) ) {
            print STDERR ("FIXING (apparent +1 day typo): ", $metarVars{"rawOb"}, "\n");
            $obsTime = $try_time2;
        } else {
            if ($month > 1) {
                $try_time2 = &my_timegm(0,$ob_min,$ob_hour,$ob_day,$month-2,$year);
            } else {
                $try_time2 = &my_timegm(0,$ob_min,$ob_hour,$ob_day,12-1,$year-1);
            }
            if ( ($try_time2 < $cur_time+3600) && ($try_time2 > $oldest_allowed) ) {
                $obsTime = $try_time2;
            } elsif ($try_time2 < $oldest_allowed) {
                if ( ($try_time1-$cur_time) < (3600*24) && $try_time1 > 0 ) {
                    print STDERR ("FIXING (apparently post-dated, assigning current): ", $metarVars{"rawOb"}, "\n");
                    $obsTime = $cur_time;
                } else {
                    print STDERR ("SKIPPING (apparently too old) NOW=$timeString: ", $metarVars{"rawOb"}, "\n");
                    return ("undefined");
                }
            } else {
                print STDERR ("FIXING (apparently post-dated, assigning current): ", $metarVars{"rawOb"}, "\n");
                $obsTime = $cur_time;
            }
        }

    } elsif ( $try_time1 < $oldest_allowed ) {
        $try_time2 = $try_time1 + 3600*24;
        if ( ($try_time2 < $cur_time+3600) && ($try_time2 > $oldest_allowed) ) {
            print STDERR ("FIXING (apparent -1 day typo): ", $metarVars{"rawOb"}, "\n");
            $obsTime = $try_time2;
        } else {
            ($month,$year) = (gmtime($try_time2))[4..5];
            $month += 1;
            $year  += 1900;
            $try_time2 = &my_timegm(0,$ob_min,$ob_hour,$ob_day,$month-1,$year);
            if ( ($try_time2 < $cur_time+3600) && ($try_time2 > $oldest_allowed) ) {
                $obsTime = $try_time2;
            } else {
                print STDERR ("SKIPPING (apparently too old) NOW=$timeString: ", $metarVars{"rawOb"}, "\n");
                return ("undefined");
            }
        }

    }

    $metarVars{"obsTime"} = $obsTime;
    ($ob_min,$ob_hour,$ob_day,$ob_month,$ob_year) = (gmtime($obsTime))[1..5];
    $ob_year += 1900;
    $ob_month += 1;

    if ($ob_min >= 45 && $metarVars{"metarType"} ne "SPECI") {
        $extraTime = (60 - $ob_min) * 60;      #...Round up to next hour if close to top of hour.
        ($omin,$ohour,$oday,$omonth,$oyear) = (gmtime($obsTime+$extraTime))[1..5];
        $oyear += 1900;
        $omonth += 1;
        $metarVars{"reportTime"} = sprintf("%04d-%02d-%02d %02d:%02d:%02d", $oyear,$omonth,$oday,$ohour,$omin,0);
    } else {
        $metarVars{"reportTime"} = sprintf("%04d-%02d-%02d %02d:%02d:%02d", $ob_year,$ob_month,$ob_day,$ob_hour,$ob_min,0);
    }

    #...------------...We could also check against a known station list
    #...------------...to print list of unknown sites.  Not doing this yet.
    return ("undefined") if ($stn_name eq "CMAN");
    $metarVars{"icaoId"} = $stn_name;
  

    #...------------...Check if just a NIL, return early.
    if ( s#^ ?NIL ?## ) {
        return "undefined";
    }

    #...------------...Is this site automated or without SIGWX...
    $COR = 0;
    $COR   = 1 if ( s/COR // );
    $metarVars{"qcField"} += 1 if $COR;
    $AUTO  = 1 if ( s/AUTO // );
    $metarVars{"qcField"} += 2 if $AUTO;
    $NOSIG = 1 if ( s/NO?SI?G // );
    $metarVars{"qcField"} += 16 if $NOSIG;
    $RVRNO = 1 if( s/RVRNO // );
    $TEMPO = 1 if( s/TEMPO // );
    return ("undefined") unless ($_);

    #...------------...Decode wind info...

    if ( s#(E|W|N|S)?([0-3][0-9]{2}|VRB)(\d{2,3})(G)?(\d{2,3})?(KMH|KPH|KTS?|MPS|MPH)? ## ) {
        if ($2 eq "VRB") {
            $wvrb = 1;
            $wdir = 0;
        } else {
            $wdir = $2*1;
        }
        $wspd = $3*1;
        $wgst = $5*1 if ($4 eq "G");
        $wunits = $6;
        if ($wunits eq "MPS") {
            $wspd *= $MPS2KT;
            $wgst *= $MPS2KT if ($wgst);
        } elsif ($wunits eq "MPH") {
            $wspd *= $MPH2KT;
            $wgst *= $MPH2KT if ($wgst);
        } elsif ($wunits eq "KPH" || $wunits eq "KMH") {
            $wspd *= $KPH2KT;
            $wgst *= $KPH2KT if ($wgst);
        }
        $metarVars{"wdir"} = $wdir;
        $metarVars{"wspd"} = $wspd;
        $metarVars{"wgst"} = $wgst;
    }

    #...------------...Get altimeter...

    if ( s/Q([01]\d{3}(\.[0-9])?) // ) {
        $altim = $1*1.0;
        $metarVars{"altim"} = $altim;
    }

    if ( s/A([23]\d{3}) // ) {
        $altim = $1*0.01;
        $altim *= $INCHES2MB;
        $metarVars{"altim"} = $altim if (! defined($metarVars{"altim"}));
    }

    #...------------...Decode vis info...

    if ( s/C ?A ?V ?O ?K // ) {
        $CAVOK = 1;
        $vis = $UNRESTRICTED_VIS_M;
        $vis *= $M2SM;
        $metarVars{"cldCvg1"} = "CAVOK";

    } elsif ( s/100SM // ) {
        $vis = 99;

    } elsif ( s/100KM // ) {
        $vis = 100*$KM2SM;

    } elsif ( s/10SM // ) {
        $vis = $UNRESTRICTED_VIS;

    } elsif ( s/9999 // ) {
        $vis = $UNRESTRICTED_VIS_M;
        $vis *= $M2SM;

    } elsif ( s/[M\<](\d)\/(\d{1,2})([SK]M) // ) {
        $vis = $1 / $2 unless ($2 == 0);
        $vis *= $KM2SM if ($3 eq "KM");

    } elsif ( s/(\d )?(\d)\/(\d{1,2})([SK]M) // ) {
        if ($1) {
            $vis = $1 + $2 / $3 unless ($3 == 0);
        } elsif ($2) {
            $vis = $2 / $3 unless ($3 == 0);
        } else {
            $vis = $3*1;
        }
        $vis *= $KM2SM if ($4 eq "KM");

    } elsif ( s/P?(\d{1,2})([SK]M) // ) {
        $vis = $1*1;
        $vis *= $KM2SM if ($2 eq "KM");

    } elsif( s/(\d{4})((NE)|(NW)|(SE)|(SW)|(N)|(S)|(E)|(W)|(M))? // ) {
        $vis = $1*1;
        $vis *= $M2SM;
        # $VIS_dir = $2 ;

    }
    $metarVars{"visib"} = $vis if ($vis >= 0);

    #...------------...Decode vertical vis info...

    if( s/VV(\d{3}) // ) {
        $visVert = $1*100.0;
        push (@cloud_types, "OVX");
        push (@cloud_hts, 0);
        $metarVars{"vertVis"} = $visVert;
    } elsif( s/VV\/\/\/ // ) {
        push (@cloud_types, "OVX");
        push (@cloud_hts, 0);
    }

    #...------------...Runway Visual Range (RVR)...
    #...------------...Example: R14R/2800V5500FT

    if ( s/RVR RWY(\d{2})(R|L)? (M|P)?(\d{4})(FT|M) // ) {
        $visRVR = $4*1;
        $visRVR *= $M2SM if ($5 eq "M");
    } elsif ( s/R(\d{2})[RLC]?\/[MP]?(\d{4})V?[MP]?(\d{4})?(FT|N) // ) {
        $visRVR = $2*$FT2SM;
    }

    #...------------...Variable wind direction (must come after RVR)...

    if ( s/(\d{3})V(\d{3}) // ) {
        $wdir1 = $1;
        $wdir2 = $2;
    }

    #...------------...Get sky conditions...
    #...------------...6 possible but really?...4 will suffice...


    #Update from the AWC to map NSC and NCD to CLR on 10/01/2013
    if ( s/(CLR|SKC|NSC|NCD) // ) {
	   if( $1 eq "NSC" || $1 eq "NCD" ){
	      push (@cloud_types, "CLR");
	   } else {
	      push (@cloud_types, $1);
	   }
        push (@cloud_hts, "NULL");
    }
    
    for( $i=0; $i<6; $i++ ) {
        if( s/(OVC|SCT|FEW|BKN) ?(\d{3})(CB|T?CU|\/\/\/)? // ){
            $clouds_cb = $3 if $3;
            push (@cloud_types, $1);
            push (@cloud_hts, $2*100);
        } else {
            last;
        }
    }
    if ($cloud_types[0] eq "CLR" && defined($cloud_types[1]) ) {
        shift @cloud_types;
        shift @cloud_hts;
    }

    $metarVars{"cldCvg1"} = $cloud_types[0] if $cloud_types[0];
    $metarVars{"cldCvg2"} = $cloud_types[1] if $cloud_types[1];
    $metarVars{"cldCvg3"} = $cloud_types[2] if $cloud_types[2];
    $metarVars{"cldCvg4"} = $cloud_types[3] if $cloud_types[3];
    $metarVars{"cldCvg5"} = $cloud_types[4] if $cloud_types[4];
    $metarVars{"cldCvg6"} = $cloud_types[5] if $cloud_types[5];

    $metarVars{"cldBas1"} = $cloud_hts[0] if (defined($cloud_hts[0]));
    $metarVars{"cldBas2"} = $cloud_hts[1] if $cloud_hts[1];
    $metarVars{"cldBas3"} = $cloud_hts[2] if $cloud_hts[2];
    $metarVars{"cldBas4"} = $cloud_hts[3] if $cloud_hts[3];
    $metarVars{"cldBas5"} = $cloud_hts[4] if $cloud_hts[4];
    $metarVars{"cldBas6"} = $cloud_hts[5] if $cloud_hts[5];

    #...------------...Get temperature and dewpoint...

    if ( s/(M)?(\d{2})\/(M)?(\d{2})? // ) {
        $temp = $2*1.0;
        $temp *= -1 if( $1 && ($temp != 0) );
        $dewp = $4*1.0 if( $4 );
        $dewp *= -1 if( $3 && ($dewp != 0) );
        $metarVars{"temp"} = $temp;
        $metarVars{"dewp"} = $dewp;
    } elsif ( s/(M)?(\d{2})\/(M)?(\w{1,2})? // ) {
        $temp = $2*1.0;
        $temp *= -1 if( $1 && ($temp != 0) );
        $metarVars{"temp"} = $temp;
    }

    #...------------...Weather conditions (WMO code table 4678)...

    s/NOSPECI//;    # Eliminate false PE when this is not specifically put into RMK section
    s/ALL?QDS//;    # Eliminate false DS because some sites use TCU ALQDS
    s/TDS//;        # Eliminate false DS because Mexican sites use TDS to indicate "todos" = ALQDS
    s/FCST//;       # Eliminate false FC from comments about FCST
    s/GRID//;       # Eliminate false GR because of GRID wind direction for south pole (NZSP)
    s/(GRN|RED|WHT|BLU|YLO|)\+?//g; # Military (RAF or USAF) codes for airfield 'Fitness' 
                    # ('BLU' - best, 'WHT'/'GRN'/'YLO' - 1 or 2 oktas, 'AMB'/'RED' - 3 oktas or more, 'BLACK' - non-weather closure) 
    s/RE[A-Z]{2,}//;# Eliminate false RA/SN/... because some sites use RE.. to indicate recent wx?
    s/NEFO PLAYA//; # Eliminate false PL because some sites use NEFO PLAYA to indicate ???
    s/EP[CO]//;     # Eliminate false PO because PAER PAFK use EP[CO] to indicate estimate pass closed/open

    
    #Update from AWC (10/01/2013), to correctly handle -RAPL as 'light rain, light ice pellets' rather than
    #'ice pellets, light rain'.
    for ( $i=0; $i<6; $i++ ) {
        if ( s/^ ?(\+|-|VC)?(TS|SH)?((PR|MI|BC|DR|BL|FZ){0,2})((DZ|RA|SN|SG|IC|PE|PL|GR|GS){0,4}) // ) {
            # Catch precipitation elements
            $qualifier = "";
            $qualifier .= $1 if ( defined($1) );
            $qualifier .= $2 if ( defined($2) );
            $qualifier .= $3 if ( defined($3) );
            if ( $3 ne "" ) {
                # Apply exception in NSWI 10-831 1.2.6.1; AC 00-45G 7.2.2.7.1
                my @precipArr = ( $5 =~ m/../g );
                # First weather gets qualifier
                $wxString .= $qualifier;
                $wxString .= shift(@precipArr);
                $wxString .= " "; 
                # Remaining elements get no qualifier
                while ( defined( $precip = shift @precipArr)){
                     $wxString .= $precip;
                     $wxString .= " "; 
                }       
            } else {
                if ( $5 ne "" ) {
                    my @precipArr = ( $5 =~ m/../g );
                    # Apply qualifier to all precip elements
                    while ( defined( $precip = shift @precipArr)){
                        $wxString .= $qualifier;
                        $wxString .= $precip;
                        $wxString .= " "; 
                    }       
                } else {
                    $wxString .= $qualifier;
                    $wxString .= " "; 
                }       
            }       
        } elsif ( s/^ ?(\+|-|VC)?((PR|MI|BC|DR|BL|FZ){0,2})(BR|FG|FU|VA|DU|SA|HZ|PY)?(PO|SQ|SS|DS|FC)? // ) {
            # Catch obscuration/other elements
            $wxString .= $1 if ( defined($1) );
            $wxString .= $2 if ( defined($2) );
            $wxString .= $4 if ( defined($4) );
            $wxString .= $5 if ( defined($5) );
            $wxString .= " "; 
        } else {
            last;   
        }       
        $wxString =~ s/\s{1,}/ /g;              # collapse multiple spaces to a single space.
    }









    #...------------...For some reason, line above does not always catch
    #...------------...all chunks of wx.  Therefore, do again looking
    #...------------...for accepted strings.  This may change the order
    #...------------...of different wx types from original input.

    for ( $i=0; $i<3; $i++ ) {
        $wxCaught = 0;
        foreach $wxPossible (@WX) {
            $wxPossible =~ s%^([\+-])%\\$1%;
            if ( s/ ?($wxPossible) ?// ) {
                $wxString .= "$1 ";
                $wxCaught = 1;
                last;
            }
        }
        last unless $wxCaught;
        $wxString =~ s/\s{1,}/ /g;              # collapse multiple spaces to a single space.
    }

    $wxString =~ s/ $//;
    $wxString =~ s/^ //;
    $metarVars{"wxString"} = $wxString if $wxString;

    print LOG "\tLeft over body (between colons):$_:\n" if ($verbose && $_ && $_ ne " ");

    #...------------...Return zero if we did not fill something useful...
    #...------------...Then again, perhaps METAR has useful RMK section...

#   if ( ! defined($metarVars{"temp"}) && ! defined($metarVars{"dewp"}) &&
#           ! defined($metarVars{"wdir"}) && ! defined($metarVars{"wspd"}) &&
#           ! defined($metarVars{"altim"})) {
#       return ("undefined");
#   }

    if ($_ && $_ ne " ") {
        return ("$_")

    } else {
        return (1);
    }

}

#........................................................................#

sub printvars {

    local ($osec, $omin, $ohour, $oday, $omonth, $oyear, $date_string);
    ($osec,$omin,$ohour,$oday,$omonth,$oyear) = (gmtime($metarVars{"obsTime"}))[0..5];
    $oyear += 1900;
    $date_string = sprintf("%04d-%02d-%02d %02d:%02d:%02d", $oyear,$omonth+1,$oday,$ohour,$omin,$osec);

    #...------------...Now chunks of good info...
    printf LOG ("%12s \t %s \t %s \t %s   (%s %d = %s)\n", "Station", $metarVars{"icaoId"}, "at",
            $metarVars{"reportTime"}, "actual ob time:", $metarVars{"obsTime"}, $date_string);
    printf LOG ("%12s \t %s\n", "Temperature", $metarVars{"temp"});
    printf LOG ("%12s \t %s\n", "Dewpoint", $metarVars{"dewp"});
    printf LOG ("%12s \t ", "Winds");
    if (defined($metarVars{"wspd"}) && $metarVars{"wspd"} == 0) {
        printf LOG ("Calm");
    } elsif (defined($metarVars{"wspd"})) {
        printf LOG ("from %d at %d knots", $metarVars{"wdir"}, $metarVars{"wspd"});
        printf LOG (" with gusts to %d", $metarVars{"wgst"}) if ($metarVars{"wgst"} > 0);
    }
    printf LOG ("\n");
    printf LOG ("%12s \t %s\n", "Visibility", $metarVars{"visib"});
    printf LOG ("%12s \t %s\n", "Altimeter", $metarVars{"altim"});
    printf LOG ("%12s \t ", "Clouds");
    if ($metarVars{"cldCvg1"} eq "NSC") {
        printf LOG ("none below 5,000 feet");
    } elsif ($metarVars{"cldCvg1"} eq "NCD") {
        printf LOG ("none below 5,000 feet");
    } elsif ($metarVars{"cldCvg1"} eq "SKC") {
        printf LOG ("none");
    } elsif ($metarVars{"cldCvg1"} eq "CLR") {
        printf LOG ("none below 12,000 feet");
    } elsif ($metarVars{"cldCvg1"} eq "CAVOK") {
        printf LOG ("none below 5,000 feet");
    } elsif ($metarVars{"cldCvg1"}) {
        printf LOG ("%s at %d", $metarVars{"cldCvg1"}, $metarVars{"cldBas1"});
    }
    printf LOG ("\t %s at %d", $metarVars{"cldCvg2"}, $metarVars{"cldBas2"}) if ($metarVars{"cldCvg2"});
    printf LOG ("\t %s at %d", $metarVars{"cldCvg3"}, $metarVars{"cldBas3"}) if ($metarVars{"cldCvg3"});
    printf LOG ("\t %s at %d", $metarVars{"cldCvg4"}, $metarVars{"cldBas4"}) if ($metarVars{"cldCvg4"});
    printf LOG ("\t %s at %d", $metarVars{"cldCvg5"}, $metarVars{"cldBas5"}) if ($metarVars{"cldCvg5"});
    printf LOG ("\t %s at %d", $metarVars{"cldCvg6"}, $metarVars{"cldBas6"}) if ($metarVars{"cldCvg6"});
    printf LOG ("\n");
    printf LOG ("%12s \t %s\n", "Weather", $metarVars{"wxString"}) if ($metarVars{"wxString"});
    printf LOG ("%12s \t %s\n", "MSLP", $metarVars{"slp"}) if (defined($metarVars{"slp"}));
    printf LOG ("%12s \t %s\n", "Pres tend", $metarVars{"presTend"}) if (defined($metarVars{"presTend"}));

    if ($metarVars{"precip"} == 0 && defined($metarVars{"precip"})) {
        printf LOG ("%12s \t %s\n", "Precip since", "trace");
    } elsif ($metarVars{"precip"} > 0) {
        printf LOG ("%12s \t %s\n", "Precip since", $metarVars{"precip"});
    }
    if ($metarVars{"pcp3hr"} == 0 && defined($metarVars{"pcp3hr"})) {
        printf LOG ("%12s \t %s\n", "Precip 3-hr", "trace");
    } elsif ($metarVars{"pcp3hr"} > 0) {
        printf LOG ("%12s \t %s\n", "Precip 3-hr", $metarVars{"pcp3hr"});
    }
    if ($metarVars{"pcp6hr"} == 0 && defined($metarVars{"pcp6hr"})) {
        printf LOG ("%12s \t %s\n", "Precip 6-hr", "trace");
    } elsif ($metarVars{"pcp6hr"} > 0) {
        printf LOG ("%12s \t %s\n", "Precip 6-hr", $metarVars{"pcp6hr"});
    }
    if ($metarVars{"pcp24hr"} == 0 && defined($metarVars{"pcp24hr"})) {
        printf LOG ("%12s \t %s\n", "Precip 24-hr", "trace");
    } elsif ($metarVars{"pcp24hr"} > 0) {
        printf LOG ("%12s \t %s\n", "Precip 24-hr", $metarVars{"pcp24hr"});
    }
    printf LOG ("%12s \t %s\n", "Snow:", $metarVars{"snow"}) if (defined($metarVars{"snow"}));
}

#........................................................................#

sub my_timegm {
    local ($sec, $min, $hour, $day, $month, $year) = @_;
    use Time::Local;
    local ($utime, $d, $m, $y);

    eval { $utime = &timegm($sec,$min,$hour,$day,$month,$year); };

    $utime = -1 unless ($@ eq "");

    ($d, $m, $y) = (gmtime($utime))[3..5];
    $utime = -1 if ($d != $day || $m != $month || $y+1900 != $year);

    return $utime;
}

#........................................................................#

sub atexit {

    local($sig) = @_ ;

    if ( $sig eq "eof" ) {
        print STDERR "\neof on STDIN --shutting down\n\n" ;
    } elsif ( $sig eq "timeout" ) {
        print STDERR ("$prog shutting down, timeout = ", $timeout/60, " minutes\n");
    } elsif ( defined($sig) ) {
        print STDERR "\nCaught SIG$sig --shutting down\n\n" ;
    }

    close (STDOUT);
    close (LOG);
    close (STDERR);

    exit (0);
}

#........................................................................#
sub writeMetar2Csv{
    local ($icaoId, $obsTime, $reportTime, $temp, $dewp, $wdir, $wspd, $wgst, $visib,
           $altim, $slp, $qcField,
           $wxString, $cldCvg1, $cldCvg2, $cldCvg3, $cldCvg4, $cldCvg5, $cldCvg6, $cldBas1, $cldBas2, $cldBas3, $cldBas4,
           $cldBas5, $cldBas6, $presTend, $maxT, $minT, $maxT24, $minT24, $precip, $pcp3hr, $pcp6hr, $pcp24hr,
           $snow, $vertVis, $ceilLow, $ceilHi,
           $metarType, $rawOb, $COR);
    my @stack=();

    # In the future, we may enable confidence values to each decoded field.
    # Use the headerDescriptor to indicate which line corresponds to decoded Metar, and which corresponds
    # to qc/confidence values. For now, the line containing the qc/confidence values will be empty
    $headerDescriptor = 'METAR';

    $icaoId     = $metarVars{"icaoId"};
    $obsTime    = $metarVars{"obsTime"};
    $reportTime = $metarVars{"reportTime"};
    $temp       = $metarVars{"temp"};
    $dewp       = $metarVars{"dewp"};
    $wdir       =  $metarVars{"wdir"};
    $wspd       =  $metarVars{"wspd"};
    $wgst       =  $metarVars{"wgst"};
    $visib      =  $metarVars{"visib"};
    $altim      =  $metarVars{"altim"};
    $slp        =  $metarVars{"slp"};
    $qcField    = $metarVars{"qcField"};
    $wxString   = $metarVars{"wxString"}; 
    $cldCvg1    = $metarVars{"cldCvg1"};
    $cldCvg2    = $metarVars{"cldCvg2"};
    $cldCvg3    = $metarVars{"cldCvg3"};
    $cldCvg4    = $metarVars{"cldCvg4"};
    $cldCvg5    = $metarVars{"cldCvg5"};
    $cldCvg6    = $metarVars{"cldCvg6"};
    $cldBas1    =  $metarVars{"cldBas1"};
    $cldBas2    =  $metarVars{"cldBas2"};
    $cldBas3    =  $metarVars{"cldBas3"};
    $cldBas4    =  $metarVars{"cldBas4"};
    $cldBas5    =  $metarVars{"cldBas5"};
    $cldBas6    =  $metarVars{"cldBas6"};
    $presTend   =  $metarVars{"presTend"};
    $maxT       =  $metarVars{"maxT"};
    $minT       =  $metarVars{"minT"};
    $maxT24     =  $metarVars{"maxT24"};
    $minT24     =  $metarVars{"minT24"};
    $precip     =  $metarVars{"precip"};
    $pcp3hr     =  $metarVars{"pcp3hr"};
    $pcp6hr     =  $metarVars{"pcp6hr"};
    $pcp24hr    =  $metarVars{"pcp24hr"};
    $snow       =  $metarVars{"snow"};
    $ceilLow    =  $metarVars{"ceilLow"};
    $ceilHi     =  $metarVars{"ceilHi"};
    $vertVis    =  $metarVars{"vertVis"};
    $metarType  = $metarVars{"metarType"};
    $rawOb      = $metarVars{"rawOb"};
    $remarks = $metarVars{"remarks"};
    $possibleNil = $metarVars{"possibleNil"};
     
   #Remove the quotation marks surrounding the cloud coverages and
   #wxString (present weather)
   $cldCvg1 =~ s/"//g;
   $cldCvg2 =~ s/"//g;
   $cldCvg3 =~ s/"//g;
   $cldCvg4 =~ s/"//g;
   $cldCvg5 =~ s/"//g;
   $cldCvg6 =~ s/"//g;
   $wxString =~ s/"//g;
 
   my @metarVarArray = ($headerDescriptor, $icaoId, $obsTime, $reportTime, $temp, $dewp, $wdir, $wspd, $wgst, $visib, $altim, $slp, $qcField, $wxString, $cldCvg1, $cldCvg2, $cldCvg3, $cldCvg4, $cldCvg5, $cldCvg6, $cldBas1, $cldBas2, $cldBas3, $cldBas4, $cldBas5, $cldBas6, $presTend, $maxT, $minT, $maxT24, $minT24, $precip, $pcp3hr, $pcp6hr, $pcp24hr, $snow, $vertVis, $ceilLow, $ceilHi, $metarType, $rawOb, $possibleNil); 
   my $metarCsv = join(',',@metarVarArray);
   #Push this onto the FIFO stack to maintain the order in which the reports arrived
   unshift(@stack, $metarCsv); 

   print STDOUT $metarCsv;
   print STDOUT "\n";
   #print LOG "\n" if $verbose;
   
   #QC/confidence
   #For now, we don't calculate the confidence for each decoded field
   #This is where you would print them, so that the CSV file consists
   #of alternating lines of decoded METARs and CONFIDENCE values
   #
   #Always assign 'CONFIDENCE' to the headerDescriptor variable
   #print STDOUT "CONFIDENCE";
   #for ($i = 0; $i < $numberOfVariables; $i++ ){
   #    print STDOUT ",";
   #}
   #print STDOUT "\n";
}


#........................................................................#
__END__
