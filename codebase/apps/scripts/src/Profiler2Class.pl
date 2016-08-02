#!/usr/bin/perl --

use Getopt::Std qw( getopts );
use Getopt::Long;
use File::stat;

#
# Get the needed RAP libraries
#

use Env;
Env::import();
use Env qw(RAP_LIB_DIR);
use Env qw(RAP_SHARED_LIB_DIR);
use lib "$RAP_SHARED_LIB_DIR/perl5/";
use lib "$RAP_LIB_DIR/perl5/";
use Toolsa;

#===============================================================
# EXTENSION FOR OUTPUT FILES
     $ext = '00.pro.class';

# INSTANCE (DEFAULT)
     $opt_instance = "prof2class";            # Procmap instance

# TIME FOR POLLING DATA DIRECTORY (DEFAULT)
     $opt_sleep_secs = 60;

     $bad = '999.0';
#===============================================================


# Get the program basename.
($prog = $0) =~ s|.*/||;

# Unbuffer output.
$| = 1;

$usage =
    "Usage: $prog [options]\n" .
    "Options:\n" .
    "   -help                : Print usage\n" .
    "   -instance <instance> : Process instance for procmap (default = $opt_instance)\n" .
    "   -in_dir <dir>        : Directory location to monitor\n" .
    "   -out_dir <dir>       : Directory location to output class files\n" .
    "   -sleep_secs <secs>   : Number of seconds to sleep before checking for\n" .
    "                          new data (default = $opt_sleep_secs)\n" .
    "   -debug               : Print debug messages\n";


#
# Get the arguments from the command line
#

$result = &GetOptions('help',
                      'instance=s',
                      'in_dir=s',
                      'out_dir=s',
                      'sleep_secs=i',
                      'debug');

if ($result == 0 ||
    $opt_help)
{
    print $usage;
    exit 0;
}

$debug = 1 if defined $opt_debug;   # Print debug messages.

$input_dir = $opt_in_dir;
$out_dir = $opt_out_dir;

#
# Initialize process monitoring
#

&Toolsa::PMU_auto_init($prog, $opt_instance, 120);

#
# Set up signal handlers so we unregister from the process mapper
# when we quit
#

$SIG{INT} = \&tidy_and_exit;
$SIG{KILL} = \&tidy_and_exit;
$SIG{HUP} = \&tidy_and_exit;
$SIG{TERM} = \&tidy_and_exit;


# Miscellaneous variables.
$ldir = $input_dir;             # Local data directory.
$time = 0;

# Verify the local directory.
die "$ldir: no such file or directory" unless -d $ldir;
die "$out_dir: no such file or directory" unless -d $out_dir;

# Monitor the data directory for new files.
while () {

    # Get the directory's modification time.
    ($mtime = (stat($ldir))[$ST_MTIME]) || die "stat: $!";
    $time = time;

    &Toolsa::PMU_force_register("Checking data directory");
    
        opendir(DIR, $ldir) || die "Can't open directory $ldir";

        # Skip the current and parent directory entries.
        @files = grep(!/^\.\.?$/, readdir(DIR));
        closedir(DIR);

        # Exclude subdirectories.
        @files = grep(!-d, @files);

        # Exclude everything but data files, which have
        # filenames ending with the string "WSMN5".
        @files = grep(/WSMN5/, @files);

        # Look for new files.  We'll always send what's lying around
        # initially.
        grep($mark{$_}++, @old_files);
        @new_files = grep(!$mark{$_}, @files);

        # Copy the files to a local directory, renaming them.
        &retrieve(*new_files);

        # Prepare for the next iteration of the loop.
        $old_mtime = $mtime;
        @old_files = @files;
        undef %mark;
        undef @new_files;

    sleep($opt_sleep_secs);

}

exit(0);


