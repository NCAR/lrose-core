#!/usr/bin/perl
#
# check_jars: Check installed java jar files
#
# Usage:
#   daily_backup.pl <options>
#
#    Options:
#       -help                : Print usage
#       -debug               : debugging on
#       -label               : label for output
#       -list                : check list
#
#=============================================================================
# Get the needed PERL supplied library modules.
#=============================================================================

use Getopt::Long;
use Getopt::Std;
use Time::Local;

#=============================================================================
# Set up the needed environment variables
#=============================================================================

use Env qw(RAP_LIB_DIR);

#
# Get the program basename.
#

($prog = $0) =~ s|.*/||;

#
# Unbuffer standard output
#

$| = 1;

#set flushing

$OUTPUT_AUTOFLUSH = 1;

#
# Set file permissions
#

umask 002;

# Initialize command line arguments

$opt_debug = "";
$opt_params = "";
$opt_label = "";

$usage =
    "\nUsage: $prog <options>\n" .
    "Options:\n" .
    "  -help                 :Print usage\n" .
    "  -dir                  :jar dir (required)\n" .
    "  -label                :label for messages\n" .
    "  -list                 :jar check list (required)\n" .
    "  -debug                :debugging on\n" .
    "\n";

# Get the arguments from the command line

$results = &GetOptions('help',
                       'dir:s',
                       'label:s',
                       'list:s',
                       'debug');

if ($opt_help) {
    print $usage;
    exit 0;
}

if (!$opt_list) {
    print "\nERROR - you must specify the jar list\n";
    print $usage;
    exit 0;
}

if (!$opt_dir) {
    print "\nERROR - you must specify the jar dir\n";
    print $usage;
    exit 0;
}

# read in the list of installed jars, from the jar directory

opendir(DIRHANDLE, $opt_dir) || die "Cannot open dir: $opt_dir: $!";
$count = 0;
foreach $name (sort readdir(DIRHANDLE)) {
    if ($name =~ m/.jar$/) {
        @installed_jars[$count] = $name;
        $count++;
    }
}
closedir(DIRHANDLE);

if ($opt_debug) {
    printf STDERR "=============== INSTALLED JARS ===============\n";
    foreach $jar (@installed_jars) {
        print STDERR "$jar\n";
    }
    print STDERR "================================================\n";
}

# read in list of required jars, from check list

open(CHECKLIST, "< $opt_list") || die("can't open $opt_list: $!");
$count = 0;
while(($line = <CHECKLIST>)) {
    chop($line);
    if (!($line =~ m/^\#/)) {
      ($jarname, $comments) = split(/#/, $line, 2);
      $jarname =~ s/ //g;
      $comments =~ s/^ //;
      @required_jars[$count] = $jarname;
      $jar_comments{$jarname} = $comments;
      $count++;
    }
}
close(CHECKLIST);

if ($opt_debug) {
    printf STDERR "================ REQUIRED JARS ===============\n";
    foreach $jar (@required_jars) {
        print STDERR "$jar\n";
    }
    print STDERR "================================================\n";
}

# find jars which are not installed

$nmiss = 0;
foreach $rjar (@required_jars) {
    $found = 0;
    foreach $ijar (@installed_jars) {
        if ($ijar eq $rjar) {
            $found = 1;
            break;
        }
    }
    if (!$found) {
        @missing_jars[$nmiss] = $rjar;
        $nmiss++;
    }
}

# if jars are missing, print out error message

if ($nmiss > 0) {
    printf STDERR "======================>> ERROR <<====================\n";
    printf STDERR "====>> INCOMPLETE $opt_label JARS INSTALLATION <=<====\n\n";
    printf STDERR "  $nmiss missing jar files:\n\n";
    foreach $mjar (@missing_jars) {
        printf STDERR "  jar file \'$mjar\' missing\n";
        $comments = $jar_comments{$mjar};
        printf STDERR "    NOTE: $comments\n";
        $mjar =~ /jar(.+)\.a/;
        $jarname = $1;
        printf STDERR "    Check build in directory jars/$jarname\n\n";
    }
    exit 1;
} else {
    printf STDERR "====================>> SUCCESS <<======================\n";
    printf STDERR "====>> ALL $opt_label JARS SUCCESSFULLY INSTALLED <<====\n";
    exit 0;
}
