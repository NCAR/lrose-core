#!/usr/bin/perl 
######################################################################################################################################
#
# Scrub and optionally archive old rtfdda cycles  - KLA 6/12/2003 
# Laurie Carson modified it for Magen by archiving for intervals
#
# collect_output (by Sudha Verma) - 2/1/2011
# Added various new configurable options to support multiple projects that archive output files within RAL:
# 
# 1. Added ability to use different intervals for various files  (-I <PerlRegEx:interval>+) 
# 2. Added ability to pack a set of files togetherfile-reg-ex, basename pairs (-B <PerlRegEx:basename>+)
# 3. Added ability to specify a different number of days for each file expression (-N <PerlRegEx:n>+)
# 4. Added ability to specify a set of file regular expressions to be archived in the archive dir (default='wrfout.*')
# 5. Added ability to pack all files if interval option is not given.
# 6. Added ability to specify zip utility to use (default = gzip)
# 7. Added ability to scrub only, archive only or archive and scrub. (default = archive and scrub)
#
# See usage for detailed examples. 
######################################################################################################################################

require "getopts.pl";
use File::Basename;
use File::Find;
use Time::Local;

#-----------------------------
# Verify all user options
#-----------------------------

# program name
$prog = basename($0);

# Unbuffer output.
$| = 1;

$usage = <<EOF;
Usage: $prog -d <data-dir> -n <age> [-o <archive-dir>] [-s] [-a] [-i <interval>] [ -D <data-dir-regex>] [-F <file-regex>] [-I file-regex:interval,... ] [-B file-regex:bundlename,... ] [-N file-regex:age,...] [-z <zip-util>]
Alternative Usage: $prog -c <config-file>
  -d  data directory to archive         ex = /data/cycles/GWDPG/DPG
  -o  output/archive dir                ex = /data/archive/DPG
  -c config-file
  -n  archive/scrub older than AGE days 
  -s  scrub only                        default = both
  -a  archive only                      default = both
  -i  default interval (hours)          default = all
  -z  ZipUtility                        default = 'gzip -fq'

  -D  PerlRegex                         default = '(2\\d\\d\\d)(\\d\\d)(\\d\\d)(\\d\\d)'
  -F  PerlRegex                         default = 'wrfout.*(2\\d\\d\\d)-(\\d\\d)-(\\d\\d)_(\\d\\d).*'
NOTE: both the -D and -F options have embedded groups in their regex.
      Calculation of intervals require these to map to the year, month,
      day, hour in that order. If they are not in the regex, then
      intervals are ignored and all files are taken which match the regex.
  -I  PerlRegex:interval,...            ex = 'dl.*_fcst:1,.*\\.txt:2'
  -B  PerlRegex:bundlename,...          ex = '.*_fcst:forecast,.*_f:final'
  -N  PerlRegex:older than AGE days,..  ex = '.*d1.*:10,.*d2.*:7'

EOF

if ( "$ARGV[0]" eq "-c" ) {
  if ( $#ARGV != 1 ) {
    die "Invalid Parameter: -c requires a file.\n$usage";
  }
  $config_file = "$ARGV[1]";
  read_config("$config_file");
} else {
  &Getopts('D:d:o:n:i:asI:B:N:F:z:') || die $usage;
}

# AGE should always be > 1.  That way, we won't interfere with
# 'in progress' cycles
#
#die "Invalid parameter: bad AGE declaration\n$usage" unless $opt_n and $opt_n > 1;

# Verify the root directory.
#
die "$opt_d: no such file or directory\n$usage" unless -d $opt_d; 
opendir(DIR, $opt_d) || die "Can't open directory $opt_d";

# Archive before scrubbing?
#
$archive="true";
$scrub="true";
$scrubcmd="mv";

@bundles=( );
@intervals=( );
@ages=( );

# Check to see if they've set both the -a & -s options.
if ($opt_s && $opt_a) {
  die "Invalid parameters: can't set both archive-only and scrub-only\n$usage";
}


# If they've set the -s flag, don't archive (just delete stuff).
if ($opt_s) {
  $archive="false";
}

# If they've set the -a flag, don't scrub (just archive), need to change the
# command used to move files into the archive (from "mv" to "cp")
if ($opt_a) {
  $scrub="false";
  $scrubcmd="cp";
}

