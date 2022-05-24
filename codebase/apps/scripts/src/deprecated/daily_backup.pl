#!/usr/bin/perl
#
# daily_backup.pl: Runs backup on date provided on command line or
#  if no date is specified, looks at current UTC date and archives
#  data for previous day. See example parameter file archive_data_list.
#
# Usage:
#   daily_backup.pl <options>
#
#    Options:
#       -help                : Print usage
#       -date                : YYYYMMDD
#       -print_params        : prints a default parameter file
#       -params              : archive parameter file name
#       -test                : do not execute system calls
#       -clean_up            : remove tar files after sending to MSS
#       -verify              : prints table of file size sent and received on MSS
#       -sendMail            : email address (sends warning messages to this email)
#       -debug               : debugging on
#       -noMSS               : option to tar files but not send to MSS
#       -nogzip              : option to not gzip the files in the archive
#
#===================================================================================
# Get the needed PERL supplied library modules.
#===================================================================================
use Getopt::Std qw( getopts );
use Getopt::Long;                                                                           
use Time::Local;

#===================================================================================
# Set up the needed environment variables
#===================================================================================
use Env qw(RAP_DATA_DIR);
use Env qw(PROJ_DIR);
use Env qw(HOST);

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

$mailprog = 'sendmail';

# Initialize command line arguments
$opt_date = "";
$opt_debug = "";
$opt_test = "";
$opt_params = "";
$opt_print_params = "";
$opt_verify = "";
$opt_sendMail = "";
$opt_noMSS = "";
$opt_nogzip = "";

$usage =
    "\nUsage: $prog <options>\n" .
    "Options:\n" .
    "  -help                 :Print usage\n" .
    "  -date                 :YYYYMMDD\n" .
    "  -print_params         :prints default parameter file\n" .
    "  -params               :archive parameter file name\n" .
    "  -test                 :do not execute system calls\n" .
    "  -clean_up             :removed tar files after sending to MSS\n" .
    "  -verify               :prints table of file size sent and received on MSS\n" .
    "  -sendMail             :email address (sends warning messages to this email)\n" .
    "  -debug                :debugging on\n" .
    "  -project              :project number to charge request to MSS.\n" .
    "                         If not set msrcp looks for project number in\n" .
    "                         .ncar_project file in users home directory or\n" .
    "                         checks for existence of the NCAR_PROJECT environment variable.\n" .
    "  -noTAR                :option to send to MSS but not TAR files.\n" .
    "  -noMSS                :option to tar files but not send to MSS.\n" .
    "  -nogzip               :option to not gzip the files in the archive.\n\n";

# Get the arguments from the command line
$results = &GetOptions('help',
                       'date:i',
                       'print_params',
                       'params:s',
                       'verify',
                       'sendMail:s',
                       'test',
                       'clean_up',
                       'debug',
                       'project:i',
                       'noTAR',
                       'noMSS',
                       'nogzip');

