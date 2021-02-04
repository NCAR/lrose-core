#! /usr/bin/perl
#
# Name: check_build_dirs.pl
#
# Function:
#	Perl script to check the status of builds.
#       Checks the dates on specified files in specified directories
#       and sends email on what is out of date.
#
# Overview:
#       Looks through the specified pairs of (dir,list) for files 
#       for files that are older than a specified age (in days) in the
#       directory (dir) against a known list (list). 
#
# Usage:
#       check_build_dirs.pl -h
#
# Input:
#       1) Pairs of entries:
#             - directory name to check
#             - ASCII file with list of files to check in that directory
#       2) An ASCII file with a list of email addresses
#
# Output:
#       Writes an output file listing the old and missing files in
#       the input directories. If set, this file is mailed to the
#       list of addresses in the input email list file.
#
# Dependencies:
#	Perl must be available in the location at the top of this file.
# 	
# Author: Deirdre Garvey - NCAR/RAP/SDG		28-MAR-2007
#         from a similar Perl script "check_build_status.pl"
#
#---------------------------------------------------------------------------------
#
# --------------------------------- Defaults ------------------------
#

# Required Perl libraries

use Getopt::Long;
use Time::gmtime;
use Time::localtime;

# Set program defaults

$Now=time;
$Host_name=`hostname`;
chop($Host_name);

# Set the OS (if possible)

$Host_os=$ENV{'HOST_OS'};
if ($Host_os !~ /\w/) {
    $Host_os=$ENV{'MS_OS_TYPE'};
}

# Get the program basename.

($prog = $0) =~ s|.*/||;

# like pound-defines

$FileTooOld=0;
$FileNotExist=1;

$FilenameIdx=0;
$FilestatusIdx=1;

$DirIdx=0;
$ListIdx=1;

$ExitSuccess=0;
$ExitFailure=-1;
$ExitWithProblemsFound=1;

# Set default flags for command line args

$Debug=0;
$Debug_level=0;
$Do_mail=0;
$Do_check=1;
$Do_web_build_log_link=0;
$Always_send_mail=0;
$Out_file="check.out";
$Ndays_old=1;
$List="";
$Add_header_text="";
$Do_add_header_text=0;

# Save the usage to print to the user if there is a problem

$usage =                                                 
    "\nUsage: $prog [-dhs] -l <list> [-a <text>] [-m <file>] [-n <ndays>] [-o <file>] [-w <file>]\n" .
    "Purpose: Checks the status of nightly builds. Checks the pairs of --list (dir,list)\n" .
    "         for missing and out-of-date files. Writes the results to the --outfile\n" .
    "         Emails the output file to a list of addresses.\n" .
    "   -a --addheader <text>: Additional text to add to the top of the --outfile\n" .
    "                          The string must be inside quotes.\n" .
    "   -d --debug           : Print debug messages\n" .
    "   -h --help            : Print this usage message\n" .
    "   -l --list <list>     : List of paired entries (dir,list): \n" .
    "                              - directory to check\n" .
    "                              - ASCII file with list of files to check\n" .
    "                          The pairs must be inside parens, separated by a comma.\n" .
    "                          List of pairs must be separated by a comma.\n" .
    "                          The list string must be inside quotes.\n" .
    "                          Use full pathnames, no ~ or env vars.\n" .
    "                          e.g., --list \"(a,b),(c,d),(e,f)\" \n" .
    "   -m --maillist <file> : File containing the list of email addresses\n" .
    "                          to send to. If not specified, no email is sent.\n" .
    "   -n --ndays <n>       : Age in days before a file is considered out of date\n" .
    "                          Default is: $Ndays_old\n" .
    "   -o --outfile <file>  : File containing the results of the check. Is sent as email\n" .
    "                          if --maillist specified.\n" .
    "                          Default is: $Out_file\n" .
    "   -s --sendalways      : Always send email to the addressess in --maillist.\n" .
    "                          Default is to only send mail if problems detected.\n" .
    "   -v --verbose <n>     : Verbose debug level. Also sets --debug\n" .
    "   -w --weblog <file>   : Location on a web server of the nightly checkout\n" .
    "                          and build log so can put this link into the output email.\n" .
    "                          If not specified, no link is put into the email.\n";