if ("$archive" eq "true") {
  $archive_dir=$opt_o;
  if (!$opt_o) {
    die "Output directory must be provided for archiving\n$usage";
  }
 
  # Set defaults 
  if (! $opt_F) {
    # file looks like: wrfout_d03_2011-01-17_23:00:00.GRM_P+FCST.gz
    $opt_F='wrfout.*(2\d\d\d)-(\d\d)-(\d\d)_(\d\d).*';
  }
  if (! $opt_z) {
    $opt_z="gzip -fq";
  }

  if (! $opt_D) {
    $opt_D='(2\d\d\d)(\d\d)(\d\d)(\d\d)';
  }

  # If global interval (-i) option is set, then use its value for all matching files
  # If interval is set to 0, treats it same as not being set
  if ($opt_i && $opt_i != 0) {
     if ($opt_i !~ /^[0-9]+$/)
     {
        die("Bad Interval Argument (numeric expected): ".$f."\n$usage");
     }
     unshift(@intervals, [ ".*", $opt_i ]);
  }

  #
  # Push all file-expressions and their intervals
  if ($opt_I) {
     @files_interval = split( ',', $opt_I);
     for $f ( @files_interval ) {
        unshift(@intervals, [ split(':', $f) ]); 
        $t=$intervals[0];
        if (scalar(@$t) != 2) {
           die("Bad Interval Argument: ".$f."\n$usage");
        }
        if ($intervals[0][1] !~ /^[0-9]+$/)
        {
           die("Bad Interval Argument (numeric expected): ".$f."\n$usage");
        }
     }
  }
  
  #

  # Now do the same thing for what files will get bundled
  if ($opt_B) {
     @files_bundle = split( ',', $opt_B);
     for $f ( @files_bundle ) {
        unshift(@bundles, [ split(':', $f) ]);
        $t=$bundles[0];
        if (scalar(@$t) != 2) {
           die("Bad Bundle Argument: ".$f."\n$usage");
        }
     }
  }
} 

#
# Now process age
if ($opt_N) {
  @files_age = split(',', $opt_N);
  for $f ( @files_age ) {
    unshift(@ages, [ split(':', $f) ]);
    $t=$ages[0];
    if (scalar(@$t) != 2) {
      die("Bad Age Argument: ".$f."\n$usage");
    }
    if ($t[1] > $opt_n || $ages[0][1] < 1) {
      die("Bad Age Argument (file ages must be less than scrubbing default and greater than 0): ".$f."\n$usage"); 
    }
  }
}

# How to find the correct subdirectories?
# Some date directories right below the $opt_d directory: ie., 
# $opt_d/ccyymmddhh.  Other date directories are one or more
# level(s) below the $opt_d directory:  $opt_d/x/ccyymmddhh 
#                                  or: $opt_d/x/x/ccyymmddhh 
#                                  or: ???

@dirs = ();

# find all directories with cycle time pattern and add them to dirs
find(\&wanted, $opt_d);

#-----------------------------------------------
# Archive or scrub if modification time > age
#-----------------------------------------------

