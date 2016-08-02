#!/usr/bin/perl
#
# GFS files arrive per notification
# by LDM when available. This script will poll the GriddedDataFiles DB-table
# to determine when a new file is "ready" and will spawn Grib2toMdv via
# gfs_grb2mdv.pl
# 
#
# This script runs continuously (kept going by procmap), sleeping between
# checks for new data.
#

use Time::Local;
use Getopt::Std;
use Env qw(ADDSHOME);                    # Home dir
use lib "$ADDSHOME/perl_lib";
use Toolsa;
use GriddedData_DbUtils;

($prog = $0) =~ s%.*/%%;                 # Determine program basename.
$| = 1;                                  # Unbuffer standard output.
umask 002;                               # Set file permissions.
$debug = 1;                              # Ordinary debugging flag.

$gridName = "GFS004";                       # Name of grid (modelName in GriddedDataFiles).
$instance = "$gridName";                 # Name of instance for procmap registration.
$grb2mdvScript = "gfs_grb2mdv.pl";       # Name of script for GRIB->MDV conversion.
$plotScript = "";                        # Name of plotting script. Not needed for GFS.
$mdvTimeout = 360;                       # Timeout for grib2mdv script.
$plotTimeout = 360;                      # Timeout for plot script.
$sleepInterval = 60;                     # Sleep interval between checks for new data.
$regInterval = 360;                      # Procmap registration interval.
$dontDoIt = 0;                           # Quick flag to disable actual tasks.

my $usage = <<EOF;
Usage:  $prog [-i instance_name] [-n] [-h]
   -i:       Set instance to instance_name.
   -n:       skip actual tasks (used for debugging).
   -h:       Prints this usage info.
EOF
&getopts('nhi:') || die $usage;
$instance = $opt_i if $opt_i;
die $usage if $opt_h;
$dontDoIt = 1 if $opt_n;

#.. Register with the procmap

$ldataHandle=Toolsa::LDATA_init_handle($prog, $debug);
Toolsa::PMU_auto_init($prog, $instance, $regInterval);
Toolsa::PMU_force_register("Starting $prog");

while (1) {

    @procFNames = ();
    $numFound = 0;

    print STDOUT "Retrieving list of any files that need to be post-processed\n" if $debug;
    ($numFound, @procFNames) = GriddedData_DbUtils::filesNeedingPostproc($gridName);
    print STDOUT "$procFNames\n" if $debug;

    foreach $procFName (@procFNames) {

        unless ( -e $procFName ) {
            print STDOUT "ERROR!!!! File $procFName does not exist!\n";
            next;
        }

        $file_done = 0;
        until ($file_done) {
            sleep 2;
            $file_done = 1 if (-M "$procFName" > 0.0003);
            $^T = time;
        }
 
        Toolsa::PMU_force_register("spawning GRIB-to-MDV conversion script:  $grb2mdvScript");
        print STDOUT "\tspawning GRIB-to-MDV conversion script:  $grb2mdvScript -t $mdvTimeout $procFName\n" if $debug;
        (Toolsa::safe_system("$grb2mdvScript -t $mdvTimeout $procFName", $mdvTimeout)) unless $dontDoIt;

        Toolsa::PMU_force_register("spawning plot script:  $plotScript");
        print STDOUT "\tspawning plot script:  $plotScript -t $plotTimeout $procFName\n" if $debug;
        (Toolsa::safe_system("$plotScript -t $plotTimeout $procFName", $plotTimeout)) unless $dontDoIt;

        print STDOUT "\tresetting postProcessFlag\n" if $debug;
        (GriddedData_DbUtils::reset_postProcFlag($procFName, 1)) unless $dontDoIt;

        print STDOUT "Completed $procFName\n\n" if $debug;
    }

    #.. Go to sleep for a bit
    print STDOUT "nothing new, awaiting new $gridName grids\n" if $debug;
    Toolsa::PMU_force_register("awaiting new $gridName files");
    sleep($sleepInterval);
 
}

Toolsa::PMU_unregister();

warn "WARNING: how on earth did we get here?  (message from $prog)\n\n";
exit 0;