if($opt_print_params) {

    print "\########################################################################################################################\n";
    print "\# TAR FILE NAME           - Name that will be giving to tar file. YYYYMMDD is prepended and .tar is appended to file.\n";
    print "\#                         - final file will look like: YYYYMMDD_TARFILENAME.tar\n";
    print "\# LOCAL ARCHIVE DIRECTORY - Were to write tar files on local disk.\n";
    print "\# FILE_TYPE               - options:\n";
    print "\#                            time - data files stored as YYYYMMDD/HHMMSS (i.e. mdv files)\n";
    print "\#                            date - data files stored as YYYYMMDD.* (i.e. spdb files)\n";
    print "\#                            forecast - data files stored in RAP forecast directory format\n";
    print "\#                               (i.e. YYYYMMDD/g_HHMMSS/f_SSSSSSSS.ext)\n";
    print "\#                            hourly_time - data files stored as YYYYMMDD/HHMMSS (i.e. mdv files)\n";
    print "\#                               Files are put into separate tar files for each hour of data\n";
    print "\#                               NOTE THAT EACH HOURLY DATASET MUST BE PUT INTO A DIFFERENT TAR FILE.\n";
    print "\#                            hourly_dir - data files stored as YYYYMMDD/HHMMSS/filename\n";
    print "\#                               Files are put into separate tar files for each hour of data\n";
    print "\#                               NOTE THAT EACH HOURLY DATASET MUST BE PUT INTO A DIFFERENT TAR FILE.\n";
    print "\#                            hourly_swp - data files stored as YYYYMMDD/swp.YYJJJHH*\n";
    print "\#                               Files are put into separate tar files for each hour of data\n";
    print "\#                               NOTE THAT EACH HOURLY DATASET MUST BE PUT INTO A DIFFERENT TAR FILE.\n";
    print "\#                            hourly_ncswp - data files stored as YYYYMMDD/ncswp*YYYYJJJ_HH*\n";
    print "\#                               NOTE THAT EACH HOURLY DATASET MUST BE PUT INTO A DIFFERENT TAR FILE.\n";
    print "\# DATA PATH               - Were to find data to archive (relative to \$RAP_DATA_DIR).\n";
    print "\# MSS PATH                - Were to put data on MSS. A YYYY/MMDD directory structure will be created under this path\n";
    print "\########################################################################################################################\n";
    print "#\n#\n";

    print "\# ";
    for($i=0;$i<150;$i++) {
	print "=";
    }
    print "\n";

    print "\# ";
    print "TAR FILE NAME\t\t";
    print "LOCAL ARCHIVE DIRECTORY\t\t\t";
    print "FILE_TYPE\t\t";
    print "DATA_PATH (relative to \$RAP_DATA_DIR)\t\t";
    print "MSS PATH\n";

    print "\# ";
    for($i=0;$i<150;$i++) {
	print "=";
    }
    print "\n";

    print "\#\n";
    print "my_data_files\t\t";
    print "/temporary/tarfile/location\t\t";
    print "time\t\t\t";
    print "mdv/my_data_files_path\t\t\t\t";
    print "/RAPDMG/CWX/PROJECT\n";
    exit;
} 

if ($opt_help || ! $opt_params) {
    print $usage;
    exit 0;
}

if($opt_sendMail) {
    $opt_verify = 1;
}

if(!$opt_date) {
    # if date is not provied on command line
    # use date based on system time minus 24 hours

    $time = time() - 86400;
    ($sec, $min, $hr, $day, $month, $year) =  gmtime($time); 
    
    $full_year = $year + 1900;
    $month += 1;

    $date = sprintf("%04d%02d%02d", $full_year, $month, $day);
} else {
    # get date from command line argument
    $full_year = substr($opt_date, 0, 4);
    $month = substr($opt_date, 4, 2);
    $day = substr($opt_date, 6, 2);
    $year = $full_year - 1900;

    $date = $opt_date;
}
$month_day = sprintf("%02d%02d", $month, $day);

if ($opt_debug) {
    print "Archiving data for $date\n";
}

open(IF,"$opt_params") || die "\nCannot find file \"$opt_params\"\n\n";
$count_archive_files = 0;

if ($opt_debug) {
    print "cd $RAP_DATA_DIR\n";
}
chdir("$RAP_DATA_DIR");

my(@lines) = <IF>; # read file into list
@lines = sort(@lines); # sort the list
my($line);
close(IF);
$this_file = 0;

