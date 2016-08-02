#!/usr/bin/perl
#
#.. Convert incoming RUC20P GRIB files to mdv format and do conversion
#.. from little to big-endian. Note, this script spawned by rucp_drive.pl.

use Time::Local;
use Getopt::Std;
use Env qw(ADDSHOME);                    # Home dir
use lib "$ADDSHOME/perl_lib";
use Toolsa;
use Monitor;

($prog = $0) =~ s%.*/%%;                 # Determine program basename.
$| = 1;                                  # Unbuffer standard output.
umask 002;                               # Set file permissions.
$debug = 1;                              # Ordinary debugging flag.
$doRegister = 1;                         # Flag to register with procmap.
$doLdata = 1;                            # Flag for creating _latest_data_info file.
$regInterval = 6*3600;                   # Expect RUC20P files approx every 3 hours.

#.. Variables for GRIB to MDV conversion

$grb2mdv = "Grib2toMdv";                    # GRIB-to-MDV conversion program.
$mdv_outdir = "$ADDSHOME/data/gfs/mdv";
$mdv_param_file = "$ADDSHOME/control/gfs/MdvConvert.params"; # Params to be post-processed
$le_file = '"$mdv_outdir/${yyyy}${mm}${dd}/g_${hh}0000/le\.f_${fcst_secs}.mdv"';

my $usage = <<EOF;
Usage:  $prog [-t timeout] [-h]
   -t:       Set timeout for safe_system calls.
   -h:       Prints this usage info.
EOF
&getopts('ht:') || die $usage;
$timeout = ($opt_t - 30) if $opt_t;
$timeout = 180 unless ($timeout);
die $usage if $opt_h;
die $usage if $#ARGV < 0;
$grib_file2 = $ARGV[0];
die "$prog: $grib_file2 is non-existent or zero-size.\n" if (!-s "$grib_file2");

print "Changing to dir $mdv_outdir\n" if $debug;
chdir("$mdv_outdir");

#.. Register with the procmap

if ($doRegister) {
    $ldataHandle=Toolsa::LDATA_init_handle($prog, $debug);
    Toolsa::PMU_auto_init($prog, $prog, $regInterval);
    Toolsa::PMU_force_register("Starting $prog on file: $grib_file");
}

#.. Parse input filename

($yyyy, $mm, $dd, $hh, $ff) = $grib_file2 =~ /(2[0-9]{3})([01][0-9])([0-3][0-9])([0-2][0-9])_F([0-9]{2,3})_.*\.grb2$/;
print "grib2 file is: $grib_file2\n";
$fcst_secs = sprintf("%08d", $ff*3600);
grep($_ = eval, $le_filename = $le_file);
($be_filename = $le_filename) =~ s/le\.//;
print "be_filename = $be_filename\n";

if ( (($hh % 3) == 0) && (($ff % 3) == 0) ) {

    #.. Now, run the GRIB-to-MDV conversion.
 
    print STDOUT "running $grb2mdv on: $grib_file2\n" if $debug;
    Toolsa::PMU_force_register("running $grb2mdv: $grib_file2") if $doRegister;
    $result = Toolsa::safe_system("$grb2mdv -f $grib_file2 -o_f $mdv_outdir -params $mdv_param_file", $timeout);
    print "request for conversion: $grb2mdv -f $grib_file2 -o_f $mdv_outdir\n";
    $is_ok=Monitor::writeLdataInfoToolsa($ldataHandle, $be_filename, 2, -1, $data_time, -1, -1, -1,$ff*1, $debug) if ($doLdata );

    print STDOUT "is_ok (1 for success, 0 otherwise)= $is_ok\n";
}
else {
    print "Skipping non- 3hr run or non- 3hr forecast\n" if $debug;
}
 
unlink($le_filename) || warn "cannot unlink: $le_filename: $!\n";	
Toolsa::PMU_force_register("awaiting new data") if $doRegister;

print STDOUT "completed $prog\n\n" if $debug;
exit 0;
