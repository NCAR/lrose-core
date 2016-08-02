#!/usr/bin/perl -w
use strict;                            # Dont try to be too lazy
use DBI;                               # Connect up with DBI library
use Time::Local;                       # Time conversion libraries
use vars qw($opt_h $opt_b);
use Getopt::Std;
use Env qw(WEBWXHOME);                 # Get top-level project dir
(my $prog = $0) =~ s%.*/%%;            # Determine program basename
$| = 1;                                # Unbuffer std_out

my $dbHost = "debris";                 # MySQL user account
my $dbUser = "www-data";               # MySQL user account
my $dbPasswd = "or-knots";             # MySQL password for account
my $dbName = "weather";                # Name of DB to access
my $metarTable = "Metars";             # Name of METARs table
my $stationTable = "StationInfo";      # Name of station table
my $debug = 1;                         # Debug flag
my $verbose = 1;                       # Verbose debug flag

my $numFound = 0;
my @binaryOutput = ();
my $binaryFlag = 0;

my $usage = <<EOF;
Usage:  $prog [-b] [-h]
   -b:       Outputs a binary file with data values. 
   -h:       Prints this usage info.
EOF
&getopts('hb') || die $usage;
die $usage if $opt_h;

my ($tempFile, $outFile, $finOutFile, $outputDir);
$tempFile = "$$\_temp.txt";
unlink ("$tempFile") if (-e "$tempFile");

if ($opt_b) {
    $binaryFlag = 1;
    $outFile = '"${yyyymmddhh}_metars.bin"';
    $outputDir = "/www/htdocs/data/metars";  # Directory to place output file
} else {
    $outFile = '"${yyyymmddhh}_metars.txt"';
    $outputDir = ".";  # Directory to place output file
}

#..Get the current time (GMT), fill in vars with time elements
#..Set the time history based on number of minutes past current hour

my ($hour, $day, $new_day, $month, $year, $yyyymmdd, $yyyymmddhh);
my ($now, $nominalTime, $minDifference, $pastHist);
$now = $^T;
($hour, $day, $month, $year) = (gmtime($now))[2..5];
$nominalTime = &timegm(0,0,$hour,$day,$month,$year);

$minDifference = int(($now - $nominalTime)/60 + 0.5);
$pastHist = ($minDifference + 16)*60;
#$pastHist = 5*60;          # FOR TESTING PURPOSES, SHORTEN TIME TO THIS!

$year += 1900;
$month+=1;
$yyyymmdd = sprintf("%d%02d%02d", $year, $month, $day);
$yyyymmddhh = sprintf("%d%02d%02d%02d", $year, $month, $day, $hour);

#..Now we have date strings so we can fill in date-dependent variable names
grep($_ = eval, $finOutFile = $outFile);

#..Connect to the database or die
my $dbh = DBI->connect("DBI:mysql:$dbName:$dbHost", $dbUser, $dbPasswd, { RaiseError => 1 } );

print "Using tables: $metarTable\n" if $verbose;

@binaryOutput = &pullAndPrint($metarTable, $dbh);

#..If binary option, write output to binary file (otherwise ascii file written by sub above).
if ($binaryFlag) {

    print STDOUT ("Dumping these data to binary file\n") if $debug;
    open (BIN, ">$tempFile") or die "Cannot open $tempFile for writing\n$!\n\n";
    my $minLat = unpack('V', pack('f', -90.0));        # Fudge byte-swap for big-endian output
    my $maxLat = unpack('V', pack('f', 90.0));
    my $minLon = unpack('V', pack('f', -180.0));
    my $maxLon = unpack('V', pack('f', 180.0));
    my $headerInfo = pack("N N4 N a4", $numFound, $minLat, $maxLat, $minLon, $maxLon, $now, "XALL");
    print BIN ($headerInfo,@binaryOutput);
    close (BIN);
}


#..Though not absolutely necessary, nicely close the handle to the DB
$dbh->disconnect() or warn "disconnecting from database failed: $DBI::errstr\n";

unlink ("$finOutFile") if (-e "$finOutFile");
print "Moving file $tempFile to file:  $outputDir/$finOutFile\n" if $debug;
((system ("/bin/mv $tempFile $outputDir/$finOutFile") >> 8) == 0) || warn "Sys mv error: $!\n\n";

print "done\n";
#-----------------------------------------------------------------------------------
#-----------------------------------------------------------------------------------

