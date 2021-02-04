#!/usr/bin/perl
# Script:  nightly_build_tracker.pl
#   Info:  Used to track applications and libraries that are built
#          by the cvs nightly build system. (checkout_and_build.pl)
#          Uses the output of check_build_dirs.pl script as input.  
#          Keeps track of when app/lib started failing to build 
#          and optionaly notifies via email.  Uses stdout if no email file given.
# Inputs:  check_build.out  Loctaion of checkout_and_build.pl output file.
#          build_track.txt  File to use to keep track of failed builds, will create.
#                           (this should not be put in the build dir, 
#                            the build script can/will delete it)
#          [mail_list]      Optional list of email addresses to use if new build failure.
# Author:  Jason Craig  Dec 2015

my %machine_descriptions = (
    "k5"  => "RedHat 5 32bit",
    "k6"  => "",
    "k7"  => "",
    "k8"  => "RedHat 5 64bit",
    "k9"  => "Debian wheezy 64bit",
    "k10" => "Debian wheezy 32bit",
    "k11"  => "Debian jessie 64bit",
    "k12"  => "CentOS6.7 (RedHat6)",
    "k12.rap.ucar.edu"  => "CentOS6.7 (RedHat6)",
    "k13"  => "CentOS7.2 (RedHat7)",
    "k13.rap.ucar.edu"  => "CentOS7.2 (RedHat7)",
    );

if($#ARGV < 1) {
  print "Usage: nightly_build_tracker.pl check_build.out build_track.txt [mail_list]\n";
}

my $inputfile = shift(@ARGV);
my $trackfile = shift(@ARGV);
my $maillist = "";
if($#ARGV == 0) {
    $maillist = shift(@ARGV);
    $mailfixed = "";
    $mailfailed = "";
}

my $header, $machine, $os, $date = "";
my @failures = ();
open INFILE, "<$inputfile" or die $!;
my $i = 0;
foreach $line (<INFILE>) 
{
  if($i == 0 && index($line, 'Current machine:') == -1) {
    $header = substr($line, 0, -1);
  }
  if(($pos = index($line, 'Current machine:')) != -1) {
      $machine = substr($line, $pos+17,-1);
  }
  if(($pos = index($line, 'HOST_OS:')) != -1) {
      $os = substr($line, $pos+9,-1);
  }
  if(($pos = index($line, 'Date:')) != -1) {
      $date = substr($line, $pos+6,-1);
  }
  if(($pos = index($line, 'does not exist')) != -1) {
      $failed = substr($line, 0, $pos);
      $failed =~ s/^\s+|\s+$//g;  #remove whitespace
      push(@failures, $failed);
  }
  $i++;
}
close INFILE;
if($maillist eq "") {
  if($header ne "") {
    print "$header\n";
  }
  print "Machine: $machine\n";
  print "HOST_OS: $os\n";
  print "Date: $date\n";
}

my ($day_of_week, $Month, $day, $hour_min_sec, $year) = split(" ", $date);
if($Month eq "") {
    print "ERROR: Failed to parse date string!\n";
}
my $day =  "$Month $day $year";

my @fail_dates = ();
my @fail_times = ();
for (my $i=0; $i <= $#failures; $i++) {
    push(@fail_dates, $day);
    push(@fail_times, $hour_min_sec);
}

my $track = 1;
open TRACKFILE, "<$trackfile" or $track = 0;
if($track == 1) {
  my $mac = <TRACKFILE>;
  my $los = <TRACKFILE>;
  my $lday = <TRACKFILE>;
  foreach $line (<TRACKFILE>) 
  {
    chomp($line);
    my $isfailing = 0;
    (my $prevfail, $prevdate, $prevtime) = split(", ", $line);
    for (my $i=0; $i <= $#failures; $i++) {
      if($prevfail eq $failures[$i]) {
	  $isfailing = 1;
	  $fail_dates[$i] = $prevdate;
	  $fail_times[$i] = $prevtime;
      }
    }
    if($isfailing == 0) {
      if($maillist eq "") {
        print "$prevfail has been fixed\n";
      } else {
	$mailfixed = $mailfixed."\t\t$prevfail\n";
      }
    }
  }
  close TRACKFILE
} 


open TRACKFILE, ">$trackfile" or die $!;
print TRACKFILE "$machine\n";
print TRACKFILE "$os\n";
print TRACKFILE "$day\n";
for (my $i=0; $i <= $#failures; $i++) {
  if($fail_dates[$i] eq $day && $fail_times[$i] eq $hour_min_sec) {
      if($maillist eq "") {
	print "New build failure: $failures[$i]\n";
      } else {
	$mailfailed = $mailfailed."\t\t$failures[$i]\n";
      }
  }
  print TRACKFILE "$failures[$i], $fail_dates[$i], $fail_times[$i]\n"
}
close TRACKFILE;

#if($maillist eq "" && $mailfailed eq "" && $mailfixed eq "") {
#    print "No changes found.\n";
#}

if($maillist ne "" && ($mailfailed ne "" || $mailfixed ne "")) {
    ($is_ok, $n_mailnames)=&get_mail_list($maillist, *arr_mailnames, 0);
    my $description = $machine_descriptions{$machine};
    open TMP, ">tmp.txt" or die $!;
    if($header ne "") {
      print TMP "$header\n";
    }
    print TMP "Machine: $machine\n";
    print TMP "HOST_OS: $os\n";
    print TMP "Date: $date\n\n";
    print TMP "New build failures:\n";
    print TMP $mailfailed;
    if($mailfixed ne "") {
      print TMP "\n\nNow Fixed that were failing:\n";
      print TMP $mailfixed;
    }
    close TMP;
    $subject="Build results for host: $machine, OS: $description";
    foreach $mailname (@arr_mailnames) {
	system("mail -s \"$subject\" $mailname < tmp.txt");
    }
    system("rm tmp.txt");
}



#-------------------------------------------------------------------------------
#
# Subroutine: get_mail_list()
#
# Usage: ($return_val, $narr)=&get_mail_list($file, *arr, $debug);
#
# Function:   To read the input $file and build an array of mail addresses.
#
# Input:      $file             filename containing the mail list
#             $debug            flag
#
# Output:     $return_val       1 on success, 0 on error
#             *arr              array of mail addresses in $file
#             $narr             size of *arr
# 

sub get_mail_list {
    local($filename, *arr, $debug) = @_;

    # Local variables

    local($return_val, $subname);
    local($dbg2, $narr);
    local($is_ok, $counter, $line);

    # Set defaults
    
    $return_val=0;
    $narr=0;
    $subname="get_mail_list";
    $dbg2=0;

    # Debug

    if ($debug == 2) {
	$dbg2=1;
    }

    # Initialize

    $counter=0;

    # Open file with list of mail aliases

    if (!open(MAILLISTFILE, $filename)) {
	print(STDERR "$subname: Cannot open mailfile $filename\n");
	return($return_val, $narr);
    }

    # Read the file, skip lines with leading # (comments) or blank lines

    while ($line = <MAILLISTFILE>) {
	if (($line !~ /^\#/) && ($line =~ /\w/)) {
	    chop($line);           # remove the trailing newline
	    $arr[$counter]=$line;
	    $counter++;
	}
    } #endwhile
    
    close(MAILLISTFILE);

    # Debug

    if ($debug) {
	print(STDERR "$subname: file: $filename, counter: $counter\n");
    }

    # Done

    $narr=$counter;
    $return_val=1;
    return($return_val, $narr);
}