# --------------------------------- Parse the command line ------------------------
#
# Get the arguments from the command line

$result = &GetOptions('debug',
		      'help',
		      'addheader=s',
		      'list=s',
		      'maillist=s',
		      'ndays=i',
		      'outfile=s',
		      'sendalways',
		      'verbose=i',
		      'weblog=s',
                       '<>', \&badArg );

if ( $result == 0 || $opt_help ) {
   print $usage;
   exit $ExitFailure;
}

$FoundErrors=0;

if ($opt_debug) {
    $Debug=1;
    $Debug_level=1;
    print(STDERR "Input options specified: \n");
    print(STDERR "\taddheader: $opt_addheader\n");
    print(STDERR "\tlist: $opt_list\n");
    print(STDERR "\tmaillist: $opt_maillist\n");
    print(STDERR "\tndays: $opt_ndays\n");
    print(STDERR "\toutfile: $opt_outfile\n");
    print(STDERR "\talways send email: $opt_sendalways\n");
    print(STDERR "\tverbose debug: $opt_verbose\n");
    print(STDERR "\tweblog: $opt_weblog\n");
}
if ($opt_addheader) {
    $Do_add_header_text=1;
    $Add_header_text=$opt_addheader;
}

if ($opt_list) {
    $List=$opt_list;
} else {
    print(STDERR "ERROR: No list provided to check\n");
    $FoundErrors=1;
}

if ($opt_maillist) {
    $Do_mail=1;
    $Mail_list=$opt_maillist;
}

if ($opt_ndays) {
    if ($opt_ndays > 0) {
	$Ndays_old=$opt_ndays;
    } else {
	print(STDERR "ERROR: Invalid --ndays. Must be >0\n");
	$FoundErrors=1;
    }
}

if ($opt_outfile) {
    $Out_file=$opt_outfile;
}

if ($opt_sendalways) {
    $Always_send_email=1;
}

if ($opt_verbose) {
    $Debug=1;
    if ($opt_verbose < 0) {
	$Debug_level=1;
    } else {
	$Debug_level=$opt_verbose;
    }
}

if ($opt_weblog) {
    $Do_web_build_log_link=1;
    $Web_build_log_link=$opt_weblog;
}

if ($FoundErrors) {
    exit $ExitFailure;
}

# Error checking on input params

$FoundErrors=0;

if (($Do_mail) && (!-e $Mail_list)) {
    print(STDERR "ERROR: Mail list does not exist: $Mail_list\n");
    $FoundErrors=1;
}

if ($FoundErrors) {
    exit $ExitFailure;
}
 
#------------------------------------ Main ------------------------------
#
# Convert the number of days to seconds

$Nsecs_old = $Ndays_old * (24 * 60 * 60);

# Split the list into pairs

($is_ok, $nlist)=getArray($List, *listarr, $Debug_level);
if (!$is_ok) {
    exit $ExitFailure;
}

# Open the output file

if (!open(OUTFILE, ">$Out_file")) {
    ($is_ok, $dir)=createMissingDirForOutfile($Out_file, $Debug_level);
    if (!$is_ok) {
	print(STDERR "ERROR: Unable to open file $Out_file\n");
	exit $ExitFailure;
    } else {
	if (!open(OUTFILE, ">$Out_file")) {
	    print(STDERR "ERROR: Unable to open file $Out_file\n");
	}
    }
}

# Get the date

$Date=&ctime(time);
($sec,$min,$hour,$mday,$mon,$year,$wday,$yday,$isdst)=localtime(time);

# Print the additional text

if ($Do_add_header_text) {
    print(OUTFILE $Add_header_text);
    print(OUTFILE "\n");
}

# Get the current machine name and host-os