sub pullAndPrint {
    my ($metarTable, $dbhandle) = @_;
    use vars qw ($pastHist $stationTable $tempFile $debug $hour $numFound $binaryFlag);
    my ($icao, $icao2, $otime, $lat, $lon, $elev, $vis, $ceil, $slp, $altim, $temp, $dewp, $wdir, $wspd, $wgst);
    my ($wx, $cvg1, $cvg2, $cvg3, $cvg4, $bas1, $bas2, $bas3, $bas4, $raw, $lenf, $clds, $priority);
    my ($sql, $sth, $rv, $rc);
    my (@binaryData);

    my $MB2INCHES = 30.0/1016.0;           # 30.00 inches of mercury equals 1016.0 mb

    if (!($binaryFlag)) {
        #..Open a temporary output file (moved by main at end of prog)
        open (TXT, ">>$tempFile") or die "Cannot open $tempFile for writing\n$!\n\n";
    }

    #..Prepare the SQL query - get METARs from past ($pastHist)
    $sql = qq{
        SELECT $metarTable.icaoId, obsTime, visib, slp, altim, temp, dewp, wdir, wspd, wgst, \
        wxString, cldCvg1, cldCvg2, cldCvg3, cldCvg4, cldBas1, cldBas2, cldBas3, cldBas4, rawOb, \
        $stationTable.icaoId, lat, lon, elev, priority \
        FROM $metarTable STRAIGHT_JOIN $stationTable \
        WHERE obsTime>(UNIX_TIMESTAMP(NOW())-$pastHist) \
        AND $metarTable.icaoId=$stationTable.icaoId \
        ORDER BY priority ASC, obsTime DESC \
    };

    print "Preparing query: $sql\n\n" if $verbose;
    $sth = $dbhandle->prepare($sql);

    $rv = $sth->execute;
    $rc = $sth->bind_columns(undef, \$icao, \$otime, \$vis, \$slp, \$altim, \$temp, \$dewp, \$wdir,
             \$wspd, \$wgst, \$wx, \$cvg1, \$cvg2, \$cvg3, \$cvg4, \$bas1,
             \$bas2, \$bas3, \$bas4, \$raw, \$icao2, \$lat, \$lon, \$elev, \$priority);
 
    #..Process results of query

    while ($sth->fetch) {
        $numFound++;
        $lenf = 0;

        $vis   = (defined($vis && $vis >= 0))?        $vis/100 : -99;
        $slp   = (defined($slp && $slp > 5000))?      $slp/10  : -99.0;
        $altim = (defined($altim && $altim > 5000))?  $altim/10*$MB2INCHES : -99.00;
        $temp  = (defined($temp && $temp > -800))?    $temp/10 : -99.0;
        $dewp  = (defined($dewp && $dewp > -800))?    $dewp/10 : -99.0;
        $wdir  = (defined($wdir && $wdir >= 0))?      $wdir    : -99;
        $wspd  = (defined($wspd && $wspd >= 0))?      $wspd    : -99;
        $wgst  = (defined($wgst && $wgst >= 0))?      $wgst    : -99;
        $wx    = ($wx ne "NULL")?                     $wx      : " "x16;
        $cvg1  = ($cvg1 ne "NULL")?                   $cvg1    : " "x3;
        $cvg2  = ($cvg2 ne "NULL")?                   $cvg2    : " "x3;
        $cvg3  = ($cvg3 ne "NULL")?                   $cvg3    : " "x3;
        $cvg4  = ($cvg4 ne "NULL")?                   $cvg4    : " "x3;
        $bas1  = (defined($bas1 && $bas1 >= 0))?      $bas1    : -99;
        $bas2  = (defined($bas2 && $bas2 >= 0))?      $bas2    : -99;
        $bas3  = (defined($bas3 && $bas3 >= 0))?      $bas3    : -99;
        $bas4  = (defined($bas4 && $bas4 >= 0))?      $bas4    : -99;
        $lat   = (defined($lat && $lat >= -90))?      $lat     : -89.99;
        $lon   = (defined($lon && $lon >= -180))?     $lon     : -179.99;
        $elev  = (defined($elev && $elev >= -900))?   $elev    : -999;

        #..Derive:  total raw length, ceiling, and new cloud string
        $lenf  = length($raw);
        $ceil  = -99;
        $ceil = $bas1 if ($cvg1 eq "OVC" || $cvg1 eq "BKN");
        $ceil = $bas2 if ($ceil < 0 && ($cvg2 eq "OVC" || $cvg2 eq "BKN"));
        $ceil = $bas3 if ($ceil < 0 && ($cvg3 eq "OVC" || $cvg3 eq "BKN"));
        $ceil = $bas4 if ($ceil < 0 && ($cvg4 eq "OVC" || $cvg4 eq "BKN"));
        $ceil = sprintf("%03d", $ceil);
        $clds = undef;
        if ($cvg1 ne "   ") {
            $clds = "$cvg1";
            $clds .= sprintf("%03d", $bas1) if ($bas1 > 0);
            $clds .= " $cvg2" if ($cvg2 ne "   ");
            $clds .= sprintf("%03d", $bas2) if ($bas2 > 0);
            $clds .= " $cvg3" if ($cvg3 ne "   ");
            $clds .= sprintf("%03d", $bas3) if ($bas3 > 0);
        }
        $clds .= ' 'x(24-length($clds)) if (length($clds) < 24);

        #..Handle data packing for binary output, stored in array here, passed to main for file output.
        if ($binaryFlag) {  

            $lat = unpack('V', pack('f', $lat));        # Fudge byte-swap for big-endian output
            $lon = unpack('V', pack('f', $lon));
            $elev = unpack('V', pack('f', $elev));
            $vis = unpack('V', pack('f', $vis));
            $altim = unpack('V', pack('f', $altim));
            $temp = unpack('V', pack('f', $temp));
            $dewp = unpack('V', pack('f', $dewp));
            push (@binaryData, pack("a4 N2 N4 N N3 N4 a2 a16 a24 a2 a*", $icao, $otime, $hour, $lat, $lon, $elev, $vis, $ceil, $altim, $temp, $dewp, $wdir, $wspd, $wgst, $lenf, "M ", $wx, $clds, "  ", $raw));

        } else {

            #..Write output to file, clean up go to next row
            write TXT;

        }
    }

    close(TXT) unless ($binaryFlag);

    print STDOUT ("Found ", $numFound, " METARs in DB within last ",
            int($pastHist/60 + 0.5), " mins\n") if $debug;

    $rc = $sth->finish;


#-----------------------------------------------------------------------------------
format TXT =
@## @<<< @#########  @##.### @###.### @### @##.## @<< @###.# @##.## @##.# @##.# @## @## @## M @<<<<<<<<<<<<<<< @<<<<<<<<<<<<<<<<<<<<<<< @*
$lenf, $icao, $otime, $lat, $lon, $elev, $vis, $ceil, $slp, $altim, $temp, $dewp, $wdir, $wspd, $wgst, $wx, $clds, $raw
.
#-----------------------------------------------------------------------------------

return @binaryData;

}
