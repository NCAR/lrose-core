#!/usr/bin/perl

#.. This script is spawned upon receipt of an GRIB AVAL notification from LDM
#.. for the availability of a new RUC or Eta model GRIB file.  On standard
#.. input should be a single line containing the absolute path filename 
#.. and time (separated with a space).  For RUC notifications, the filename is
#.. actually the name of the ldm server "status file," which must be remapped
#.. to the grib file name. This script will then go copy that file and populate
#.. the MySQL table containing Meta-data related to GRIB files. In many cases, 
#.. the grib file is not yet available when the notification is received, so 
#.. this script must wait up to 10 minutes for it to arrive. In addition, Eta
#.. files come in as seperate layers which are cat'd together by the main LDM
#.. server. For these files, wait until all of the layers are in (>950K) to 
#.. retrieve and process the file.

use strict;
use Time::Local;                          # Time conversion libraries
use Env qw(ADDSHOME);                     # Get ADDS project dir
use lib "$ADDSHOME/perl_lib";             # Library modules found here
use Toolsa;
use GriddedData_DbUtils;

use vars qw($opt_h $opt_d $opt_t);        # Command-line options.
use Getopt::Std;                          # Use the command-line processing lib.
(my $prog = $0) =~ s%.*/%%;               # Determine program basename.
$| = 1;                                   # Unbuffer std_out.
umask 002;                                # Set file permissions.
my $debug = 0;                            # Override with command-line option.

#...------------...Important script variables...-------------

my $doRegister  = 1;                      # Flag for procmap registration
my $regInterval = 3*3600;                 # Expect GRIB files at least every three hours.
my $waitTimeout = 600;                    # Max secs to wait for grib file to be available.
my $timeout = 150;                        # Override with command-line option.
my ($time, $inputFile, $outputFile);
my ($yyyymmddhh, $ff, $modelName);
my ($yyyy, $mm, $dd, $hh);
my ($initTime, $fcstLength, $validTime, $vertLevel);
my $cpCmd = "";
my $statusFileBasePath = "/grib/tmp";
my $gribFileBasePath = "/grib";
my $statusFileExtension = ".status";
my $gribFileExtension = ".grb2";

#...------------...Parse options/command line...-------------

my $usage = <<EOF;
Usage:  $prog [-hd] [-t timeout] output_filename
   -h:       Prints this usage info.
   -d:       Turn on debug info.
   -t:       timeout length(s) to wait for file and for safe_system execution of copy.
EOF
&getopts('hdt:') || die $usage;
die $usage if $opt_h;
$debug = 1 if $opt_d;
$timeout = $opt_t if $opt_t;
die $usage if $#ARGV < 0;
$outputFile = "$ARGV[0]";

(my $fileBase = $outputFile) =~ s%.*/%%;
(my $outDir = $outputFile) =~ s%(.*)/$fileBase%$1%;
(my $baseDir = $outputFile) =~ s%(.*)/.*/$fileBase%$1%;
(mkdir("$baseDir") || die "cannot create base dir $baseDir\n$!\n\n") unless (-d "$baseDir");
(mkdir("$outDir") || die "cannot create dir $outDir\n$!\n\n") unless (-d "$outDir");

#...------------...Register with procmap...------------------

if ($doRegister) {
    my $ldataHandle=Toolsa::LDATA_init_handle($prog, $debug);
    Toolsa::PMU_auto_init($prog, $prog, $regInterval);
    Toolsa::PMU_force_register("Starting $prog");
}

#...------------...Determine the model name...---------------

#($modelName = $outputFile) =~ s%.*/\d{10}_F\d{2}_(\w{3})\w*\.grb2%$1%;

#..to accomodate the 3-digit Forecast hours supported by GFS
($modelName = $outputFile) =~ s%.*/\d{10}_F\d*_(\w{3})\w*\.grb2%$1%;
print STDOUT "outputfile name = $outputFile \n";
print STDOUT "model name = $modelName \n";
if ( $modelName ne "RUC" && $modelName ne "ETA" && $modelName ne "GFS" ) {
    die "ABORT: Invalid model name, ${modelName}, parsed from output filename, ${outputFile}.";
}
 
#...------------...Parse std-in...---------------------------

while (<STDIN>) {
    chop;
    ($inputFile, $time) = split(' ');
}

if (!($inputFile)) {
    die "ABORT: cannot continue without inputFile\n";
}

#...------------...map status file to grib file...-----

$inputFile =~ s%$statusFileBasePath%$gribFileBasePath%;
$inputFile =~ s%$statusFileExtension%$gribFileExtension%;

#...------------...Sleep until file is available...---------

print "Got file: $inputFile for copy...\n" if $debug;

my $beginTime = time();
while ( ! (-e "$inputFile") ) {
    print "Sleeping 10 seconds...\n" if $debug;
    sleep 10;
    if ( time() - $beginTime > $waitTimeout ) {
	die "ABORT: Timed out ($waitTimeout seconds) waiting for grib file: $inputFile\n";
    }
}

#...------------...copy the file...-------------------------

$cpCmd = "cp -f $inputFile $outputFile";
print "executing $cpCmd\n" if $debug;
Toolsa::PMU_force_register("$cpCmd") if $doRegister;
Toolsa::safe_system("$cpCmd", $timeout);

#...------------...populate GriddedDataFiles table...--------
#.. $outputFile MUST include the form: 2003031323_F12_RUCp.grb2
#   and 20071011_F180_GFS004.grb2
if ( $modelName eq "GFS"){
   #one more digit in the Forecast time for GFS
   ($yyyymmddhh, $ff, $modelName) = $outputFile =~ /(2[0-9]{3}[0-1][0-9][0-3][0-9][0-2][0-9][0-9])_F([0-9][0-9])_([A-Za-z0-9]+)\.grb2/;
}
else{
   ($yyyymmddhh, $ff, $modelName) = $outputFile =~ /(2[0-9]{3}[0-1][0-9][0-3][0-9][0-2][0-9])_F([0-9][0-9])_([A-Za-z0-9]+)\.grb2/;
}
print "Found following ingredients in filename: $yyyymmddhh, $modelName, $ff\n" if $debug;
($yyyy, $mm, $dd, $hh) = $yyyymmddhh =~ /(2[0-9]{3})([0-1][0-9])([0-3][0-9])([0-2][0-9])/;
$initTime = &timegm(0,0,$hh,$dd,$mm-1,$yyyy-1900);
$fcstLength = $ff*3600;
$validTime = $initTime + $fcstLength;
$vertLevel = 0;

if ( -e $outputFile ) {
    print "Entering this RUC forecast, $inputFile into GriddedDataFiles table\n" if $debug;
    GriddedData_DbUtils::fillGriddedData_db($outputFile, $modelName, $initTime, $fcstLength, $vertLevel);
} else {
    print "WARNING: Skipping MySQL registration of outputFile: $outputFile, which failed to copy properly\n";
    print "         (InputFile: $inputFile)\n";
}

Toolsa::PMU_force_register("awaiting new data") if ($doRegister);

exit 0;
