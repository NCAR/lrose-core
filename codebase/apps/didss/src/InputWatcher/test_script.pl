#!/usr/bin/perl
#
# Test script to mail message file

# system Perl modules

use Time::Local;
use Getopt::Long;
 
$debug = 0;
$verbose = 0;

#------------------------- Parse command line ----------------------------

($prog = $0) =~ s%.*/%%;                 # Determine program basename.
&GetOptions("debug!" => \$debug,
	    "verbose!" => \$verbose,
	    "input_file_path=s" => \$input_file_path,
	    "file_modify_time=s" => \$file_modify_time);

$data_path = $full_path . '/' . $rel_data_path;

if ($debug) {
    print STDERR "Running test_script.pl:\n";
    print STDERR "  input_file_path: $input_file_path\n";
    print STDERR "  file_modify_time: $file_modify_time\n";
}

$cmd = "mail junk@gmail.com -s RdasTest < $input_file_path";
if ($debug) {
    print STDERR "Running mail command: $cmd\n";
}
system($cmd);

exit 0;
