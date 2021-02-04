#!/usr/bin/perl
#
# Simple script to run pqinsert to notify ADDS that model GRIB data
# has been received at RAP by LDM-CONDUIT feed and ADDS may now retrieve
# the files for their use.
#
#modified May 25, 2006 - Gary Cunning & Paul Prestopnik.  Added destination_dir argument.
#                                                         Added -n argument.

use Getopt::Std;
use Time::Local;

($prog = $0) =~ s%.*/%%;                 # Determine program basename.
$| = 1;                                  # Unbuffer standard output.
umask 002;                               # Set file permissions.

#initialize pq_insert to default (true)
$pq_insert = 1;

#...------------...Usage Info...-----------------------

$usage = <<EOF;
Usage: $prog [-dhn] [-p productId] status_file destination_dir
  -d  Print out debug information
  -h  Print out this usage information
  -n  Do not insert status message into product queue
  -p  set productId; otherwise, retrieved from status_file naming convention
      Status and GRIB filenames take form of:  20041104_i18_f000_GFS004.status|grb2
EOF
&getopts('dhp:') || die $usage;
$debug = 1 if $opt_d;
$pq_insert = 0 if $opt_n;
die $usage if $opt_h;
die $usage if $#ARGV < 0;


#...Get destination dir from arguments.
$destination_dir = $ARGV[1];

#...From status filename, create GRIB filename.

$status_file = $ARGV[0];
($grib_file = $status_file) =~ s/\.status/.grb2/;
($gfile = $grib_file) =~ s%.*/%%;

#...Make certain both status and GRIB files exist.

die ("Status file, $status_file, does not exist\n\n") unless (-e "$status_file");
die ("GRIB file, $grib_file, does not exist\n\n") unless (-e "$grib_file");

#...From status filename, get date and model info.

($yyyy, $mm, $dd, $hh, $fff, $model) = $status_file =~ /(2[0-9]{3})([0-1][0-9])([0-3][0-9])_i([0-2][0-9])_f([0-9]{3})_(.*)\.status$/;
$yyyymmdd = $yyyy . $mm . $dd;
$ddhh = $dd . $hh;
if ( ($fff*1) >= 100 ) {
    $fhr = sprintf("%03d", $fff*1);
} else {
    $fhr = sprintf("%02d", $fff*1);
}

#...Command-line option may override productId from model name.

if ($opt_p) {
    $productId = $opt_p;
} else {
    $productId = $model;
}

#...Create dir name of final resting place for GRIB files when completely received.

$final_dir = "$destination_dir/$productId/$yyyymmdd";
mkdir ("$final_dir") unless (-d "$final_dir");

#...Open status file and retrieve supposed size (bytes) of GRIB file.

$numExpect = 0;
open (SFILE, "$status_file") || die ("Could not open $status_file\n$!\n\n");
while (<SFILE>) {
    chop;
    if (/^Inserted\s+([0-9]+)\s+of\s+([0-9]+)/) {
        $numSoFar = $1;
        $numExpect = $2;
    }
}
close (SFILE);

die ("Status file, $status_file, does not contain the needed byte info.\n\n") unless $numExpect;

if ($numSoFar == $numExpect) {

    $file_done = (-M "$grib_file" < 0.0007)? 0: 1;
    until ($file_done) {
        print STDOUT "File $grib_file still being filled with data $file_done\n" if $debug;
        sleep 5;
        $file_done = 1 if (-M "$grib_file" < 0.0007);
        $^T = time;
    }

# Added 60sec wait for delay grib files  (3/21/2005 Celia - as per Nancy Rehak's request)
    sleep 60;
#
    $file_size = (-s "$grib_file");

    if ($file_size == $numExpect) {
        $productId = "\U$productId" unless ($opt_p);
        print STDOUT "GRIB file, $grib_file, fully received ($file_size).\n" if $debug;
        print STDOUT "    moving to $final_dir/$gfile.\n" if $debug;
        ((system ("/bin/mv -f $grib_file $final_dir/$gfile") >> 8) == 0) || warn "SYSTEM: mv failed.\n$!\n\n";
        $now = time();
        $temp_file = "/tmp/grib_notify_$productId.tmp";
        unlink ("$temp_file") if (-e "$temp_file");
        open (TMP, ">$temp_file") || die ("Cannot open $temp_file\n$!\n\n");
        print TMP "$status_file $now\n";
        close (TMP);
	if ($pq_insert == 1) {
	    $cmd = "pqinsert -l /home/ldm/logs/grib_notify.log -p \"GRIB AVAL ${ddhh}00 GRIB $productId $fhr\" -f OTHER $temp_file";
	    print STDOUT "Will run pqinsert to notify downstream users.\n  ($cmd) ... " if $debug;
	    ((system("$cmd") >> 8) == 0) || warn "SYSTEM: $cmd failed.\n$!\n\n";
	}
	unlink ("$status_file");
	print STDOUT "done\n" if $debug;
	
    } else {
        print STDOUT "NCEP shipped $numExpect bytes but $grib_file contains only $file_size bytes.\n" if $debug;
    }

} else {

    print STDOUT "Thus far, NCEP shipped $numSoFar of $numExpect bytes over LDM-CONDUIT.\n" if $debug;

}

exit 0;
