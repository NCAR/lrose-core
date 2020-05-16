#!/usr/bin/perl
#
# check_libs: Check installed libraries
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
    "  -dir                  :lib dir (required)\n" .
    "  -label                :label for messages\n" .
    "  -list                 :lib check list (required)\n" .
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
    print "\nERROR - you must specify the lib list\n";
    print $usage;
    exit 0;
}

if (!$opt_dir) {
    print "\nERROR - you must specify the lib dir\n";
    print $usage;
    exit 0;
}

# read in the list of installed libs, from the lib directory

opendir(DIRHANDLE, $opt_dir) || die "Cannot open dir: $opt_dir: $!";
$count = 0;
foreach $name (sort readdir(DIRHANDLE)) {
    if ($name =~ m/.a$/) {
        @installed_libs[$count] = $name;
        $count++;
    }
}
closedir(DIRHANDLE);

if ($opt_debug) {
    printf STDERR "=============== INSTALLED LIBS ===============\n";
    foreach $lib (@installed_libs) {
        print STDERR "$lib\n";
    }
    print STDERR "================================================\n";
}

# read in list of required libs, from check list

open(CHECKLIST, "< $opt_list") || die("can't open $opt_list: $!");
$count = 0;
while(($line = <CHECKLIST>)) {
    chop($line);
    if (!($line =~ m/^\#/)) {
      ($libname, $comments) = split(/#/, $line, 2);
      $libname =~ s/ //g;
      $comments =~ s/^ //;
      @required_libs[$count] = $libname;
      $lib_comments{$libname} = $comments;
      $count++;
    }
}
close(CHECKLIST);

if ($opt_debug) {
    printf STDERR "================ REQUIRED LIBS ===============\n";
    foreach $lib (@required_libs) {
        print STDERR "$lib\n";
    }
    print STDERR "================================================\n";
}

# find libs which are not installed

$nmiss = 0;
foreach $rlib (@required_libs) {
    $found = 0;
    foreach $ilib (@installed_libs) {
        if ($ilib eq $rlib) {
            $found = 1;
            break;
        }
    }
    if (!$found) {
        @missing_libs[$nmiss] = $rlib;
        $nmiss++;
    }
}

# if libs are missing, print out error message

if ($nmiss > 0) {
    printf STDERR "======================>> ERROR <<====================\n";
    printf STDERR "====>> INCOMPLETE $opt_label LIBS INSTALLATION <=<====\n\n";
    printf STDERR "  $nmiss missing libraries:\n\n";
    foreach $mlib (@missing_libs) {
        printf STDERR "  library \'$mlib\' missing\n";
        $comments = $lib_comments{$mlib};
        printf STDERR "    NOTE: $comments\n";
        $mlib =~ /lib(.+)\.a/;
        $libname = $1;
        printf STDERR "    Check build in directory libs/$libname\n\n";
    }
    exit 1;
} else {
    printf STDERR "====================>> SUCCESS <<======================\n";
    printf STDERR "====>> ALL $opt_label LIBS SUCCESSFULLY INSTALLED <<====\n";
    exit 0;
}
