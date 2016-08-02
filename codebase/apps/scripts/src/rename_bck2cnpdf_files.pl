#!/usr/bin/perl
#
# Get the needed PERL supplied library modules.
#
use Time::Local;

#
# Get the program basename.
#
($prog = $0) =~ s|.*/||;

#
# Unbuffer standard output
#
$| = 1;

#
# Set file permissions
#
umask 002;

#
# Loop through the bck2cnpdf files in the current directory and rename
# them appropriately
#
@file_list = map { glob($_) } "bck2cnpdf*";
for ($i = 0; $i < @file_list; $i++)
{
    $filename = ${file_list[$i]};
    print "file name = <$filename>\n";

    if (($date, $hour, $fcst) = ($filename =~ /bck2cnpdf.(\d\d\d\d\d\d\d\d)(\d\d)(\d\d)/))
    {
	print "date = <$date>\n";
	print "hour = <$hour>\n";
	print "fcst = <$fcst>\n";

	$new_directory = "f$fcst/$date";
	$new_filename = "${hour}0000.grb";

	print "new_directory = <$new_directory>\n";
	print "new_filename = <$new_filename>\n";

	system("mkdir -p $new_directory");
	system("mv $filename $new_directory/$new_filename");
    }
    else
    {
	print "could not parse filename <$filename>\n";
    }

}