print (OUTFILE "Current machine: $Host_name\n");
print (OUTFILE "HOST_OS: $Host_os\n");
print (OUTFILE "Date: $Date\n");

# Set defaults

$FoundProblems=0;

# Loop through the array

for ($idx=0; $idx<$nlist; $idx++) {
    $dir=$listarr[$idx][$DirIdx];
    $list=$listarr[$idx][$ListIdx];

    if (!-d $dir) {
	print(STDERR "ERROR: $prog: dir does not exist: $dir\n");
	next;
    }

    if (!-e $list) {
	print(STDERR "ERROR: $prog: list does not exist: $list\n");
	next;
    }

    ($is_ok, $nwanted)=&get_expected_list($list, *wanted_arr, $Debug_level);
    if ((!$is_ok) || ($nwanted <= 0)) {
	print(STDERR "ERROR: No items found in $list\n");
	next;
    }

    # Do the check

    ($is_ok, $nfound_arr)=&check_files($dir, $Nsecs_old, $Now, $Do_check, *wanted_arr, $nwanted, *found_info_arr, $Debug_level);
    
    # Found any problems?

    if ($nfound_arr > 0) {
	$FoundProblems=1;
    }

    # Print the details

    $is_ok=&print_results_arr(OUTFILE, $dir, $Ndays_old, *found_info_arr, $nfound_arr, $Debug_level);
}

# Add link to log location into output file

if ($Do_web_build_log_link) {
    print (OUTFILE "\n\nTo see the build log goto $Web_build_log_link\n");
}

# Close the output file

close (OUTFILE);

# Mail the output file

if ($Do_mail) {
    
    # Do we even want to mail the output file?

    if ((!$FoundProblems) && (!$Always_send_email)) {
	if ($Debug) {
	    print(STDERR "Will not send email, no problems found\n");
	}

    } else {

	# Get the list of email addresses from the mail file
    
	($is_ok, $n_mailnames)=&get_mail_list($Mail_list, *arr_mailnames, $Debug_level);

	# Set the subject

	$subject="Build results for host: $Host_name, OS: $Host_os";
	if ($FoundProblems) {
	    $subject="$subject -- Problems found";
	} else {
	    $subject="$subject -- No problems found";
	}

	foreach $mailname (@arr_mailnames) {
	    if ($Debug) {
		print(STDERR "Sending email to: $mailname\n");
	    }
	    system("mail -s \"$subject\" $mailname < $Out_file");
	} #endfor
    } #endelse
} #endif

# Done

if ($FoundProblems) {
    exit $ExitWithProblemsFound;
} else {
    exit $ExitSuccess;
}


# ----------------------------- Subroutines ------------------------------
#
# Subroutine: getArray
#
# Usage: ($return_val, $noutarr)=&getArray($inlist, *outarr, $debug)
#
# Function: Parse the input $inlist and return pairs in *outarr
#
# Input:  $inlist           list to parse
#         $debug            flag (1=on, 0=off) for debug info
#
# Output: $return_val       1 on success, 0 otherwise
#         $noutarr          size of *outarr
#         *outarr           array of pairs of dir,list to check
#           $outarr[0][$DirIdx] = directory
#           $outarr[0][$ListIdx] = list paired with directory
#           $outarr[n][$DirIdx] = directory
#           $outarr[n][$ListIdx] = list paired with directory
#
#