# loop thru list
foreach $line (@lines) {
    @line = split(/\s+/,$line);
    $comment = substr($line[0],0,1);
    if ($comment ne "#" && $line[0] ne "") {
	$tar_file_name = $line[0];
	$local_archive_dir = $line[1];
	$file_type = $line[2];
	$path_to_archive_files = $line[3];
	$path_on_mss = $line[4];
	if($opt_debug){
	    print "\n#############################\n";
	    print "tar_file_name = $tar_file_name\n";
	    print "file_type = $file_type\n";
	    print "#############################\n\n";
	}

	if($path_to_archive_files ne "-" && ! $opt_noTAR) {
	    if($file_type eq "date") {
		$tar_file_name = "${date}_${tar_file_name}";

		if(! $opt_noMSS &&
		   $tar_file_name ne $previous_tar_name &&
		    $previous_tar_name ne ""){
		    $this_file = $count_archive_files - 1;
		    &sendToMSS($archive_file_list[$this_file], $mss_path_list[$this_file], $local_path_list[$this_file], $this_file);
		}

		&mss_path($tar_file_name, $path_on_mss,$local_archive_dir);

		&gzip_files("${path_to_archive_files}/*${date}*");

		$tar_command = "tar \\
                                  -C $RAP_DATA_DIR \\
                                  -r $path_to_archive_files/*${date}* \\
                                  -f $local_archive_dir/$tar_file_name";

		if ($opt_debug) {
		    print "$tar_command\n";
		}

		if(! $opt_test) {
		    system($tar_command);
		    system("chmod 666 ${local_archive_dir}/${tar_file_name}");
		}

	    } elsif($file_type eq "time") {
		$tar_file_name = "${date}_${tar_file_name}";

		if(! $opt_noMSS &&
		   $tar_file_name ne $previous_tar_name &&
		    $previous_tar_name ne ""){
		    $this_file = $count_archive_files - 1;
		    &sendToMSS($archive_file_list[$this_file], $mss_path_list[$this_file], $local_path_list[$this_file], $this_file);
		}

		&mss_path($tar_file_name, $path_on_mss,$local_archive_dir);

		&gzip_files("${path_to_archive_files}/$date/*");

		$tar_command = "tar \\
                                 -C $RAP_DATA_DIR \\
                                 -r ${path_to_archive_files}/$date \\
                                 -f ${local_archive_dir}/${tar_file_name}";

		if ($opt_debug) {
		    print "$tar_command\n";
		}
		if(! $opt_test) {
		    system($tar_command);
		    system("chmod 666 ${local_archive_dir}/${tar_file_name}");
		}

	    } elsif($file_type eq "forecast" ||
		    $file_type eq "fcst") {
		$tar_file_name = "${date}_${tar_file_name}";

		if(! $opt_noMSS &&
		   $tar_file_name ne $previous_tar_name &&
		    $previous_tar_name ne ""){
		    $this_file = $count_archive_files - 1;
		    &sendToMSS($archive_file_list[$this_file], $mss_path_list[$this_file], $local_path_list[$this_file], $this_file);
		}

		&mss_path($tar_file_name, $path_on_mss,$local_archive_dir);

		&gzip_files("${path_to_archive_files}/$date/*/*");

		$tar_command = "tar \\
                                  -C $RAP_DATA_DIR \\
                                  -r ${path_to_archive_files}/$date \\
                                  -f ${local_archive_dir}/${tar_file_name}";

		if ($opt_debug) {
		    print "$tar_command\n";
		}
		if(! $opt_test) {
		    system($tar_command);
		    system("chmod 666 ${local_archive_dir}/${tar_file_name}");
		}

	    } elsif($file_type eq "hourly_time") {
		for ($ii = 0; $ii < 24; $ii++)
		{
		    $hour = sprintf("%02d", $ii);
		    $hourly_tar_file_name = "${date}_${hour}_${tar_file_name}";

		    if(! $opt_noMSS &&
		       $hourly_tar_file_name ne $previous_tar_name &&
		       $previous_tar_name ne ""){
			$this_file = $count_archive_files - 1;
			&sendToMSS($archive_file_list[$this_file], $mss_path_list[$this_file], $local_path_list[$this_file], $this_file);
		    }

		    &mss_path($hourly_tar_file_name, $path_on_mss, $local_archive_dir);

		    &gzip_files("${path_to_archive_files}/$date/$hour*");

		    $tar_command = "tar \\
                                  -C $RAP_DATA_DIR \\
                                  -r ${path_to_archive_files}/${date}/${hour}* \\
                                  -f ${local_archive_dir}/${hourly_tar_file_name}";

		    if ($opt_debug) {
			print "$tar_command\n";
		    }
		    if(! $opt_test) {
			system($tar_command);
			system("chmod 666 ${local_archive_dir}/${hourly_tar_file_name}");
		    }
		}
	    } elsif($file_type eq "hourly_dir") {
		for ($ii = 0; $ii < 24; $ii++)
		{
		    $hour = sprintf("%02d", $ii);
		    $hourly_tar_file_name = "${date}_${hour}_${tar_file_name}";

		    if(! $opt_noMSS &&
		       $hourly_tar_file_name ne $previous_tar_name &&
		       $previous_tar_name ne ""){
			$this_file = $count_archive_files - 1;
			&sendToMSS($archive_file_list[$this_file], $mss_path_list[$this_file], $local_path_list[$this_file], $this_file);
		    }

		    &mss_path($hourly_tar_file_name, $path_on_mss, $local_archive_dir);

		    &gzip_files("${path_to_archive_files}/$date/$hour*/*");

		    $tar_command = "tar \\
                                  -C $RAP_DATA_DIR \\
                                  -r ${path_to_archive_files}/${date}/${hour}* \\
                                  -f ${local_archive_dir}/${hourly_tar_file_name}";

		    if ($opt_debug) {
			print "$tar_command\n";
		    }
		    if(! $opt_test) {
			system($tar_command);
			system("chmod 666 ${local_archive_dir}/${hourly_tar_file_name}");
		    }
		}
	    } elsif($file_type eq "hourly_swp") {
		for ($ii = 0; $ii < 24; $ii++)
		{
		    $hour = sprintf("%02d", $ii);
		    $hourly_tar_file_name = "${date}_${hour}_${tar_file_name}";

		    if(! $opt_noMSS &&
		       $hourly_tar_file_name ne $previous_tar_name &&
		       $previous_tar_name ne ""){
			$this_file = $count_archive_files - 1;
			&sendToMSS($archive_file_list[$this_file], $mss_path_list[$this_file], $local_path_list[$this_file], $this_file);
		    }

		    &mss_path($hourly_tar_file_name, $path_on_mss,$local_archive_dir);

		    &gzip_files("${path_to_archive_files}/$date/*");

		    $tar_command = "tar \\
                                  -C $RAP_DATA_DIR \\
                                  -r ${path_to_archive_files}/${date}/swp.${year}${month_day}${hour}* \\
                                  -f ${local_archive_dir}/${hourly_tar_file_name}";

		    if ($opt_debug) {
			print "$tar_command\n";
		    }
		    if(! $opt_test) {
			system($tar_command);
			system("chmod 666 ${local_archive_dir}/${hourly_tar_file_name}");
		    }
		}
	    } elsif($file_type eq "hourly_ncswp") {
		for ($ii = 0; $ii < 24; $ii++)
		{
		    $hour = sprintf("%02d", $ii);
		    $hourly_tar_file_name = "${date}_${hour}_${tar_file_name}";

		    if(! $opt_noMSS &&
		       $hourly_tar_file_name ne $previous_tar_name &&
		       $previous_tar_name ne ""){
			$this_file = $count_archive_files - 1;
			&sendToMSS($archive_file_list[$this_file], $mss_path_list[$this_file], $local_path_list[$this_file], $this_file);
		    }

		    &mss_path($hourly_tar_file_name, $path_on_mss,$local_archive_dir);

		    &gzip_files("${path_to_archive_files}/$date/*");

		    $tar_command = "tar \\
                                  -C $RAP_DATA_DIR \\
                                  -r ${path_to_archive_files}/${date}/ncswp*${full_year}${month_day}_${hour}* \\
                                  -f ${local_archive_dir}/${hourly_tar_file_name}";

		    if ($opt_debug) {
			print "$tar_command\n";
		    }
		    if(! $opt_test) {
			system($tar_command);
			system("chmod 666 ${local_archive_dir}/${hourly_tar_file_name}");
		    }
		}
	    }else {
		warn "\nFile type \"$file_type\" not valid. Use \"time\", \"date\" or \"fcst\". \"${path_to_archive_files}\" will not be tared to \"${date}_${tar_file_name}\"\n\n";
	    }
	}
    }
}