print "Archive dir is $archive_dir\n";
foreach $rdir (@dirs) {
  # If the modify date is greater than opt_n, always mark it be archived,
  # but only mark it to be scrubbed if scrub is true.
  # Then set the willScrub flag to true
  if ( ( -M $rdir ) > $opt_n ) {
    $willArchive = 1;
    if ( "$scrub" eq "false" ) {
      $willScrub = 0;
    } else {
      $willScrub = 1;
    }
  } else {
    $willScrub = 0;
    $willArchive = 0;
  }
  print "Processing $rdir\n";
  
  # Create output directories
  if ( "$archive" eq "true" ) {
    system("mkdir -p $archive_dir") && 
      die "Could not make $archive_dir\n";

    $ymdh = basename($rdir);
    
    # Get all files based on the "-F" 
    @files = ();
    # I'm changing this to an "ls" instead of a find. A ls goes only one
    # level deep, which is semantically much cleaner. (i.e. what happens
    # if you have a directory that matches the dir pattern under a directory
    # that matches the dir pattern (if you use find, you get the same file
    # twice... ick))
  if ( $opt_S) {
    $rdir = "$rdir/$opt_S";
  }
    @lines = `"ls" $rdir`;
    foreach $line (@lines) {
      chomp($line);
      if ($line =~ /^${opt_F}$/ && ! -d "$rdir/$line" ) {
        push(@files, $line);
      }
    }
      
    #find(\&wantedfcst, $rdir);
  
    # Get the year month day hour from the directory given the
    # regex provided by the user (note: it expects them to be in that
    # order ... it shouldn't be too hard to write a configuration that
    # allows you to reorder them if someone wanted too... but there
    # isn't a reason to do so currently). The regex needs to return at
    # least 4 arguments.
    if ( $ymdh =~ /^.*${opt_D}.*$/ && $4 ) {
      $yy=$1;
      $mm=$2;
      $dd=$3;
      $hh=$4;
      $dirDate=1;
      $cycle_time=timegm(0,0,$hh,$dd,$mm-1,$yy);
    } else {
      $dirDate=0;
    }
   
    foreach $fcst (@files) {
      if (check_age("$rdir/$fcst", @ages) == 1 || $willArchive == 1)
      {
        if ( $dirDate == 1 && $fcst =~ /^.*${opt_F}.*$/ && $4)
        {
          $fileDate = 1;
          $yyf=$1;
          $mmf=$2;
          $ddf=$3;
          $hhf=$4;
          $fcst_time=timegm(0,0,$hhf,$ddf,$mmf-1,$yyf);
          $dtime = $fcst_time - $cycle_time;
	  # See intervals are set for this file
          $interval=get_interval($fcst, @intervals);
        } else {
          $fileDate = 0;
        }

        # This is getting a bit complicated, so I should probably
        # comment it. fileDate specifies whether we found date
        # information in the directory & the file.
        # dtime is the dates embedded in the file and directories names.
        # it is then used to determine if the file is a final file (before
        # the dir date), or if it's a forecast (after the dir date).
        # If it's a forecast, use the interval logic.
        if ( $fileDate == 0 || $dtime <= 0 || $dtime%$interval == 0) {
           if ( ! -e "${archive_dir}/${ymdh}" ) {
             system("mkdir -p $archive_dir/${ymdh}") && 
               die "Could not make $archive_dir/${ymdh}\n";
           }
           system("$scrubcmd ${rdir}/${fcst} $archive_dir/${ymdh}");
	   # Create bundles
	   tar_it("${archive_dir}/${ymdh}/${fcst}", ${ymdh}, @bundles);
           print "${rdir}/$fcst is archived\n";
        }
      }
    }
    # Only gzip if there is something to gzip in the directory and the
    # directory exists.
    if ( -e "${archive_dir}/${ymdh}" ) {
      system("[ `ls $archive_dir/${ymdh} | wc -l` == 0 ] || $opt_z $archive_dir/${ymdh}/*");
    }
  }

  # Only delete directory if modified date was older than n and we're deleteing directories
  if ($willScrub == 1) {
    print " ... scrubbing $rdir \n";
    system("rm -rf ${rdir}");
  }
}

sub wanted() {
  # return unless the basename of the file matches the dir_pattern and the file
  # is actually a directory. 
  return unless $_ =~ m/^${opt_D}$/ && -d "$_";
  push @dirs, $File::Find::name;
}

#sub wantedfcst() {
#  return unless  /^$opt_F/;	
#  push @files, $_;
#}

# Helper subroutine : tar_id
# Takes all the regular expressions in the -B option and if input file matches a regex in bundles, then adds it to tar-file (aka bundle)
# 
sub tar_it
{
  my ($file, $ymdh, @bundles) = @_;
  my ($base) = basename($file);
  my ($dir) = dirname($file);
  for $j (0 .. scalar(@bundles)) {
    if ( $file =~ m/^($bundles[$j][0])$/ )
    {
      # Create new tar file if it does not exist
      $tar_file="${ymdh}_${bundles[$j][1]}.tar";
      if ( ! -e "${dir}/${ymdh}_${bundles[$j][1]}.tar" )
      {
        system("cd $dir ; tar -cf ".${ymdh}."_".$bundles[$j][1].".tar ".$base) &&
          die("Could not use $base to create tar file ".${ymdh}."_".$bundles[$j][1].".tar\n");
      }
      else
      {
        system("cd $dir ; tar -rf ".${ymdh}."_".$bundles[$j][1].".tar ".$base) &&
          die("Could not add $base to tar file ".${ymdh}."_".$bundles[$j][1].".tar\n");
      }
      # file is not removed if tar is unsuccessful
      system("rm ".$file) &&
        die("Could not remove ".$file);
      return;
    }
  }
}