sub getArray {
    local($inlist, *outarr, $debug)  = @_;

    # Define local variables

    local($return_val, $subname);
    local($dbg2, $dbg3);
    local($noutarr, $counter, @first_split, @second_split, $pair, $item, $junk);
    local($sub_counter, $j);

    # Set defaults

    $return_val = 0;
    $subname = "getArray";
    $noutarr=0;
    $dbg2=0;
    $dbg3=0;

    # Debug

    if ($debug == 2) {
	$dbg2=1;
    }
    if ($debug == 3) {
	$dbg3=1;
    }

    if ($debug) {
	print(STDERR "$subname: Input...\n");
	print(STDERR "\tinlist: $inlist\n");
    }

    @first_split=split('\)', $inlist);

    $counter=0;
    foreach $pair (@first_split) {
	if ($dbg2) {
	    print(STDERR "$subname: list pair: $pair\n");
	}

	@second_split=split('\,', $pair);

	if (length($pair) <= 1) {
	    next;
	}

	$sub_counter=0;
	foreach $item (@second_split) {
	    if ($item !~ /\w/) {
		next;
	    }
	    if ($item =~ /\(/) {
		($junk, $item)=split('\(', $item);
	    }
	    if ($debug > 1) {
		print(STDERR "\titem: $item\n");
	    }
	    $outarr[$counter][$sub_counter]=$item;
	    $sub_counter++;
	}
	$counter++;
    }

    $noutarr=$counter;
    if ($counter > 0) {
	$return_val=1;
    }

    # Debug

    if ($dbg2) {
	print(STDERR "$subname: noutarr: $noutarr\n");
	for ($j=0; $j<$noutarr; $j++) {
	    print(STDERR "\tj: $j, dir: $outarr[$j][$DirIdx], list: $outarr[$j][$ListIdx]\n");
	}
    }

    return($return_val, $noutarr);
}

#--------------------------------------------------------------------------------------
#
# Subroutine: check_files
#
# Usage: ($return_val, $noutarr)=&check_files($check_dir, $nsecs, $now, $do_check, 
#                                             *wanted_arr, $nwanted, *outarr, $debug)
#
# Function: Check the input $check_dir for any files older than $nsecs.
#           If $do_check is set, also check the $check_dir against the
#           *wanted_arr.
#
# Input:  $check_dir        directory to check
#         $nsecs            number of secs before file is too old
#         $now              current unix time
#         $do_check         flag to check against the *wanted_arr or not
#         *wanted_arr       array of wanted files
#         $nwanted          size of $wanted_arr
#         $debug            flag (1=on, 0=off) for debug info
#
# Output: $return_val       1 on success, 0 otherwise
#         $noutarr          size of *outarr
#         *outarr           array of files found with problems
#           $outarr[0][0] = filename
#           $outarr[0][1] = problem status
#           $outarr[n][0] = filename
#           $outarr[n][1] = problem status
#
#

sub check_files {
    local($check_dir, $nsecs, $now, $do_check, *wanted_arr, $nwanted, *outarr, $debug)  = @_;

    # Define local variables

    local($return_val, $subname);
    local($dbg2, $dbg3);
    local($is_ok, $noutarr, $is_old, $dirEntry, $i, $full_fname, $counter);

    # Set defaults

    $return_val = 0;
    $subname = "check_files";
    $noutarr=0;
    $dbg2=0;

    # Debug

    if ($debug == 2) {
	$dbg2=1;
    }
    if ($debug == 3) {
	$dbg3=1;
    }

    if ($debug) {
	print(STDERR "$subname: Input...\n");
	print(STDERR "\tcheck_dir: $check_dir, nsecs: nsecs, now: $now, do_check: $do_check\n");
	print(STDERR "\tnwanted: $nwanted\n");
    }

    # Open the input check dir

    $is_ok=opendir(CHECKDIR, $check_dir);
    if (!$is_ok) {
	print(STDERR "ERROR: Cannot open $check_dir for reading\n");
	return($return_val, $noutarr);
    }

    # Read through the files in the $check_dir

    $counter=0;

    while( $dirEntry = readdir( CHECKDIR ) ) {

	# Skip CVS and . files

	$dirEntry =~ m/^\.{1,2}$/ and next;
	$dirEntry =~ m/^CVS$/ and next;
	
	# Skip subdirectories

	if (-d $dirEntry) {
	    if ($debug) {
		print(STDERR "$subname: Skipping $dirEntry, is a directory\n");
	    }
	    next;
	}

	# Is this file older than $nsecs? Skip to the next one if not

	($is_ok, $is_old)=&check_file_too_old("$check_dir/$dirEntry", $nsecs, $now, $debug);
	if (!$is_old) {

	    if ($dbg3) {
		print(STDERR "$subname: $dirEntry is okay, not too old\n");
	    }

	    next;
	}

	# Is this a file we want?

	if ($do_check) {
	    ($is_ok, $is_wanted)=&is_wanted($dirEntry, *wanted_arr, $nwanted, $debug);
	    if ((!$is_ok) || (!$is_wanted)) {

		if ($dbg2) {
		    print(STDERR "$subname: Skipping $dirEntry, is not wanted\n");
		}

		next;
	    }
	}

	# Add to the output array

	if ($dbg2) {
	    print(STDERR "$subname: $dirEntry added to output array as too old, counter: $counter\n");
	}

	$outarr[$counter][$FilenameIdx]=$dirEntry;
	$outarr[$counter][$FilestatusIdx]=$FileTooOld;
	$counter++;
    }

    # Close the directory

    close(CHECKDIR);

    # Check that all the files even exist

    if ($do_check) {
	for ($i=0; $i<$nwanted; $i++) {
	    $full_fname=$check_dir . "/" . $wanted_arr[$i];
	    if (!-e $full_fname) {
		if ($dbg2) {
		    print(STDERR "$subname: $full_fname does not exist in $check_dir\n");
		}
		
		$outarr[$counter][$FilenameIdx]=$wanted_arr[$i];
		$outarr[$counter][$FilestatusIdx]=$FileNotExist;
		$counter++;
	    }
	}
    }

    # Debug

    if ($debug) {
	print(STDERR "$subname: counter: $counter\n");
    }

    # Done
    
    $return_val=1;
    $noutarr=$counter;
    return($return_val, $noutarr);

}

#------------------------------------------------------------------------------
#
# Subroutine: get_expected_list()
#
# Usage: ($return_val, $nexpected)=get_expected_list($file, *expected, $debug);
#
# Function:   Reads the input $file to construct an array of expected
#             filenames.
#
# Input:      $file            filename with the list of files
#             $debug           flag
#
# Output:     $return_val      1 on success, 0 otherwise
#             *expected        array of files in $file
#             $nexpected        size of *expected
#

sub get_expected_list {
    local($file, *expected, $debug) = @_;

    # Local variables

    local($return_val, $subname);
    local($dbg2, $nexpected);
    local($is_ok, $counter, $line);
    local($name, $descrip, $justname, $junk);

    # Set defaults
    
    $return_val=0;
    $nexpected=0;
    $subname="get_expected_list";
    $dbg2=0;

    # Debug

    if ($debug == 2) {
	$dbg2=1;
    }

    if ($dbg2) {
	print(STDERR "$subname: Input...\n");
	print(STDERR "\tfile: $file\n");
    }

    # Initialize

    $counter=0;

    # Open file with list of files to check

    $is_ok=open(LISTFILE, $file);
    if (!$is_ok) {
	print(STDERR "Cannot open file: $file\n");
	return($return_val, $nexpected);
    }

    # Read the file, skip lines with leading # (comments) or blank lines

    while ($line = <LISTFILE>) {

	if (($line !~ /^\#/) && ($line =~ /\w/)) {
	     
	    # No description field so just add the lib to the array

	    if ($line !~ /\|/) {
		chomp($line);                  # Remove the trailing newline
                $line =~ s/\s+$//;             # Remove trailing white spaces
		$expected[$counter]=$line;
		$counter++;
	    }

	    # There is a description field, now need to either
	    # chop it off or if a multi-line description with
	    # no library name on the line, just skip over it
	     
	    else {
		($name, $descrip) = split(/\|/, $line);
		if ($name =~ /\w/) {
		    ($justname, $junk)=split(' ', $name);
		    $expected[$counter]=$justname;
		    $counter++;
		}
	    }
	}
    } #endwhile

    # Close file
	
    close(LISTFILE);

    $nexpected=$counter;

    # Debug

    if ($dbg2) {
	print(STDERR "$subname: counter: $counter\n");
	for ($i=0; $i<$counter; $i++) {
	    print(STDERR "\ti:$i, expected: $expected[$i]...\n");
	}
    }

    # Done
	
    $return_val=1;
    return($return_val, $nexpected);
}

# ----------------------------------------------------------------------
#
# Subroutine: check_file_too_old
#
# Usage: ($return_val, $is_old)=&check_file_too_old($file, $nsecs, $now, $debug)
#
# Function: Check the input $file to see if it is older than $nsecs.
#           Return status in $is_old.
#
# Input:  $file             file to check
#         $nsecs            number of secs before file is too old
#         $now              current unix time
#         $debug            flag (1=on, 0=off) for debug info
#
# Output: $return_val       1 on success, 0 otherwise
#         $is_old           1 if file older than $ndays, 0 otherwise
#
#

sub check_file_too_old {
    local($file, $nsecs, $now, $debug)  = @_;

    # Define local variables

    local($return_val, $subname);
    local($dbg2, $is_old);
    local($dev,$ino,$mode,$nlink,$uid,$gid,$rdev,$size,$atime,$mtime,$ctime,$blksize,$blocks);
    local($file_age_in_secs);

    # Set defaults

    $return_val = 0;
    $subname = "check_file_too_old";
    $is_old=0;
    $dbg2=0;

    # Debug

    if ($debug == 2) {
	$dbg2=1;
    }

    # Get the age of the file

    ($dev,$ino,$mode,$nlink,$uid,$gid,$rdev,$size,$atime,$mtime,$ctime,$blksize,$blocks)= stat($file);

    # Compare with now

    $file_age_in_secs=$now - $mtime;
    
    if ($file_age_in_secs >= $nsecs) {
	$is_old=1;
    }

    if ($file_age_in_secs < 0) {
	$return_val=0;
    } else {
	$return_val=1;
    }

    # Debug

    if ($dbg2) {
	print(STDERR "$subname: file: $file, is_old: $is_old, file_age_in_secs: $file_age_in_secs, mtime: $mtime, now: $now\n");
    }
    
    # Done

    return($return_val, $is_old);
}
    

#-------------------------------------------------------------------------------------------------------
# Subroutine: is_wanted()
#
# Usage: ($return_val, $is_wanted) = is_wanted($filename, *wanted_arr, $narr, $debug)
#
# Function:   To check that the input $filename is contained in the
#             *wanted_arr of filenames.
#
# Input:      $filename               filename to check
#             *wanted_arr             array of wanted files
#             $narr                   size of *wanted_arr
#             $debug                  flag
#
# Output:     $return_val             1 if succes, 0 on error
#             $is_wanted              1 if it is a file we care about. Return 0 if not.
#

sub is_wanted {
    local ($filename, *wanted_arr, $narr, $debug) = @_;

    # Define local variables

    local($return_val, $subname);
    local($dbg2, $is_wanted);

    # Set defaults

    $return_val = 0;
    $subname = "is_wanted";
    $dbg2=0;

    # Debug

    if ($debug == 2) {
	$dbg2=1;
    }

    # Check

    $is_wanted=0;
    for ($i=0; $i<$narr; $i++) {
        if ($filename eq $wanted_arr[$i]) { 
	    $is_wanted=1;
        }
    }
    
    # Done

    $return_val=1;
    return($return_val, $is_wanted);
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

#-------------------------------------------------------------------------------
#
# Subroutine: print_results_arr()
#
# Usage: $return_val=&print_results_arr($fh, $dir, $ndays, *printarr, $nprintarr, $debug);
#
# Function:   To print the contents of *arr to $fh
#
# Input:      $fh               filehandle for output file, must be open
#             $dir              directory to print
#             $ndays            number of days to print
#             *printarr         array to print
#                  $printarr[0][0] = filename
#                  $printarr[0][1] = problem status
#                  $printarr[n][0] = filename
#                  $printarr[n][1] = problem status
#             $nprintarr        size of *printarr
#             $debug            flag
#
# Output:     $return_val       1 on success, 0 on error
# 

sub print_results_arr {
    local($fh, $dir, $ndays, *printarr, $nprintarr, $debug) = @_;

    # Local variables

    local($return_val, $subname);
    local($dbg2, $i, $fname, $status, $status_str, $str);

    # Set defaults
    
    $return_val=0;
    $subname="print_results_arr";
    $dbg2=0;

    # Debug

    if ($debug == 2) {
	$dbg2=1;
    }

    if ($dbg2) {
	print(STDERR "$subname: Input: dir: $dir, nprintarr: $nprintarr\n");
    }

    # Print the directory

    print($fh "... Checking files in directory: $dir ...\n");

    # Exit if no problems found

    if ($nprintarr == 0) {
	print($fh "\n");
	print($fh "\tNo problems found\n");
	$return_val = 1;
	return($return_val);
    }
    
    # Print header
    
    print($fh "The following files did not rebuild successfully:\n");
    
    # Print

    for ($i=0; $i<$nprintarr; $i++) {

	$fname=$printarr[$i][$FilenameIdx];
	$status=$printarr[$i][$FilestatusIdx];

	if ($status == $FileTooOld) {
	    $status_str=" is more than $ndays days old";
	} elsif ($status == $FileNotExist) {
	    $status_str=" does not exist";
	} else {
	    $status_str="";
	}

	$str="$fname $status_str";

	if ($dbg2) {
	    print(STDERR "i: $i, $str\n");
	}
	print($fh "\t\t$str\n");
    }

    print($fh "\n");

    # Done

    $return_val=1;
    return($return_val);
}

#-------------------------------------------------------------------------
#
# Subroutine createMissingDirForOutfile
#
# Usage: ($return_val, $dir) = createMissingDirForOutfile($file, $dbg)
#
# Function: Check if the $file has a directory in it, if so,
#           create the directory portion of the filename
#
# Input:    $file           output filename to check
#
# Output:   $return_val     1 on success, 0 on error
#           $dir            directory portion of filename
#
# Overview:
#

sub createMissingDirForOutfile
{
  my ($file, $dbg) = @_;

  # Local variables

  my($subname, $return_val);
  my($dbg2, $dbg3);
  my($is_ok, $dir, $len, $last_slash);

  # Set defaults

  $return_val=0;
  $dir="NOTSET";
  $subname="createMissingDirForOutfile";

  # Dbg

  $dbg2=0;
  $dbg3=0;

  if ($dbg == 2) {
      $dbg2=1;
  }

  if ($dbg == 3) {
      $dbg3=1;
      $dbg2=1;
  }

  # If the output filename exists, return. No need to create a directory

  if (-e $file) {
      if ($dbg) {
	  print(STDERR "$subname: file exists, no directory missing from name: $file\n");
      }
      $return_val=1;
      return($return_val, $dir);
  }

  # Is there a directory in the filename that does not
  # exist? Attempt to create the directory.

  $last_slash=rindex($file, "/");
  $len=length($file);

  if ($dbg3) {
      print(STDERR "$subname: file: $file, last_slash: $last_slash, len: $len\n");
  }

  if ($last_slash < $len) {
      $dir=substr($file, 0, $last_slash);
      if (-d $dir) {
	  print(STDERR "$subname: Directory exists: $dir\n");
	  print(STDERR "$subname: Must be unable to create output file for some other reason, $file\n");
      } else {
	  if ($dbg) {
	      print(STDERR "$subname: Attempt to create dir, $dir\n");
	  }
	  system("mkdir -p $dir");
	  if (-d $dir) {
	      $return_val=1;
	  } else {
	      print(STDERR "ERROR: $subname: Cannot create $dir\n");
	  }
      }
  } else {
      print(STDERR "$subname: No directory in the filename, $file\n");
      print(STDERR "$subname: Must be unable to create output file for some other reason, $file\n");
      $return_val=1;
  }
    
  if ($dbg2) {
      print(STDERR "$subname: return_val: $return_val, dir: $dir\n");
  }

  return($return_val, $dir);
}
  

#============================================= EOF ==================================