# send up the last file in the list
if(! $opt_noMSS){
    $this_file = $count_archive_files - 1;
    &sendToMSS($archive_file_list[$this_file], $mss_path_list[$this_file], $local_path_list[$this_file], $this_file);
}

if($opt_verify && ! $opt_test) {
    print "\nPrinting verification list\n\n";
    for($i=0;$i<@archive_file_list;$i++) {
	print "File \"${archive_file_list[$i]}\" sent to mss with size: $sent_size[$i]\n";
	print "File \"${archive_file_list[$i]}\" received on mss with size: $received_size[$i]\n\n";
	if($sent_size[$i] < 500) {
	    warn "WARNING: file \"${archive_file_list[$i]}\" contains only $sent_size[$i]kb of data.\n";
	    $warn[$warn_count] =  "WARNING: file \"${archive_file_list[$i]}\" contains only $sent_size[$i]kb of data.\n" .
		"File sent from ${local_path_list[$i]} to ${mss_path_list[$i]}\n\n";
	    $warn_count++;
	}
	if($sent_size[$i] != $received_size[$i]) {
	    warn "WARNING: file \"${archive_file_list[$i]}\" sent at size $sent_size[$i] and received at $received_size[$i]\n";
	    $warn[$warn_count] =  "WARNING: file \"${archive_file_list[$i]}\" sent at size $sent_size[$i] but received at $received_size[$i]\n" .
		"File sent from ${local_path_list[$i]} to ${mss_path_list[$i]}\n\n";
	    $warn_count++;
	}
    }
}