# Helper subroutine: get_interval
# Checks to see if the input file matches any of the regular expression in intervals
#
sub get_interval
{
  my ($file, @intervals) = @_;
  for $j (0 .. scalar(@intervals)) {
    if ( $file =~ m/^($intervals[$j][0])$/ )
    {
      return $intervals[$j][1] * 3600;
    }
  }
  # This case is only ever reached if we do not have a match due to
  # a -I option and no -i option was specified. Returning 1 will result
  # in every file matching the interval.
  return 1;
}

# Helper subroutine: check_age
# Returns 1 if the modified date of file is > ndays
sub check_age
{
  my ($file, @ages) = @_;
  for $j (0 .. scalar(@ages)) {
    if ( $file =~ m/^($ages[$j][0])$/ )
    {
       if ( ( -M $file ) > $ages[$j][1] ) {
          return 1;
       } else {
          return 0;
       }
    }
  }
  return 0;
}

# Helper routine: read_config
# Reads the config file, dies if any errors occur.
# Note: you need to keep this function in "line" with the Getopts calll above.
sub read_config
{
  my ($file) = @_;
  print "Reading config file $file\n"; 
  # Fields and what they map to:
  # archive_only = -a = opt_a
  # scrub_only = -s = opt_s
  # data_dir = -d = opt_d
  # archive_dir = -o = opt_o
  # default_age = -n = opt_n
  # default_interval = -i = opt_i
  # file_pattern = -F = opt_F
  # interval = -I = opt_I
  # bundle = -B = opt_B
  # age = -N = opt_N
  # zip_utility = -z = opt_z
  # dir_pattern = -D = opt_D
  if (!open(CFG, "$file")) {
    die "Could not open config file: $file.\n";
  }
  @cfg_data=<CFG>;
  close(CFG);
  foreach $cfg_line (@cfg_data) {
    chop($cfg_line);
    $cfg_line =~ s/^\s*//;
    $cfg_line =~ s/\s*$//;
    if (($cfg_line !~ /^#/) && ($cfg_line ne "") ) {
      ($name, $value) = split(/=/,$cfg_line);
      $name =~ s/^\s*//;
      $name =~ s/\s*$//;
      $value =~ s/^\s*//;
      $value =~ s/\s*$//;
      if ( "$name" eq "archive_only" ) {
        $opt_a="true";
      } elsif ( "$name" eq "scrub_only" ) {
        $opt_s="true";
      } elsif ( "$name" eq "data_dir" ) {
        $opt_d=$value;
      } elsif ( "$name" eq "archive_dir" ) {
        $opt_o=$value;
      } elsif ( "$name" eq "default_age" ) {
        $opt_n=$value;
      } elsif ( "$name" eq "default_interval" ) {
        $opt_i=$value;
      } elsif ( "$name" eq "file_pattern" ) {
        $opt_F=$value;
      } elsif ( "$name" eq "dir_pattern" ) {
        $opt_D=$value;
      } elsif ( "$name" eq "sub_dir_pattern" ) {
        $opt_S=$value;
      } elsif ( "$name" eq "interval" ) {
        if ( $opt_I ) {
          $opt_I="$opt_I,$value";
        } else {
          $opt_I=$value;
        }
      } elsif ( "$name" eq "age" ) {
        if ( $opt_N ) {
          $opt_N="$opt_N,$value";
        } else {
          $opt_N=$value;
        }
      } elsif ( "$name" eq "bundle" ) {
        if ( $opt_B ) {
          $opt_B="$opt_B,$value";
        } else {
          $opt_B=$value;
        }
      } elsif ( "$name" eq "zip_utility" ) {
        $opt_z=$value;
      } else {
        die "Unknown config file parameter found: $name.\n";
      }
    }    
  }
}