# Pull the list of files to the local directory.
sub retrieve {
    local(*files) = @_;

    foreach $rfile (sort @files) {

        &Toolsa::PMU_force_register("converting file $rfile");

        #... Wait til the file hasn't been changed for 5 minutes
        
        $file_done = (-M "$ldir/$rfile" < 0.0004)? 0: 1;
        until ($file_done) {
           sleep 5;
           &Toolsa::PMU_force_register("Checking for size changes $rfile");
           $file_done = 1 if (-M "$ldir/$rfile" > 0.00005);
           # Reset the start time of the script, for testing file ages
           $^T = time;
        }

        # New file pathname.
        $rfile_date = substr( $rfile, 0, 8);
	$ftime = substr( $rfile, 0, 12);

        # Create the temporary directory.
#        mkdir("$out_dir/$rfile_date", 0775) || die "mkdir: $out_dir/$rfile_date: $!"
#            unless -d "$out_dir/$rfile_date";

        $newfile = $out_dir . "/" . $ftime . $ext;

     	open(PFILE, $ldir ."/". $rfile);
     	open(CFILE, ">$newfile"); 

chomp($line = <PFILE>);
@date3 = split ' ', $line;

chomp($line = <PFILE>);
@time3 = split ' ', $line;

chomp($line = <PFILE>);
@stn = split ' ', $line;

chomp($line = <PFILE>);
@lat = split ' ', $line;

chomp($line = <PFILE>);
@long = split ' ', $line;

chomp($line = <PFILE>);
@alt = split ' ', $line;

chomp($line = <PFILE>);
@num_hts = split ' ', $line;

chomp($line = <PFILE>);
@num_flds = split ' ', $line;

chomp($line = <PFILE>);
@delta = split ' ', $line;

chomp($line = <PFILE>);
@first = split ' ', $line;

chomp($line = <PFILE>);
@bad_flag = split ' ', $line;

chomp($line = <PFILE>);
@heading = split ' ', $line;

print CFILE "Data Type:                            Converted Profiler\n";
print CFILE "Project ID:                           TECOM\n";
print CFILE "Launch Site Type/Site ID:             $stn[1]\n";
print CFILE "Launch Location (lon,lat,alt):        $long[1], $lat[1], $long[1], $lat[1], $alt[1]\n";
print CFILE "Launch Time (y,m,d,h,m,s):            $date3[3], $date3[2], $date3[1], $time3[1], $time3[2], $time3[3]\n";
print CFILE "Sonde Type/ID/Sensor ID/Tx Freq:      Converted Profiler(wind only)\n";
print CFILE "Met Precessor/Met Smoothing:          UNKNOWN, UNKNOWN\n";
print CFILE "Winds Type/Processor/Smoothing:       UNKNOWN, UNKNOWN, UNKNOWN\n";
print CFILE "Pre-launch Surface Obs Source:        UNKNOWN\n";
print CFILE "System Operator/Comments:             NONE\n";
print CFILE "/\n";
print CFILE "/\n";

print CFILE " Time  Press  Temp  Dewpt  RH    Uwind  Vwind  Wspd  Dir   dZ       Lon       Lat     Rng   Ang    Alt    Qp   Qt   Qh   Qu   Qv   Quv\n";

print CFILE "------ ------ ----- ----- ----- ------ ------ ----- ----- ----- ---------- --------- ----- ----- ------- ---- ---- ---- ---- ---- ----\n";

while($line = <PFILE>)  {
@val = split ' ', $line;

$cnt = 0;
foreach $value (@val)  
{
     if ($value eq $bad_flag[1])  
     {
        $val[$cnt] = $bad;
     }
  $cnt++;
}

if ($val[0] ne $bad && $val[4] ne $bad)
{
$val[0] = $val[0]+$alt[1];
print CFILE "0  $bad  $bad  $bad  $bad  $val[1]  $val[2]  $val[5]  $val[4]  $bad  $bad  $bad  $bad  $bad  $val[0]  $bad $bad  $bad  $bad  $bad  $bad\n";
}

}    # END OF WHILE LOOP

close(PFILE);
close(CFILE);

    }

}

#
# Exit in a "tidy" fashion
#

sub tidy_and_exit
{
    my $signame = shift;
    &Toolsa::PMU_auto_unregister();
    die "Exiting on signal SIG$signame";
}