if($opt_sendMail && $warn_count > 0 && ! $opt_test) {
    if($opt_debug) {
	print "\nSending mail to $opt_sendMail with warnings\n\n";
    }
    open(MAIL,"|$mailprog -t");
    print MAIL "To: $opt_sendMail\n";
    print MAIL "From: daily_backup.pl on $HOST\n";
    print MAIL "Subject: daily backup warnings\n\n";

    print MAIL "Below are warnings from daily_backup.pl:\n\n";
    for($x=0;$x<$warn_count;$x++) {
	print MAIL "$warn[$x]";
    }
    close(MAIL);
}

exit;


## send files to mss
sub sendToMSS {
    $archive_file = $_[0];
    $mss_path = $_[1];
    $local_path = $_[2];
    $file_number = $_[3];

    # construct the msrcp command
    if($opt_project) {
	$msrcp_command = "msrcp -pe 4096 -project $opt_project \\
                         ${local_path}/${archive_file} \\
	                 mss:${mss_path}/${full_year}/${month_day}/${archive_file}";
    }else {
	$msrcp_command = "msrcp -pe 4096 \\
	                  ${local_path}/${archive_file} \\
                          mss:${mss_path}/${full_year}/${month_day}/${archive_file}";
	}

    if ($opt_debug) {
	print "$msrcp_command\n";
    }

    if(! $opt_test) {
	# sending file to mss
	system($msrcp_command);
    }    
    
    if($opt_verify) {
	# if verifying, retain size of file sent and received at MSS
	$sent_size[$file_number] = (-s "${local_path}/${archive_file}");

	$msls_command = "msls -l \\
		         ${mss_path}/${full_year}/${month_day}/${archive_file} > .tmp";
	if($opt_debug) {
	    print "$msls_command\n";
	}

	if(! $opt_test) {
	    system($msls_command);
	    open(IF,".tmp") || warn "cannot open file .tmp\n";
	    while(<IF>) {
		@list = split;
		$received_size[$file_number] = $list[4];
	    }
	    close(IF);
	    unlink(".tmp");
	    if($received_size[$file_number] eq "") {
		$received_size[$file_number] = 0;
	    }
	}
	if($opt_debug){
	    print "file_number = $file_number\n";
	    print "sent_size = $sent_size[$file_number]\n";
	    print "received_size = $received_size[$file_number]\n";
	}
    }
	
    if($opt_debug && $opt_clean_up) {
	print "removing file \"${local_path}/${archive_file}\"\n";
    }
    if($opt_clean_up && ! $opt_test) {
	$rm_command = "/bin/rm ${local_path}/${archive_file}";
	system($rm_command);
    }
}


sub mss_path {
    $archive_file = $_[0];
    $mss_path = $_[1];
    $local_path = $_[2];
    if($archive_file ne "$previous_tar_name") {
	$archive_file_list[$count_archive_files] = $archive_file;
	$mss_path_list[$count_archive_files] = $mss_path;
	$local_path_list[$count_archive_files] = $local_path;
	$previous_tar_name = $archive_file;
	$count_archive_files++;
    }
}

sub gzip_files {
    if ($opt_nogzip)
    {
	return;
    }
    $gzip_path = $_[0];
    @file_list = map { glob($_) } $gzip_path;
    for ($i = 0; $i < @file_list; $i++)
    {
	$filename = ${file_list[$i]};
        if ($filename !~ /.*\.gz/)
	{
	    $gzip_command = "gzip -f $filename";

	    if ($opt_debug)
	    {
		print "$gzip_command\n";
	    }

	    if (! $opt_test)
	    {
		system($gzip_command);
	    }
	}
    }
}
