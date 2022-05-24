#!/usr/bin/perl
#
# Function:
#       Perl script to create the HTML front page for RAP apps and libs 
#       including links to doxygen-generated pages. Also runs doxygen to generate
#       needed pages. Expected to be run from crontab on a regular
#       (nightly?) basis.
#	
# Usage:
#	see -h option.
#
# Input: (1) application or library source code
#
# Output:
#       Creates an output HTML file and runs doxygen to generate
#
# Dependencies:
#       (1) Must have access to a CVS checked out area of the apps or libs
#           to build a page for and run doxygen against
#
# Author: Deirdre Garvey	29-DEC-2003
#
#---------------------------------------------------------------------------

#       Externals

use Getopt::Long;
use Env;
use Cwd;
Env::import();
require "find.pl";

sub badArg;
sub translateDataDir;
sub verifyDataDir;

# Set program defaults
#  prog: program basename
#  Host: current hostname
#  Today: current date

($prog = $0) =~ s|.*/||;
$Host=$ENV{'HOST'};
$Today=`date -u`;
chop($Today);

# Hardwired defaults - could be moved to config file
#  Makefile_appname_string: String to search for in apps Makefile to determine
#                          the actual name of the application
#  Makefile_ldlibs_string: String to search for in apps Makefile to determine
#                          the load libraries for the application
#  Makefile_libname_string: String to search for in libs Makefile to determine
#                           the actual name of the library

$Makefile_appname_string="TARGET_FILE";
$Makefile_ldlibs_string="LOC_LIB";
$Makefile_libname_string="MODULE_NAME";

# Like pound-defines
#  The *Idx are for array indices

$Exit_success=1;
$Exit_failure=0;
$StringDefault="Unknown";
$NameIdx=0;
$DoxygenUrlIdx=1;
$CvsDirIdx=2;
$LibDepIdx=3;
$CvsDocUrlIdx=4;

#
# Set command line defaults
#

$Debug=0;                           # Flag to print debug messages to STDERR
$Debug_level=0;                     # Level of debugging
$DoPrintParams=0;                   # Flag for print_params
$Test=0;                            # Flag for test mode

# Save the usage to print to the user if there is a problem

$usage =                                                 
    "\nUsage: $prog -p|-c <file> [-dht] <-v level>\n" .
    "Purpose: Create an HTML output file with a sorted list of RAP apps or libs\n" .
    "         and links to URLs with descriptions and doxygen-generated docs\n" .
    "         Requires a checked-out CVS reference area to search.\n" .
    "   -c --config <file>    : (Required.) The name of the config/param\n" .
    "                           file to read. NOT required if running\n" .
    "                           --print_params\n" .
    "   -d --debug            : Print debugging messages\n" .
    "   -h --help             : Print this usage message\n" .
    "   -p --print_params     : Generate an input config/param file for\n" .
    "                           this script. Writes to STDOUT.\n" .
    "   -t --test             : Test mode\n" .
    "   -v --verbose <num>    : A debug level number >0\n" .
    "                           Will also turn on --debug\n";


# ----------------------------- Parse command line --------------------------
#
# Get the arguments from the command line
#
$result = &GetOptions('config=s',
		      'debug',
		      'help',
		      'print_params',
		      'test',
		      'verbose=i',
                       '<>', \&badArg );

if ( $result == 0 || $opt_help ) {
   print $usage;
   exit $ExitFailure;
}

if ((!$opt_config) && (!$opt_print_params)) {
    print(STDERR "ERROR: You must specify a config/param file!\n");
    print $usage;
    exit $ExitFailure;
}

if (($opt_debug) || ($opt_level)) {
  $Debug=1;
  $Debug_level=1;
  print(STDERR "Input options specified...\n");
  print(STDERR "\tconfig: $opt_config\n");
  print(STDERR "\ttest: $opt_test\n");
  print(STDERR "\tverbosedebuglevel: $opt_verbose\n");
}

if ($opt_config) {
    $ConfigFile= $opt_config;
}

if ($opt_print_params) {
    $DoPrintParams=1;
}

if ($opt_test) {
    $Test=1;
}

if ($opt_verbose) {
  if ($opt_verbose < 0) {
    print(STDERR "ERROR: $prog: Invalid debug level. Ignoring it\n");
    $Debug=1;
    $Debug_level=1;
  } else {
    $Debug_level=$opt_verbose;
    $Debug=1;
  }
}

# ------------------------- Initialization -----------------------
#
$startProcessingTime = time();

# --------------------------- Config/Params -------------------------
#
# Are we doing print_params? If so, generate the file and exit
#
if ($DoPrintParams) {
    $is_ok = &generateParamFile;
    exit ($is_ok);
}


# Read the config/param file and set globals

$is_ok = &readParamFile($ConfigFile, $Debug_level);
if (!$is_ok) {
    print(STDERR "Exiting...\n");
    $ExitFailure;
}


# ------------------------- Error checking -----------------------
#
if (!-d $Src_dir) {
    print(STDERR "$prog: ERROR: Input directory does not exist: $Src_dir\n");
    exit $Exit_failure;
}

if (!-d $Tmp_dir) {
    $cmd="mkdir -p $Tmp_dir";
    system($cmd);
}

if ($Do_doxygen) {

    # Does the config file exist?

    if (!-f $Doxygen_config_file) {
	print(STDERR "$prog: ERROR: Doxygen config file does not exist: $Doxygen_config_file\n");
	exit $Exit_failure;
    }

    # Does doxygen binary exist?

    foreach $exe (@Doxygen_exes_arr) {
	$check=`which $exe`;
	if (!$check) {
	    print(STDERR "$prog: ERROR: executable does not exist: $exe\n");
	    exit $Exit_failure;
	}
    }
}

if ($Doxygen_out_dir ne $StringDefault) {
    if (!-d $Doxygen_out_dir) {
	$cmd="mkdir -p $Doxygen_out_dir";
	system($cmd);
    }
}

if ($Doxygen_tagfile_dir ne $StringDefault) {
    if (!-d $Doxygen_tagfile_dir) {
	$cmd="mkdir -p $Doxygen_tagfile_dir";
	system($cmd);
    }
}

# -------------------------------- MAIN --------------------------
# Run find to find the apps-level directories and Makefiles
# Need to use a global-level array since cannot pass additional options
# to find().

*AppsArr;
$AppsCounter=0;
&find($Src_dir);

# Sort the apps by name -- not yet implemented

*SortedAppsArr;
$is_ok = &doSort(*AppsArr, $AppsCounter, *SortedAppsArr, $Debug_level);

# Open the output file

if (!open(OUTFILE, "> $Output_fname")) {
    print(STDERR "ERROR: Cannot open output file $Output_fname... \n");
    exit $Exit_failure;
}

# Write the HTML stuff to the top of the output file

$is_ok = &WriteHTMLHeader($Output_fname, $Do_doxygen, $Debug_level);

# Write the array of apps/libs to the output file

$is_ok = &WriteHTMLTable($Output_fname, *SortedAppsArr, $AppsCounter, $Debug_level);
# Close the HTML table

$is_ok = &WriteHTMLFooter($Output_fname, $Cvs_doc_src_dir, $Distribs_file, $Author, $Author_email, $Host, $Debug_level);

# Print elapsed time

$endProcessingTime = time();
&printElapsedTime($startProcessingTime, $endProcessingTime);

# Done

exit $Exit_success;


#----------------------------- SUBROUTINES -----------------------------
#
# This is the subroutine called by find()
#
# The AppsArr and AppsCounter are global

sub wanted {
    (($dev,$ino,$mode,$nlink,$uid,$gid) = lstat($_))
	|| warn "stat: $name: $!\n";

    # Local variables
    
    local($subname);
    local($appname, $is_ok, $cvs_doc_file_url, $list, $doxygen_url);
    local($file, $found, $cvs_dir, $class_name, $app_dir, $filename);
    local($dbg2, $dbg3);

    # Debugging

    $dbg2=0;
    $dbg3=0;

    if ($Debug_level == 2) {
	$dbg2=1;
    }
    if ($Debug_level == 3) {
	$dbg3=1;
    }

    # Set defaults

    $subname="wanted";
    $doxygen_url=$StringDefault;
    $cvs_doc_file_url=$StringDefault;

    # Debugging

    if ($dbg3) {
	print(STDERR "$subname: name: $name\n");
    }

    # Return if a directory

    if (-d $name) {
	return;
    }

    # Get just the filename from the $name

    $last_slash=rindex($name, "/");
    $filename=substr($name, $last_slash+1);

    if ($dbg3) {
	print(STDERR "$subname: filename: $filename\n");
    }
    
    # Return if filename is not a Makefile

    $found=0;
    foreach $file (@Makefile_names_arr) {
	if ($filename eq $file) {
	    $found=1;
	}
    }
    if (!$found) {
	return;
    }

    if ($dbg2) {
	print(STDERR "----- $subname: Found a wanted file: $name\n");
    }

    # Get the name of the app from its Makefile. If cannot get the name
    # then do not want this app. This is the case with Makefiles above the
    # single app level.

    ($is_ok, $appname) = &GetAppOrLibNameFromMakefile($name, $Debug_level);
    if (!$is_ok) {
	return;
    }

    # Get the CVS source directory and the class name
    
    ($is_ok, $app_dir, $cvs_dir, $class_name) = &GetCvsSrcDir($name, $Src_dir, $Debug_level);

    # Get the libraries this is dependent on

    if ($DoApps) {
	($is_ok, $list) = &GetAppLibDepend($name, $Debug_level);
    }

    # Get the Doxygen URL or run Doxygen

    if ($Do_doxygen) {
	($is_ok, $doxygen_url) = &RunDoxygen($appname, $class_name, $list, $Doxygen_config_file, $Tmp_dir, $Doxygen_tagfile_dir, $Doxygen_libs_dir, $Doxygen_out_dir, $Www_doxygen_out_dir, $app_dir, $Debug_level);
    } else {
	($is_ok, $doxygen_url) = &GetDoxygenUrl($appname, $class_name, $Doxygen_out_dir, $Www_doxygen_out_dir, $Debug_level);
    }

    # Get the doc file URL for doc in CVS, not the Doxygen doc

    ($is_ok, $cvs_doc_file_url) = &GetCvsDocUrl($appname, $class_name, $Cvs_doc_src_dir, $Www_cvs_doc_src_dir, $Debug_level);

    # Add to the array of apps

    $AppsArr[$AppsCounter][$NameIdx]=$appname;
    $AppsArr[$AppsCounter][$DoxygenUrlIdx]=$doxygen_url;
    $AppsArr[$AppsCounter][$CvsDirIdx]=$cvs_dir;
    $AppsArr[$AppsCounter][$LibDepIdx]=$list;
    $AppsArr[$AppsCounter][$CvsDocUrlIdx]=$cvs_doc_file_url;
    $AppsCounter++;
}

#--------------------------------------------------------------------#
# Subroutine: GetAppOrLibNameFromMakefile
#
# Usage: ($return_val, $appname) = GetAppOrLibNameFromMakefile($makefile, $dbg)
#
# Function: Read the input Makefile to extract the application or library name.
#           Note that this is hardwired to look for a string 
#           $Makefile_appname_string or $Makefile_libname_string in the $makefile.
#
# Input:    makefile       name of Makefile to read
#           dbg            debug flag
# 
# Output:   return_val     1 if success, 0 if failure
#           appname        name of application or library
#
# Overview:
# 
#

sub GetAppOrLibNameFromMakefile {
    local($makefile, $dbg)  = @_;

    # Define local variables

    local($return_val, $subname, $appname);
    local($key, $name, $found, $dbg2, $dbg3, $is_ok, $wantstring);

    # Debugging

    $dbg2=0;
    $dbg3=0;
    if ($dbg == 2) {
	$dbg2=1;
    }
    if ($dbg == 3) {
	$dbg2=1;
	$dbg3=1;
    }

    # Set defaults

    $return_val = 0;
    $subname = "GetAppOrLibNameFromMakefile";
    $appname = $StringDefault;
    $found = 0;

    # Set string to search for

    if ($DoApps) {
	$wantstring=$Makefile_appname_string;
    } else {
	$wantstring=$Makefile_libname_string;
    }

    # Open and read the Makefile

    if (!open(MKFILE, $makefile)) {
	print(STDERR "ERROR: $subname: Cannot open input file $makefile\n");
	return($return_val, $appname);
    }
    
    while ($line = <MKFILE>) {

	if ($line =~ /^$wantstring/) {
	    ($key, $name) = split(/=/, $line);
	    $found = 1;

	    # Remove trailing newline and any leading spaces

	    ($is_ok, $appname) = &ChopName($name, $dbg);

	    # Are there any characters?

	    if ($appname !~ /\w+/) {
		$found=0;
	    }

	    if ($dbg2) {
		print (STDERR "$subname: Found $wantstring: found: $found, name: $appname, line: $line");
	    }
	} #endif
    } # endwhile

    close(MKFILE);

    if (!$found) {
	print(STDERR "INFO: $subname: did not find $wantstring in $makefile\n");
    }

    if (($found) && ($dbg > 0)) {
	print(STDERR "$subname: App/Lib name for $makefile is $appname\n");
    }

    # Done

    $return_val = $found;
    return($return_val, $appname);
}


#--------------------------------------------------------------------#
# Subroutine: GetCvsSrcDir
#
# Usage: ($return_val, $app_dir, $cvs_dir, $class_name) = 
#                     GetCvsSrcDir($fname, $cvs_co_dir, $dbg)
#
# Function: Get the CVS source code directory and class name for this app
#
# Input:    fname          full filename
#           cvs_co_dir     top-level CVS reference directory
#           dbg            debug flag
# 
# Output:   return_val     1 if success, 0 if failure
#           app_dir        current full app directory
#           cvs_dir        CVS source directory
#           class_name     app class name
#
# Overview:
# 
#

sub GetCvsSrcDir {
    local($fname, $cvs_co_dir, $dbg)  = @_;

    # Define local variables

    local($return_val, $subname);
    local($dbg2, $dbg3);
    local($cvs_dir, $class_name, $app_dir, $len, $last_slash, *dirs);
    local($found, $found_first, $dir, $apps_pos);

    # Debugging

    $dbg2=0;
    $dbg3=0;
    if ($dbg == 2) {
	$dbg2=1;
    }
    if ($dbg == 3) {
	$dbg2=1;
	$dbg3=1;
    }

    # Set defaults

    $return_val = 0;
    $subname = "GetCvsSrcDir";
    $app_dir = $StringDefault;
    $cvs_dir = $StringDefault;
    $class_name = $StringDefault;

    if ($dbg2) {
	print(STDERR "$subname: Inputs...\n");
	print(STDERR "\tfname: $fname\n");
	print(STDERR "\tcvs_co_dir: $cvs_co_dir\n");
    }

    # Get the current app dir, remove the filename from the dir
    # Remove the filename from the dir to get the cvs directory.
    # Get the position of the last slash
    
    $last_slash=rindex($fname, "/");
    $app_dir=substr($fname, 0, $last_slash);

    if ($dbg2) {
	print(STDERR "$subname: remove fname, app_dir: $app_dir\n");
    }

    # Get the length of the cvs reference directory 
    # and add 1 to remove the slash. dir will be the cvs directory.
    # Need to be careful NOT to strip off the 'apps' level if it is
    # in the $cvs_co_dir

    if ($cvs_co_dir =~ /\/apps/) {
	$apps_pos = index($app_dir, "/apps");
	$dir=substr($cvs_co_dir, 0, $apps_pos);
    } else {
	$dir=$cvs_co_dir;
    }

    $len=length($dir);
    $cvs_dir=substr($app_dir, $len+1);

    if ($dbg2) {
	print(STDERR "$subname: remove cvs_co_dir, cvs_dir: $cvs_dir\n");
    }

    # Now want the app-class so can use in doxygen output to avoid
    # namespace collisions. Take the first directory below apps in
    # the CVS directory name as the app-class name

    @dirs=split('/', $cvs_dir);
    $found=0;
    $found_first=0;
    foreach $dir (@dirs) {
	if ($dir eq "apps") {
	    $found_first=1;
	    next;
	}
	if ($found_first) {
	    $class_name=$dir;
	    $found_first=0;
	    $found=1;
	}
	if ($found) {
	    next;
	}
    }

    # Debug

    if ($dbg) {
	print(STDERR "$subname: fname: $fname\n");
	print(STDERR "\tapp_dir: $app_dir, cvs_dir: $cvs_dir, class_name: $class_name\n");
    }

    # Return
    
    $return_val=1;
    return($return_val, $app_dir, $cvs_dir, $class_name);
}


#--------------------------------------------------------------------#
# Subroutine: GetAppLibDepend
#
# Usage: ($return_val, $list) = GetAppLibDepend($makefile, $dbg)
#
# Function: Read the input Makefile to extract the list of
#           libraries that this application is dependent on.
#           Note that this is hardwired to look for a string 
#           $Makefile_ldlibs_string in the $makefile.
#
# Input:    makefile       name of Makefile to read
#           dbg            debug flag
# 
# Output:   return_val     1 if success, 0 if failure
#           list           list of libraries
#
# Overview:
# 
#

sub GetAppLibDepend {
    local($makefile, $dbg)  = @_;

    # Define local variables

    local($return_val, $subname, $list);
    local($key, $name, $found, $dbg2, $dbg3);

    # Debugging

    $dbg2=0;
    $dbg3=0;
    if ($dbg == 2) {
	$dbg2=1;
    }
    if ($dbg == 3) {
	$dbg2=1;
	$dbg3=1;
    }

    # Set defaults

    $return_val = 0;
    $subname = "GetAppLibDepend";
    $list = $StringDefault;
    $found = 0;

    # Open and read the Makefile

    if (!open(MKFILE, $makefile)) {
	print(STDERR "ERROR: $subname: Cannot open input file $makefile\n");
	return($return_val, $appname);
    }
    
    while ($line = <MKFILE>) {

	if ($line =~ /^$Makefile_ldlibs_string/) {
	    ($key, $name) = split(/=/, $line);
	    $found = 1;

	    $list=$name;

	    if ($dbg2) {
		print (STDERR "$subname: Found $Makefile_ldlibs_string: list: $list, line: $line");
	    }

	    # are we inside a multi-line definition?

	    if ($line =~ /\\/) {
		$inside_defn = 1;
	    }

	} elsif (($found) && ($inside_defn)) {
	    
	    # Remove the trailing newline

	    $list = $list . $line;

	    # is there a line continuation?

	    if ($line !~ /\\/) {
		$inside_defn = 0;
	    } else {
		if ($dbg2) {
		    print(STDERR "$subname: Found line continuation line: $line");
		}
	    }
	} #endelsif
    } # endwhile

    close(MKFILE);

    if (!$found) {
	print(STDERR "WARNING: $subname: did not find $Makefile_ldlibs_string in $makefile\n");
    }

    if (($found) && ($dbg)) {
	print(STDERR "$subname: List of libs for $makefile is $list\n");
    }

    # Done

    $return_val = $found;
    return($return_val, $list);
}


#---------------------------------------------------------------------#
# Subroutine: GetCvsDocUrl
#
# Usage: ($return_val, $url) = 
#             GetCvsDocUrl($name, $class, $cvs_doc_dir, $www_doc_dir, $dbg)
#
# Function: Look for the CVS doc file for this app name. Construct
#           the URL and return it.
#
# Input:    name           app name
#           class          class name
#           cvs_doc_dir    CVS top level doc dir
#           www_doc_dir    web-accessible location for $cvs_doc_dir
#           dbg            debug flag
# 
# Output:   return_val     1 if success, 0 if failure
#           url            URL for doc file
#
# Overview:
# 
#

sub GetCvsDocUrl {
    local($name, $class, $cvs_doc_dir, $www_doc_dir, $dbg)  = @_;

    # Define local variables

    local($return_val, $subname, $dbg2, $dbg3);
    local($dir, $url, $found, $found_href, $found_dir, $file, $tmpfile);
    local($len);
    
    # Debugging

    $dbg2=0;
    $dbg3=0;
    if ($dbg == 2) {
	$dbg2=1;
    }
    if ($dbg == 3) {
	$dbg2=1;
	$dbg3=1;
    }

    if ($dbg2) {
	print(STDERR "$subname: Inputs...\n");
	print(STDERR "\tname: $name, class: $class\n");
	print(STDERR "\tcvs_doc_dir: $cvs_doc_dir\n");
	print(STDERR "\twww_doc_dir: $www_doc_dir\n");
    }

    # Set defaults

    $return_val = 0;
    $subname = "GetCvsDocUrl";
    $url=$StringDefault;

    # Does a directory exist in the CVS doc directory for this app-class?
    # Return if not

    $dir="${cvs_doc_dir}/${class}";
    if (!-d $dir) {
	if ($dbg2) {
	    print(STDERR "INFO: $subname: Did not find a CVS doc in $dir\n");
	}
	return($return_val, $url);
    }
    
    # Is there an index.html file at the app-class level with a reference
    # to the app-name?

    $found=0;
    $found_href=0;
    $file="${dir}/index.html";
    if (-f $file) {

	# Open and read the index.html

	if (!open(CLASS_DOC_FILE, $file)) {
	    print(STDERR "ERROR: $subname: Cannot open file $file\n");
	}
	else {
    	    while ($line = <CLASS_DOC_FILE>) {

		# Skip if found

		if ($found_href) {
		    next;
		}

		# Skip blank lines

		if ($line !~ /\w/) {
		    next;
		}

		# Find the name

		if ($line =~ /$name/) {
	    
		    $found = 1;

		    if ($dbg2) {
			print(STDERR "$subname: Found $name in $file\n");
		    }

		    print(STDERR "line: $line");
		    
		    # Best is to find the name as an HREF NAME 
		    # Pattern to find is <A NAME=$name>
		    # The reason for the more complex regular expression
		    # is to allow spaces and quotes in the <A NAME=$name>
		    # string

		    if ($line =~ /\<\s*A\s+NAME\s*\=\s*\"?$name\"?/) {
			
			$found_href=1;

			if ($dbg2) {
			    print(STDERR "$subname: Found A NAME HREF $name in $file at line: $line\n");
			}
			break;
		    }
		}

	    } # endwhile

	    close(CLASS_DOC_FILE);

	    if ($dbg2) {
		print(STDERR "$subname: name: $name, found: $found, found_href: $found_href\n");
	    }
	} #endelse
    } #endif (-f file)

    # is there a subdirectory for this app name instead of an index.html file
    # under the app-class?

    $found_dir=0;
    $dir="${dir}/${name}";
    if (-d $dir) {
	$found_dir=1;
	$found=1;
    }
    
    # If nothing found, return

    if (!$found) {
	if ($dbg2) {
	    print(STDERR "$subname: no CVS doc found for name: $name, class: $class\n");
	}
	return($return_val, $url);
    }

    # Now decide what to return as the URL
    
    $url="${www_doc_dir}/${class}/index.html";
    if ($found_href) {

	# Remove the $cvs_doc_dir from the $file and replace with the 
	# $www_doc_dir so is web enabled url
	$len=length($cvs_doc_dir);
	$tmpfile=substr($file, $len+1);
	$url="${www_doc_dir}/${tmpfile}\#$name";
    } elsif ($found) {
	if ($found_dir) {
	    $url="${www_doc_dir}/${class}/${name}";
	}
    }

    # Debug

    if ($dbg2) {
	print(STDERR "$subname: url: $url\n");
    }

    # Done

    $return_val = $found;
    return($return_val, $url);
}

#--------------------------------------------------------------------#
# Subroutine: RunDoxygen
#
# Usage: ($return_val, $url) =
#             &RunDoxygen($name, $class, $list, $conf, $tmp, $tdir, $ldir, $odir, $www_odir, 
#                         $sdir, $dbg)
#
# Function: Generate doxygen file
#
# Input:    $name          app name
#           $class         app class
#           $list          list of library dependencies (only for apps)
#           $conf          Doxygen config file
#           $tmp           temporary directory
#           $tdir          Doxygen tagfile dir
#           $ldir          Doxygen libs dir (only for apps)
#           $odir          Top-level Doxygen output directory
#           $www_odir      web-accessible path to $odir
#           $sdir          full pathname to source directory for this app
#           dbg            debug flag
# 
# Output:   return_val     1 if success, 0 if failure
#           $url           URL to doxygen file
#
# Overview:
# 
#

sub RunDoxygen {
    local($name, $class, $list, $conf, $tmp, $tdir, $ldir, $odir, $www_odir, $sdir, $dbg)  = @_;

    # Define local variables

    local($return_val, $subname);
    local($dbg2, $dbg3, $dbg5);
    local($tmpfile, $url, $newline, $cmd, $file, $outdir);
    local($narr, *arr, $i, $taglist, $str);

    # Debugging

    $dbg2=0;
    $dbg3=0;
    if ($dbg == 2) {
	$dbg2=1;
    }
    if ($dbg == 3) {
	$dbg2=1;
	$dbg3=1;
    }
    if ($dbg == 5) {
	$dbg5=1;
    }

    # Set defaults

    $return_val = 0;
    $subname = "RunDoxygen";
    $url=$StringDefault;

    # Debug

    if ($dbg2) {
	print(STDERR "$subname: Input...\n");
	print(STDERR "\tname: $name, class: $class, list: $list, conf: $conf, tmp: $tmp\n");
	print(STDERR "\ttdir: $tdir, ldir: $ldir, odir: $odir\n");
	print(STDERR "\ww_odir: $www_odir, sdir: $sdir\n");
    }

    # Get the libs from the list

    if ($DoApps) {
	($is_ok, $narr)=GetLibsFromList($list, *arr, $dbg);
    }

    # Delete the tmp file if it exists

    $tmpfile="${tmp}/doxygen_config.tmp";
    if (-f $tmpfile) {
	unlink($tmpfile);
    }

    # Open and read the config file

    if (!open(CONF_FILE, $conf)) {
	print(STDERR "ERROR: $subname: Cannot open config file $conf\n");
	return($return_val, $url);
    }

    # Open the temp file for writing

    if (!open(TMP_FILE, "> $tmpfile")) {
	print(STDERR "ERROR: $subname: Cannot open tmp file $tmpfile... \n");
	return($return_val, $url);
    }

    # Go thru the config file and write to the temp file,
    # modify the needed lines

    if ($dbg5) {
	print(STDERR "$subname: name: $name, class: $class, list: $list\n");
    }
    
    while ($line = <CONF_FILE>) {

	# Look for the 2 critical lines to change, eg:
	#   OUTPUT_DIRECTORY       = /d1/sdg/data/doxygen_html/apps
	#   INPUT                  = /d1/sdg/data/cvs/apps
	# Want to append the class-name and app-name to the output directory
	# Need to set the input directory

	if ($line =~ /^OUTPUT_DIRECTORY/) {
	    if ($DoApps) {
		$outdir="${odir}/${class}/${name}";
	    } elsif ($DoLibs) {
		$outdir="${odir}/${name}";
	    }
	    $newline="OUTPUT_DIRECTORY = ${outdir}\n";
	    if ($dbg5) {
		print(STDERR "$subname: Replace line: $line");
		print(STDERR "\twith: $newline");
	    }
	    $line=$newline;
	}

	if ($line =~ /^INPUT/) {
	    $newline="INPUT = ${sdir}\n";
	    if ($dbg > 5) {
		print(STDERR "$subname: Replace line: $line");
		print(STDERR "\twith: $newline");
	    }
	    $line=$newline;
	}

	# If we are doing apps AND tagfiles are set,
	# look for the TAGFILES line to change to add info on the
	# library tagfiles

	if (($DoApps) && ($tdir !~ /$StringDefault/) && ($line =~ /^TAGFILE/)) {
	    $taglist="";
	    for ($i=0; $i<$narr; $i++) {
		$str=$arr[$i];
		$taglist="${taglist} ${tdir}/${str}.tag";
	    }
	    $newline="TAGFILES = $taglist\n";
	    if ($dbg5) {
		print(STDERR "$subname: Replace line: $line");
		print(STDERR "\twith: $newline");
	    }
	    $line=$newline;
	}

	# If we are doing libs AND tagfiles are set,
	# look for the GENERATE TAGFILE line to change to add info
	# on the output dir

	if (($DoLibs) && ($tdir !~ /$StringDefault/) && ($line =~ /^GENERATE_TAGFILE/)) {
	    
	    $newline="GENERATE_TAGFILE = ${tdir}/${name}.tag";
	    if ($dbg5) {
		print(STDERR "$subname: Replace line: $line");
		print(STDERR "\twith: $newline");
	    }
	    $line=$newline;
	}

	# Print to output file

	print(TMP_FILE $line);

    } #endwhile

    close(TMP_FILE);
    close(CONF_FILE);

    # Create the output directory

    if (!-d $outdir) {
	$cmd="mkdir -p $outdir";
	if ($dbg > 0) {
	    print(STDERR "$subname: Running cmd: $cmd\n");
	}
	if (!$Test) {
	    system($cmd);
	}
    }

    # Now that we have a doxygen config file, run doxygen

    $cmd="doxygen $tmpfile";
    if ($dbg5) {
	print(STDERR "$subname: Running cmd: $cmd\n");
    }

    if (!$Test) {
	system($cmd);
    }
    
    # Construct the doxygen output file, if it exists,
    # set the return URL

    if ($DoApps) {
	$file="${odir}/${class}/${name}/html/index.html";
    } elsif ($DoLibs) {
	$file="${odir}/${name}/html/index.html";
    }

    if (!$Test) {
	if (-f $file) {
	    $return_val=1;
	    if ($DoApps) {
		$url="${www_odir}/${class}/${name}/html/index.html";
	    } elsif ($DoLibs) {
		$url="${www_odir}/${name}/html/index.html";
	    }
	} else {
	    $return_val=0;
	}
    }

    # If doing apps AND the tdir is set, run installdox
    # Need to 'cd' to the directory where the app doxygen html files are and run:
    #   ./installdox -l <lib1>.tag@<place1> -l <lib2>.tag@<place2>
    # where lib1 is a libname and place1 is where the lib1 tagfiles are

    if (($DoApps) && ($tdir !~ /$StringDefault/)) {
	$taglist="";
	for ($i=0; $i<$narr; $i++) {
	    $str=$arr[$i];
	    $taglist="${taglist} -l ${str}.tag\@${ldir}";
	}

	# Does the installdox command exist?

	if (-e "${odir}/${class}/${name}/html/installdox") {
	    $cmd="cd ${odir}/${class}/${name}/html; ./installdox $taglist; cd";
	    if ($dbg5) {
		print(STDERR "$subname: Running cmd: $cmd\n");
	    }

	    if (!$Test) {
		system($cmd);
	    }
	} else {
	    print(STDERR "Error: $subname: installdox does not exist: ${odir}/${class}/${name}/html/installdox\n");
	}
    }
    
    if ($dbg2) {
	print(STDERR "$subname: url: $url\n");
    }

    # Done

    return($return_val, $url);
}


#--------------------------------------------------------------------#
# Subroutine: GetDoxygenUrl
#
# Usage: ($return_val, $url) =
#             &GetDoxygenUrl($name, $class, $dir, $www_dir, $dbg)
#
# Function: Get the URL to the doxygen file
#
# Input:    $name          app name
#           $class         app class
#           $dir           directory of Doxygen-generated documents
#           $www_dir       web-accessible $dir
#           dbg            debug flag
# 
# Output:   return_val     1 if success, 0 if failure
#           $url           URL to doxygen file
#
# Overview:
# 
#

sub GetDoxygenUrl {
    local($name, $class, $dir, $www_dir, $dbg)  = @_;

    # Define local variables

    local($return_val, $subname);
    local($dbg2, $dbg3);
    local($url, $file);

    # Debugging

    $dbg2=0;
    $dbg3=0;
    if ($dbg == 2) {
	$dbg2=1;
    }
    if ($dbg == 3) {
	$dbg2=1;
	$dbg3=1;
    }

    # Set defaults

    $return_val = 0;
    $subname = "GetDoxygenUrl";
    $url=$StringDefault;

    # Debugging

    if ($dbg2) {
	print(STDERR "$subname: Inputs...\n");
	print(STDERR "\tname: $name, class: $class, dir: $dir, www_dir: $www_dir\n");
    }

    # Construct the path to the doxygen file. If it exists,
    # construct the URL using the $www_dir

    $file="${dir}/${class}/${name}/html/index.html";

    if (-f $file) {
	$return_val=1;
	$url="${www_dir}/${class}/${name}/html/index.html";
    } else {
	$return_val=0;
    }

    # Debug

    if ($dbg2) {
	print(STDERR "$subname: url: $url, found: $return_val\n");
    }

    # Done

    return($return_val, $url);
    
}

#---------------------------------------------------------------------#
# Subroutine: GetLibsFromList
#
# Usage: ($return_val, $narr) = 
#             GetLibsFromList($list, *arr, $dbg)
#
# Function: Parse the $list and return the list of library names in *arr
#
# Input:    list           LDLIBS string from Makefile
#           dbg            debug flag
# 
# Output:   return_val     1 if success, 0 if failure
#           arr            array of libs
#           narr           size of arr
#
# Overview:
# 
#

sub GetLibsFromList {
    local($list, *arr, $dbg)  = @_;

    # Define local variables

    local($return_val, $subname, $dbg2, $dbg3, $dbg4);
    local($narr);
    local($loc, $i, $item, *tmp_arr, $tmp_narr, $wanted);
    
    # Debugging

    $dbg2=0;
    $dbg3=0;
    $dbg4=0;
    if ($dbg == 2) {
	$dbg2=1;
    }
    if ($dbg == 3) {
	$dbg2=1;
	$dbg3=1;
    }
    if ($dbg == 4) {
	$dbg2=1;
	$dbg3=1;
	$dbg4=1;
    }

    # Set defaults

    $return_val = 0;
    $subname = "GetLibsFromList";
    $narr=0;

    if ($dbg2) {
	print(STDERR "$subname: Inputs...\n");
	print(STDERR "\tlist: $list\n");
    }

    # Parse the libs from the string
    # Expect it to look like: 
    #     -lSpdb -ldsserver -ldidss -lrapformats -leuclid \ -ltoolsa -ldataport -ltdrp -lm     

    # ... split on spaces ...

    @tmp_arr=split(" ", $list);
    $tmp_narr=@tmp_arr;

    # ... cleanup the individual list members ...

    for ($i=0; $i<$tmp_narr; $i++) {
	$item=$tmp_arr[$i];
	$wanted=1;

	if ($dbg4) {
	    print(STDERR "$subname: i: $i, tmp_arr: $item\n");
	}

	# ... remove -l ...

	if ($item =~ /\-l/) {
	    $loc=index($item, "\-l");
	    $item=substr($item, $loc+2);
	    $wanted=1;
	}

	# ... remove slashes ...
	
	if ($item =~ /\\/) {
	    $wanted=0;
	}

	# ... remove items that have dollar signs in them

	if ($item =~ /\$/) {
	    $wanted=0;
	}

	# ... remove unknown

	if ($item =~ /$StringDefault/) {
	    $wanted=0;
	}

	# ... remove system libs

	if ((($item =~ /m/) && (length($item) == 1)) ||
	    ($item =~ /pthread/) || ($item =~ /^pgf90/) ||
	    ($item =~ /^pgc/) || ($item =~ /^gcc/) ||
	    ($item =~ /Bstatic/) || ($item =~ /^pgmp/) ||
	    ($item =~ /^g2c/) || ($item =~ /^pgf/) ||
	    ($item =~ /^X11/) || ($item =~ /^Xpm/) ||
	    ($item =~ /^Xm/) || ($item =~ /^Xext/) ||
	    ($item =~ /^Xt/) || ($item =~ /^netcdf/) ||
	    ($item =~ /^jasper/) || ($item =~ /ncarg/)) {
	    if ($dbg4) {
		print(STDERR "\tSkipping system lib: -l${item}\n");
	    }
	    $wanted=0;
	}

	# Add to the output array

	if ($wanted > 0) {
	    if ($dbg4) {
		print(STDERR "Add to array: item: $item, narr: $narr\n");
	    }
	    $arr[$narr]=$item;
	    $narr++;
	}
    }
    

    # debug

    if ($dbg2) {
	print(STDERR "$subname: narr: $narr\n");
	for ($i=0; $i<$narr; $i++) {
	    print(STDERR "\ti: $i, arr at i: $arr[$i]\n");
	}
    }

    # Done

    $return_val=1;
    return($return_val, $narr);
   
}

#--------------------------------------------------------------------#
# Subroutine: doSort
#
# Usage: $return_val=doSort(*arr, $narr, *sorted_arr, $dbg)
#
# Function: Sort the *arr based on $arr[n][$NameIdx]
#
# Input:    *arr           array
#           $narr          size of *arr
#           $dbg           debug flag
# 
# Output:   $return_val    1 if success, 0 if failure
#           *sorted_arr    results of sorted *arr    
#
# Overview:
# 
#

sub doSort {
    local(*arr, $narr, *sorted_arr, $dbg)  = @_;

    # Define local variables

    local($return_val, $subname);
    local($dbg2, $dbg3);
    local(*tmp_arr, *sorted_tmp_arr, $i, $name, $found, $j, $idx);

    # Debugging

    $dbg2=0;
    $dbg3=0;
    if ($dbg == 2) {
	$dbg2=1;
    }
    if ($dbg == 3) {
	$dbg2=1;
	$dbg3=1;
    }

    # Set defaults

    $return_val = 1;
    $subname = "doSort";

    # Build a reference array with only the names in it for sorting

    for ($i=0; $i<$narr; $i++) {
	$tmp_arr[$i]=$arr[$i][$NameIdx];
    }
	
    # Sort the tmp_arr case-insensitively
    #       @articles = sort {uc($a) cmp uc($b)} @files

    @sorted_tmp_arr = sort {uc($a) cmp uc($b)} @tmp_arr;

    # Go through the sorted tmp array to build the return sort array

    for ($i=0; $i<$narr; $i++) {
	$name=$sorted_tmp_arr[$i];
	
	$found=0;
	for ($j=0; $j<$narr; $j++) {
	    if ($found) {
		next;
	    }
	    if ($name eq $arr[$j][$NameIdx]) {
		$found=1;
		$idx=$j;
	    }
	}

	if (!$found) {
	    $return_val=0;
	    print(STDERR "ERROR: $subname: Cannot find $name in arr!\n");
	}
	
	$sorted_arr[$i][$NameIdx]=$arr[$idx][$NameIdx];
	$sorted_arr[$i][$DoxygenUrlIdx]=$arr[$idx][$DoxygenUrlIdx];
	$sorted_arr[$i][$CvsDirIdx]=$arr[$idx][$CvsDirIdx];
	$sorted_arr[$i][$LibDepIdx]=$arr[$idx][$LibDepIdx];
	$sorted_arr[$i][$CvsDocUrlIdx]=$arr[$idx][$CvsDocUrlIdx];
    }

    # Done

    return($return_val);
}

#--------------------------------------------------------------------#
# Subroutine: WriteHTMLHeader
#
# Usage: $return_val = WriteHTMLHeader($file, $flag, $dbg)
#
# Function: Write the header info to the output $file in HTML table
#           format.
#
# Input:    file           file handle to open output file
#           flag           do_doxygen 1 =yes, 0=no
#           dbg            debug flag
# 
# Output:   return_val     1 if success, 0 if failure
#
# Overview:
# 
#

sub WriteHTMLHeader {
    local($file, $flag, $dbg)  = @_;

    # Define local variables

    local($return_val, $subname);
    local($dbg2, $dbg3);

    # Debugging

    $dbg2=0;
    $dbg3=0;
    if ($dbg == 2) {
	$dbg2=1;
    }
    if ($dbg == 3) {
	$dbg2=1;
	$dbg3=1;
    }

    # Set defaults

    $return_val = 0;
    $subname = "WriteHTMLHeader";

    # Open the output file

    if (!open(OUTFILE, "> $file")) {
	print(STDERR "ERROR: $subname: Cannot open output file $file... \n");
	exit $Exit_failure;
    }

    # Do generic stuff

    printf(OUTFILE "<HTML>\n");
    printf(OUTFILE "<HEAD>\n");
    printf(OUTFILE "<TITLE>$Title</TITLE>\n");
#    printf(OUTFILE "<STYLE TYPE=text/css></STYLE>\n");
    printf(OUTFILE "</HEAD>\n");
    printf(OUTFILE "<BODY BGCOLOR=\"white\">\n");
    printf(OUTFILE "<H1 ALIGN=CENTER>$Title</H1>\n");
    printf(OUTFILE "<p>\n");
    printf(OUTFILE "Updated: $Today\n");
    printf(OUTFILE "<p>\n");

    # Do header

    printf(OUTFILE "<table border>\n");
    printf(OUTFILE "<tr>\n");
    printf(OUTFILE "<th align=left>Name</th>\n");
    printf(OUTFILE "<th align=left>Doxygen URL</th>\n");

    if ($DoApps) {
	printf(OUTFILE "<th align=left>CVS source dir</th>\n");
	printf(OUTFILE "<th align=left>CVS doc URL</th>\n");
	printf(OUTFILE "<th align=left>Library dependencies</th>\n");
    }

    printf(OUTFILE "</tr>\n");

    close(OUTFILE);
	
    # Done

    $return_val = 1;
    return($return_val);
}


#--------------------------------------------------------------------#
# Subroutine: WriteHTMLTable
#
# Usage: $return_val = WriteHTMLTable($file, *arr, $narr, $dbg)
#
# Function: Write the *arr info to the $file in table format.
#           Open the $file as APPEND.
#
# Input:    file           file handle to open output file
#           *arr           array of app information
#           $narr          size of *arr
#           dbg            debug flag
# 
# Output:   return_val     1 if success, 0 if failure
#
# Overview:
# 
#

sub WriteHTMLTable {
    local($file, *arr, $narr, $dbg)  = @_;

    # Define local variables

    local($return_val, $subname);
    local($i, $name, $doxygen_url, $cvs_dir, $lib_dep, $cvs_doc_url);

    # Set defaults

    $return_val = 0;
    $subname = "WriteHTMLTable";

    # Open the output file

    if (!open(OUTFILE, ">> $file")) {
	print(STDERR "ERROR: $subname: Cannot open output file $file... \n");
	exit $Exit_failure;
    }

    # Write info to output file in HTML format

    for ($i=0; $i<$narr; $i++) {
	$name=$arr[$i][$NameIdx];
	$doxygen_url=$arr[$i][$DoxygenUrlIdx];

	if ($DoApps) {
	    $cvs_dir=$arr[$i][$CvsDirIdx];
	    $lib_dep=$arr[$i][$LibDepIdx];
	    $cvs_doc_url=$arr[$i][$CvsDocUrlIdx];
	}

	printf(OUTFILE "<tr>\n");
	printf(OUTFILE "<TD> $name\n");
	if ($doxygen_url eq $StringDefault) {
	    printf(OUTFILE "<TD></TD>\n");
	} else {
	    printf(OUTFILE "<TD> <A HREF=\"$doxygen_url\">Doxygen link</A>\n");
	}

	if ($DoApps) {

	    printf(OUTFILE "<TD> $cvs_dir\n");

	    if ($cvs_doc_url eq $StringDefault) {
		printf(OUTFILE "<TD></TD>\n");
	    } else {
		printf(OUTFILE "<TD> <A HREF=\"$cvs_doc_url\">Doc link</A>\n");
	    }

	    if ($lib_dep eq $StringDefault) {
		printf(OUTFILE "<TD></TD>\n");
	    } else {
		printf(OUTFILE "<TD> $lib_dep\n");
	    }
	}

	printf(OUTFILE "</tr>\n");
    }

    # Close the output file

    close(OUTFILE);

    # Done

    $return_val = 1;
    return($return_val);
}


#--------------------------------------------------------------------#
# Subroutine: WriteHTMLFooter
#
# Usage: $return_val = WriteHTMLFooter($file, $doc_dir, $co_script,
#                                      $author, $author_email, $dbg)
#
# Function: Write the footer info to the output $file in HTML table
#           format. Open the file as APPEND.
#
# Input:    file           file handle to open output file
#           doc_dir        CVS doc directory, to put into footer
#           co_script      checkout script, to put into footer
#           author         name of author to put into footer
#           author_email   email address of author to put into footer
#           host           hostname to put into footer
#           dbg            debug flag
# 
# Output:   return_val     1 if success, 0 if failure
#
# Overview:
# 
#

sub WriteHTMLFooter {
    local($file, $doc_dir, $co_script, $author, $author_email, $host, $dbg)  = @_;

    # Define local variables

    local($return_val, $subname);
    local($datetime);

    # Set defaults

    $return_val = 0;
    $subname = "WriteHTMLFooter";

    # Open the output file

    if (!open(OUTFILE, ">> $file")) {
	print(STDERR "ERROR: $subname: Cannot open output file $file... \n");
	exit $Exit_failure;
    }

    # Close the table

    printf(OUTFILE "</table>\n");
    printf(OUTFILE "</p>\n");

    # Print instructions for updating this page

    printf(OUTFILE "<HR>\n");
    printf(OUTFILE "NOTE:\n");
    printf(OUTFILE "<UL>\n");
    printf(OUTFILE "<LI>This page is updated automatically by the script: $prog\n");
    printf(OUTFILE "run on host $host\n");
    printf(OUTFILE "<LI>The CVS documents are under: $doc_dir\n");

    printf(OUTFILE "<LI> The applications or libraries in the left column in this table are dictated by the checkout ");
    printf(OUTFILE "script: $co_script\n");
    printf(OUTFILE "</UL>\n");
    printf(OUTFILE "<P>\n");

    # Print today's date

    printf(OUTFILE "<HR>\n");
    printf(OUTFILE "Updated at: $Today\n");
    printf(OUTFILE "<p>\n");
    
    # Print email address
    
    printf(OUTFILE "<ADDRESS>\n");
    printf(OUTFILE "Author: $author<BR>\n");
    printf(OUTFILE "RAP, NCAR, P.O. Box 3000, Boulder, CO, 80307-3000<BR>\n");
    printf(OUTFILE "<A HREF=mailto:$author_email>$author_email</A> <BR>\n");
    printf(OUTFILE "</ADDRESS>\n");

    # Print final

    printf(OUTFILE "</BODY>\n");
    printf(OUTFILE "</HTML>\n");

    # Close output file

    close(OUTFILE);

    # Done

    $return_val = 1;
    return($return_val);
}


#--------------------------------------------------------------------#
# Subroutine: ChopName
#
# Usage: ($return_val, $outname) = ChopName($inname, $dbg)
#
# Function: Chop the trailing newline and any leading spaces
#           of off $inname and return in $outname.
#
# Input:    inname         name to chop
#           dbg            debug flag
# 
# Output:   return_val     1 if success, 0 if failure
#           outname        chopped name
#
# Overview:
# 
#

sub ChopName {
    local($inname, $dbg)  = @_;

    # Define local variables

    local($return_val, $subname, $outname);
    local($junk);
    local($dbg2);

    # Set defaults

    $return_val = 0;
    $subname = "ChopName";
    $dbg2 = 0;

    # Remove trailing newline

    chop($inname);

    # Remove any leading spaces
	    
    if ($inname =~ /^\s/) {
	if ($dbg2) {
	    print(STDERR "$subname: found a leading space in inname: $inname\n");
	}
	($junk, $outname) = split(/\s/, $inname);
    } else  {
	$outname = $inname;
    }

    if ($dbg) {
	print(STDERR "$subname: chopped: $inname to create: $outname\n");
    }

    $return_val = 1;
    return($return_val, $outname);
}

#-------------------------------------------------------------------------
#
# Subroutine readParamFile
#
# Usage: $return_val = readParamFile($param_file, $debug)
#
# Function: Read the $param_file.
#
# Input:    $param_file          file to read
#           $debug               flag for debugging (1=on, 0=off)
#
# Output:   $ret_val             1 on success, 0 on error
#
# Overview:
#

sub readParamFile
{
  local ($param_file, $debug) = @_;

  # Local variables

  local($return_val, $subname);
  local($dbg2, $dbg3);
  local($is_ok, $keyword, $keyvalue, *out_arr, $type, $unstripped_keyvalue);

  # Set defaults

  $return_val=0;
  $subname="readParamFile";

  # Debug

  $dbg2=0;
  $dbg3=0;

  if ($debug == 2) {
      $dbg2=1;
  }

  if ($debug == 3) {
      $dbg3=1;
      $dbg2=1;
  }

  # Debugging

  if ($dbg2) {
      print(STDERR "$prog: $subname: Input paramfile: $param_file\n");
  }

  # Set global parameter defaults

  $is_ok = &setAllParams2Defaults;

  # Open the param file

  $is_ok=open(PARAM_FILE, $param_file);
  if (!$is_ok) {
      print(STDERR "ERROR: $prog: $subname: Cannot open file $param_file\n");
      return($return_val);
  }

  # Parse the file

  while ($line = <PARAM_FILE>) {

      # Skip comment lines

      if ($line =~ /^#/) {
	  next;
      }

      # Skip blank lines

      if ($line !~ /\w/) {
	  next;
      }

      # Debug

      if ($dbg3) {
	  print (STDERR $line);
      }

      # Have we found a line with an equals sign? Parse the keyword

      if ($line =~ /\=/) {

	  # Split the line at the equals sign

	  ($keyword, $keyvalue) = split('\=',$line);

	  # Are there >1 equals signs? If so, just split at the first one
	  
	  @junk=split('\=',$line);
	  if (@junk > 2) {
	      $firstequals=index($line, "=");
	      $keyword=substr($line, 0, $firstequals);
	      $keyvalue=substr($line, $firstequals+1);
	      
	      if ($dbg3) {
		  print(STDERR "more than 1 equals sign in the line\n");
		  print(STDERR "\tfirstequals: $firstequals, keyword: $keyword, keyvalue: $keyvalue\n");
	      }
	  }

	  # Chop off any leading and trailing blanks, save the unstripped
	  # keyvalue as we may need it in a special case

	  $unstripped_keyvalue=$keyvalue;
	  chop($unstripped_keyvalue);
	  ($is_ok, $keyword)=&removeSpaces($keyword, $debug);
	  ($is_ok, $keyvalue)=&removeSpaces($keyvalue, $debug);

	  # Are there any quotes? If so, use the unstripped keyvalue and remove
	  # the quotes, since there may be blanks in the string

	  if ($unstripped_keyvalue =~ /\"/) {
	      ($is_ok, $keyvalue)=&removeQuotes($unstripped_keyvalue, $debug);
	  }

	  # Handle the keywords

	  if ($keyword eq "apps_or_libs") {
	      $type="enum";
	      $Apps_or_libs=$keyvalue;
	      $DoApps=1;
	      $DoLibs=0;
	      if ($Apps_or_libs =~ /libs/) {
		  $DoApps=0;
		  $DoLibs=1;
	      } 
	  }

	  elsif ($keyword eq "output_file") {
	      $type="string";
	      $Output_fname= $keyvalue;
	      ($is_ok, $Output_fname) = &expandEnvVar($Output_fname, $debug);
	      if ($Output_fname !~ /\w+/) {
		  $Output_fname=$StringDefault;
	      }
	  }

	  elsif ($keyword eq "src_dir") {
	      $type="string";
	      $Src_dir= $keyvalue;
	      ($is_ok, $Src_dir) = &expandEnvVar($Src_dir, $debug);
	      if ($Src_dir !~ /\w+/) {
		  $Src_dir=$StringDefault;
	      }
	  }

	  elsif ($keyword eq "tmp_dir") {
	      $type="string";
	      $Tmp_dir= $keyvalue;
	      ($is_ok, $Tmp_dir) = &expandEnvVar($Tmp_dir, $debug);
	      if ($Tmp_dir !~ /\w+/) {
		  $Tmp_dir=$StringDefault;
	      }
	  }

	  elsif ($keyword eq "makefile_names") {
	      $type="comma_delimited_string";
	      ($is_ok, $Makefile_names) = &checkKeywordValue($keyword, $keyvalue, $type, *Makefile_names_arr, $debug);
	  }

	  elsif ($keyword eq "doxygen_exes") {
	      $type="comma_delimited_string";
	      ($is_ok, $Doxygen_exes) = &checkKeywordValue($keyword, $keyvalue, $type, *Doxygen_exes_arr, $debug);
	  }

	  elsif ($keyword eq "run_doxygen") {
	      $type="bool";
	      if ($keyvalue eq "TRUE") {
		  $Do_doxygen=1;
	      } else {
		  $Do_doxygen=0;
	      }
	  }

	  elsif ($keyword eq "doxygen_config_file") {
	      $type="string";
	      $Doxygen_config_file= $keyvalue;
	      ($is_ok, $Doxygen_config_file) = &expandEnvVar($Doxygen_config_file, $debug);
	      if ($Doxygen_config_file !~ /\w+/) {
		  $Doxygen_config_file=$StringDefault;
	      }
	  }

	  elsif ($keyword eq "doxygen_out_dir") {
	      $type="string";
	      $Doxygen_out_dir = $keyvalue;
	      ($is_ok, $Doxygen_out_dir) = &expandEnvVar($Doxygen_out_dir, $debug);
	      if ($Doxygen_out_dir !~ /\w+/) {
		  $Doxygen_out_dir=$StringDefault;
	      }
	  }

	  elsif ($keyword eq "doxygen_tagfile_dir") {
	      $type="string";
	      $Doxygen_tagfile_dir = $keyvalue;
	      ($is_ok, $Doxygen_tagfile_dir) = &expandEnvVar($Doxygen_tagfile_dir, $debug);
	      if ($Doxygen_tagfile_dir !~ /\w+/) {
		  $Doxygen_tagfile_dir=$StringDefault;
	      }

	  }

	  elsif ($keyword eq "doxygen_libs_dir") {
	      $type="string";
	      $Doxygen_libs_dir = $keyvalue;
	      ($is_ok, $Doxygen_libs_dir) = &expandEnvVar($Doxygen_libs_dir, $debug);
	      if ($Doxygen_libs_dir !~ /\w+/) {
		  $Doxygen_libs_dir=$StringDefault;
	      }

	  }

	  elsif ($keyword eq "cvs_doc_src_dir") {
	      $type="string";
	      $Cvs_doc_src_dir = $keyvalue;
	      ($is_ok, $Cvs_doc_src_dir) = &expandEnvVar($Cvs_doc_src_dir, $debug);
	  }
	  elsif ($keyword eq "www_doxygen_out_dir") {
 	      $type="string";
	      $Www_doxygen_out_dir = $keyvalue;
	      ($is_ok, $Www_doxygen_out_dir) = &expandEnvVar($Www_doxygen_out_dir, $debug);
	  }

	  elsif ($keyword eq "www_cvs_doc_src_dir") {
	      $type="string";
	      $Www_cvs_doc_src_dir = $keyvalue;
	      ($is_ok, $Www_cvs_doc_src_dir) = &expandEnvVar($Www_cvs_doc_src_dir, $debug);
	  }

	  elsif ($keyword eq "title") {
	      $type="string";
	      $Title = $keyvalue;
	      ($is_ok, $Title) = &expandEnvVar($Title, $debug);
	  }

	  elsif ($keyword eq "author") {
	      $type="string";
	      $Author = $keyvalue;
	      ($is_ok, $Author) = &expandEnvVar($Author, $debug);
	  }

	  elsif ($keyword eq "author_email") {
	      $type="string";
	      $Author_email = $keyvalue;
	      ($is_ok, $Author_email) = &expandEnvVar($Author_email, $debug);
	  }

	  elsif ($keyword eq "distribs_file") {
	      $type="string";
	      $Distribs_file = $keyvalue;
	      ($is_ok, $Distribs_file) = &expandEnvVar($Distribs_file, $debug);
	  }



	  else {
	      print(STDERR "ERROR: Cannot parse keyword line: $line");
	  }
	  
	  # End of keyword processing

	  next;
      }
	  
  } #endwhile

  close(PARAM_FILE);

  # ...Handle special cases...

  # Okay, did everything get set???

  ($is_ok, $missing) = &checkAllParams;

  if ($debug) {
      &printAllParams;
  }

  # Debugging

  # Set return

  if ($missing > 0) {
      $return_val = 0;
  } else {
      $return_val = 1;
  }

  # Debug
  
  if ($dbg2) {
    print(STDERR "$subname: return: $return_val, missing: $missing\n");
  }

  # Done

  return($return_val);
}


#------------------------------------------------------------------------
#
# Subroutine checkKeywordValue
#
# Usage: ($return_val, $valid_keyword_value) = 
#              checkKeywordValue($keyword, $input_keyword_value, $type, 
#                                *out_arr, $debug)
#
# Function: Check the $input_keyword_value against the $type and return
#           a valid value in $valid_keyword_value.
#
# Input:    $keyword             keyword, needed for error messages
#           $input_keyword_value value to check
#           $type                type to check
#           $debug               flag for debugging (1=on, 0=off)
#
# Output:   $return_val          1 on success, 0 on error
#           $valid_keyword_value valid value for the keyword, may be the
#                                  same as the $input_keyword_value
#           *out_arr             output array (only used for some keywords)
#
# Overview:
#

sub checkKeywordValue
{
  local ($keyword, $input_keyword_value, $type, *out_arr, $debug) = @_;

  # Local variables

  local($return_val, $subname);
  local($dbg2, $dbg3);
  local($found, $found_valid, $valid_keyword_value, $tmp_keyword_value);
  local($is_ok, $narr, $val);

  # Set defaults

  $return_val=0;
  $subname="checkKeywordValue";

  # Debug

  if ($debug == 2) {
      $dbg2=1;
  } else {
      $dbg2=0;
  }

  if ($debug == 3) {
      $dbg3=1;
      $dbg2=1;
  } else {
      $dbg3=0;
  }

  # Set the return value to equal the input value by default

  $valid_keyword_value=$input_keyword_value;

  # Deal with the various types if possible

  $found=1;

  # Is an int positive?

  if ($type eq "int"){

      if ($input_keyword_value < 0) {
	  print(STDERR "ERROR: $keyword $input_keyword_value is less than 0, will set to 0\n");
	  $valid_keyword_value=0;
      } 
  }

  # Is a float positive?

  if ($type eq "float"){

      if ($input_keyword_value < 0) {
	  print(STDERR "ERROR: $keyword $input_keyword_value is less than 0, will set to 0\n");
	  $valid_keyword_value=0.0;
      } 
  }

  # If a comma-delimited string, split the string

  elsif ($type = "comma_delimited_string") {
      @out_arr=split(',',$input_keyword_value);
      $narr=@out_arr;

      if ($dbg3) {
	  print(STDERR "$subname: keyword: $keyword, keyvalue: $keyvalue, numarr: $narr\n");
	  foreach $val (@out_arr) {
	      print(STDERR "\t$val\n");
	  }
      }

  } 

  # Error condition

  else {
      if ($debug) {
	  print(STDERR "$prog: $subname: Cannot resolve type: $type\n");
      }
      $found=0;
  }

  # Done

  $return_val=$found;
  return($return_val, $valid_keyword_value);
}

#------------------------------------------------------------------------------
#
# Subroutine removeSpaces
#
# Usage: ($return_val, $newstring)=removeSpaces($instring, $debug) 
#
# Function: Remove leading and trailing spaces from $instring and
#           return in $newstring. If no spaces are found, returns
#           the $instring.
#
# Input:    $instring            string to remove blanks from
#           $debug               flag for debugging (1=on, 0=off)
#
# Output:   $ret_val             1 on success, 0 on error
#           $newstring           $instring without leading/trailing
#                                   blanks
#
# Overview:
#

sub removeSpaces
{
  local ($instring, $debug) = @_;

  # Local variables

  local($return_val, $subname);
  local($dbg2);
  local($is_ok, $newstring);

  # Set defaults

  $return_val=0;
  $subname="removeSpaces";
  $newstring=$instring;

  # Debug

  if ($debug == 2) {
      $dbg2=1;
  } else {
      $dbg2=0;
  }

  # Do it

  ($newstring) = split (' ', $instring);

  # Done

  $return_val=1;
  return($return_val, $newstring);
}



#------------------------------------------------------------------------------
#
# Subroutine removeQuotes
#
# Usage: ($return_val, $newstring)=removeQuotes($instring, $debug) 
#
# Function: Remove leading and trailing quotes from $instring and
#           return in $newstring. If no quotes are found, returns
#           the $instring.
#
# Input:    $instring            string to remove quotes from
#           $debug               flag for debugging (1=on, 0=off)
#
# Output:   $ret_val             1 on success, 0 on error
#           $newstring           $instring without leading/trailing
#                                   quotes
#
# Overview:
#

sub removeQuotes
{
  local ($instring, $debug) = @_;

  # Local variables

  local($return_val, $subname);
  local($dbg2, $dbg3);
  local($is_ok, $newstring, *arr);

  # Set defaults

  $return_val=0;
  $subname="removeQuotes";
  $newstring=$instring;

  # Debug

  $dbg2=0;
  $dbg3=0;
  if ($debug == 2) {
      $dbg2=1;
  }
  if ($debug == 3) {
      $dbg3=1;
  }

  $dbg3=1;

  # If no quotes, return

  if ($instring !~ /\"/) {
      $return_val=1;
      return($return_val, $instring);
  }
	 
  # Do it. Weirdly, need the 2nd array element, not the first.

  @arr = split ('\"', $instring);
  $newstring=$arr[1];

  # Done

  $return_val=1;
  return($return_val, $newstring);
}



#------------------------------------------------------------------------
#
# Subroutine generateParamFile
#
# Usage: $return_val = generateParamFile
#
# Function: Generate a parameter file for this script.
#
# Input:    none
#
# Output:   $ret_val             1 on success, 0 on error
#           STDOUT               generates the param file text and writes it
#                                    to STDOUT
#
# Overview:
#

sub generateParamFile
{

  # Local variables

  local($return_val, $subname);

  # Set defaults

  $return_val=0;
  $subname="generateParamFile";

  # Start writing param stuff to STDOUT

  # ... print header ...

  print(STDOUT "# Parameters for $prog\n");
  print(STDOUT "#\n");
  print(STDOUT "# This param file was generated on: $Today\n");
  print(STDOUT "# Environment variables can be used but must be inside ()\n");
  print(STDOUT "#=====================================================\n");

  # ... Apps or Libs ...

  print(STDOUT "#------------ Apps or Libs ---------------\n");
  print(STDOUT "# apps_or_libs is a flag for whether we are parsing for apps\n");
  print(STDOUT "# or libs in this param file. Libs generate both tagfiles and HTML files.\n");
  print(STDOUT "# Apps use lib tagfiles and generate HTML files.\n");
  print(STDOUT "# Type: enum. Only valid options are: apps, libs\n");
  print(STDOUT "#\n");
  print(STDOUT "apps_or_libs = apps\n\n");

  # ... Output file ...

  print(STDOUT "#------------ Output file ----------------\n");
  print(STDOUT "# output_file is the name of the HTML output file to create\n");
  print(STDOUT "# Type: string\n");
  print(STDOUT "#\n");
  print(STDOUT "output_file = a.html\n\n");

  # ... Src dir ...

  print(STDOUT "#------------ Src dir ----------------\n");
  print(STDOUT "# src_dir is the directory with the checked-out apps source code\n");
  print(STDOUT "# in it. This directory will be searched recursively for any files\n");
  print(STDOUT "# that are in the list of makefile_names\n");
  print(STDOUT "# Type: string\n");
  print(STDOUT "#\n");
  print(STDOUT "src_dir = \n\n");

  # ... Tmp dir ...

  print(STDOUT "#------------ Tmp dir ----------------\n");
  print(STDOUT "# tmp_dir is the directory to use for small temporary files\n");
  print(STDOUT "# used by this script\n");
  print(STDOUT "# Type: string\n");
  print(STDOUT "#\n");
  print(STDOUT "tmp_dir = /tmp\n\n");

  # ... Makefile names ...

  print(STDOUT "#------------ Makefile names ----------------\n");
  print(STDOUT "# makefile_names is a list of Makefile names to open and look for\n");
  print(STDOUT "# the following keywords: TARGET_FILE and LOC_LIB. The TARGET_FILE\n");
  print(STDOUT "# sets the the application name and LOC_LIB sets the load lib info\n");
  print(STDOUT "# Type: comma-delimited string with no blanks or quotes\n");
  print(STDOUT "#\n");
  print(STDOUT "makefile_names = Makefile\n\n");

  # ... Doxygen exes ...

  print(STDOUT "#------------ Doxygen exes ----------------\n");
  print(STDOUT "# doxygen_exes is a list of doxygen binaries to search for in the\n");
  print(STDOUT "# path before starting to run.\n");
  print(STDOUT "# Type: comma-delimited string with no blanks or quotes\n");
  print(STDOUT "#\n");
  print(STDOUT "doxygen_exes = doxygen\n\n");

  # ... Run doxygen ...

  print(STDOUT "#------------ Run doxygen ----------------\n");
  print(STDOUT "# run_doxygen is a flag to run doxygen or not\n");
  print(STDOUT "# Type: enum, with the following options:\n");
  print(STDOUT "#         TRUE\n");
  print(STDOUT "#         FALSE\n");
  print(STDOUT "#\n");
  print(STDOUT "run_doxygen = TRUE\n\n");

  # ... Doxygen config file ...

  print(STDOUT "#------------ Doxygen config file ----------------\n");
  print(STDOUT "# doxygen_config_file is the full pathname to the Doxygen config\n");
  print(STDOUT "# file to use.\n");
  print(STDOUT "# Type: string\n");
  print(STDOUT "#\n");
  print(STDOUT "doxygen_config_file = \n\n");

  # ... Doxygen output dir ...

  print(STDOUT "#------------ Doxygen output dir ----------------\n");
  print(STDOUT "# doxygen_out_dir is the directory to output the doxygen files to\n");
  print(STDOUT "# Type: string\n");
  print(STDOUT "#\n");
  print(STDOUT "doxygen_out_dir = \n\n");

  # ... Doxygen tagfile dir ...

  print(STDOUT "#------------ Doxygen tagfile dir ----------------\n");
  print(STDOUT "# doxygen_tagfile_dir is the directory with the doxygen tagfiles\n");
  print(STDOUT "# for libraries in it. If apps_or_libs is apps, the directory and\n");
  print(STDOUT "# must already exist. If apps_or_libs is libs, will generate tagfiles\n");
  print(STDOUT "# in this directory. If not set, no tagfiles are generated or linked.\n");
  print(STDOUT "# Type: string\n");
  print(STDOUT "#\n");
  print(STDOUT "doxygen_tagfile_dir = \n\n");

  # ... Doxygen libs dir ...

  print(STDOUT "#------------ WWW Doxygen libs dir ----------------\n");
  print(STDOUT "# doxygen_libs_dir is the web-accessible (e.g., under /var/www)\n");
  print(STDOUT "# directory with the doxygen HTML files for libs in it.\n");
  print(STDOUT "# If apps_or_libs is libs, is ignored. If apps_or_libs is apps,\n");
  print(STDOUT "# and doxygen_tagfile_dir is set, this directory and files must already exist.\n");
  print(STDOUT "# Type: string\n");
  print(STDOUT "#\n");
  print(STDOUT "doxygen_libs_dir = \n\n");

  # ... Cvs doc src dir ...

  print(STDOUT "#------------ CVS doc src dir ----------------\n");
  print(STDOUT "# cvs_doc_src_dir is the disk directory with the checked-out apps doc\n");
  print(STDOUT "# files. This directory should have subdirectories for each app-class\n");
  print(STDOUT "# Type: string\n");
  print(STDOUT "#\n");
  print(STDOUT "cvs_doc_src_dir = \n\n");

  # ... WWW doxygen dir ...

  print(STDOUT "#------------ WWW doxygen output dir ----------------\n");
  print(STDOUT "# www_doxygen_out_dir is the web-accessible location for the\n");
  print(STDOUT "# doxygen-generated documents. It is the web-accessible location\n");
  print(STDOUT "# for the doxygen_out_dir.\n");
  print(STDOUT "# Type: string\n");
  print(STDOUT "#\n");
  print(STDOUT "www_doxygen_out_dir = \n\n");

  # ... WWW cvs doc dir ...

  print(STDOUT "#------------ WWW CVS doc src dir ----------------\n");
  print(STDOUT "# www_cvs_doc_src_dir is the web-accessible location for the\n");
  print(STDOUT "# checked-out apps doc files. It is the web-accessible location\n");
  print(STDOUT "# for the cvs_doc_src_dir.\n");
  print(STDOUT "# Type: string\n");
  print(STDOUT "#\n");
  print(STDOUT "www_cvs_doc_src_dir = \n\n");

  # ... author ...

  print(STDOUT "#========================= Reference Info =====================\n");

  print(STDOUT "#------------ Title ----------------\n");
  print(STDOUT "# title is the title to put into the HTML output_file\n");
  print(STDOUT "# Type: string\n");
  print(STDOUT "#\n");
  print(STDOUT "title = \"RAP Applications List\"\n\n");

  print(STDOUT "#------------ Author ----------------\n");
  print(STDOUT "# author is a string to put in the HTML output_file\n");
  print(STDOUT "# Type: string\n");
  print(STDOUT "#\n");
  print(STDOUT "author = \n\n");

  # ... author email ...

  print(STDOUT "#------------ Author email ----------------\n");
  print(STDOUT "# author_email is a string to put in the HTML output_file\n");
  print(STDOUT "# Type: email address\n");
  print(STDOUT "#\n");
  print(STDOUT "author_email = \n\n");

  # ... distribs file ...

  print(STDOUT "#------------ Distribs file ----------------\n");
  print(STDOUT "# distribs_file is the CVS path to the distribs file used to checkout\n");
  print(STDOUT "# the apps code from CVS and is a string to put into the HTML\n");
  print(STDOUT "# output_file for reference only\n");
  print(STDOUT "# Type: string\n");
  print(STDOUT "#\n");
  print(STDOUT "distribs_file = distribs/rap_doxygen_apps_checkout\n\n");

  # ... end of param file ...

  print(STDOUT "#\n");
  print(STDOUT "#-------------------------- End of File ---------------------\n");

  # Done

  $return_val=1;
  return($return_val);
}


#------------------------------------------------------------------------------
#
# Subroutine setAllParams2Defaults
#
# Usage: $return_val = setAllParams2Defaults
#
# Function: Initialize all the params to defaults
#
# Input:    none, all is handled by globals
#
# Output:   $ret_val             1 on success, 0 on error
#
# Overview:
#

sub setAllParams2Defaults
{
  # Local variables

  local($return_val, $subname);
  local($missing);

  # Set defaults

  $return_val=0;
  $subname="setAllParams2Defaults";
  $missing=0;

  $Apps_or_libs="apps";
  $DoApps=1;
  $DoLibs=0;
  $Output_fname="a.html";
  $Src_dir=$StringDefault;
  $Tmp_dir="/tmp";
  $Makefile_names="Makefile";
  $Doxygen_exes="doxygen";
  $Do_doxygen=1;
  $Doxygen_config_file=$StringDefault;
  $Doxygen_out_dir=$StringDefault;
  $Doxygen_tagfile_dir=$StringDefault;
  $Doxygen_libs_dir=$StringDefault;
  $Cvs_doc_src_dir=$StringDefault;
  $Www_doxygen_out_dir=$StringDefault;
  $Www_cvs_doc_src_dir=$StringDefault;
  $Title="RAP Applications List";
  $Author=$StringDefault;
  $Author_email=$StringDefault;
  $Distribs_file=$StringDefault;

  # Done

  $return_val=1;
  return($return_val);
}

#------------------------------------------------------------------------------
#
# Subroutine checkAllParams
#
# Usage: ($return_val, $nmissing) = checkAllParams
#
# Function: Check that all the params are set. Return the number missing.
#
# Input:    none, all is handled by globals
#
# Output:   $ret_val             1 on success, 0 on error
#           $nmissing            number of params not set
#
# Overview:
#

sub checkAllParams
{
  # Local variables

  local($return_val, $subname);
  local($missing, $num);

  # Set defaults

  $return_val=0;
  $subname="checkAllParams";
  $missing=0;

  # Do it

  if ($Src_dir eq $StringDefault) {
      print(STDERR "WARNING: src_dir not set\n");
      $missing++;
  }
  if (($Do_doxygen) && ($Doxygen_config_file eq $StringDefault)) {
      print(STDERR "WARNING: doxygen_config_file not set\n");
      $missing++;
  }

  # Done

  $return_val=1;
  return($return_val, $missing);
}


#------------------------------------------------------------------------------
#
# Subroutine printAllParams
#
# Usage: printAllParams
#
# Function: Print all the global params
#
# Input:    none, all is handled by globals
#
# Output:   none
#
# Overview:
#

sub printAllParams
{
  # Local variables

  local($subname, $name);

  # Set defaults

  $subname="printAllParams";

  # Print all the params

  print(STDERR "Printing all global params...\n");
  print(STDERR "\tapps_or_libs: $Apps_or_libs\n");
  print(STDERR "\toutput_fname: $Output_fname\n");
  print(STDERR "\tsrc_dir: $Src_dir\n");
  print(STDERR "\ttmp_dir: $Tmp_dir\n");
  print(STDERR "\tmakefile_names=$Makefile_names\n");
  if ($Debug_level > 1) {
      foreach $name (@Makefile_names_arr) {
	  print(STDERR "\t\tmakefile_names_arr: $name\n");
      }
  }
  print(STDERR "\tdoxygen_exes=$Doxygen_exes\n");
  if ($Debug_level > 1) {
      foreach $name (@Doxygen_exes_arr) {
	  print(STDERR "\t\tdoxygen_names_arr: $name\n");
      }
  }
  print(STDERR "\trun_doxygen: $Do_doxygen\n");
  print(STDERR "\tdoxygen_config_file: $Doxygen_config_file\n");
  print(STDERR "\tdoxygen_out_dir: $Doxygen_out_dir\n");
  print(STDERR "\tdoxygen_tagfile_dir: $Doxygen_tagfile_dir\n");
  print(STDERR "\tdoxygen_libs_dir: $Doxygen_libs_dir\n");
  print(STDERR "\tcvs_doc_src_dir: $Cvs_doc_src_dir\n");
  print(STDERR "\twww_doxygen_out_dir: $Www_doxygen_out_dir\n");
  print(STDERR "\twww_cvs_doc_src_dir: $Www_cvs_doc_src_dir\n");
  print(STDERR "\ttitle: $Title\n");
  print(STDERR "\tauthor: $Author\n");
  print(STDERR "\tauthor_email: $Author_email\n");
  print(STDERR "\tdistribs_file: $Distribs_file\n");
  
  # Done

  $return_val=1;
  return($return_val);
}


#---------------------------------------------------------------------
# Subroutine: expandEnvVar()
#
# Usage:      ($return_val, $expanded_string) = 
#                   expandEnvVar($string, $debug)
#
# Function:   Expand the environment variable on the input string
#
# Input:      $string        environment variable string to expand.
#             $debug         debug flag
#
# Output:     $return_val    1 on success or 0 on error (e.g., env var
#                            syntax used in string but env var not defined)
#             $expanded_string  the expanded environment variable, 
#                               or if not an enviroment variable, just returns 
#                               the input string.
# 

sub expandEnvVar {
    local ($string, $debug) = @_;

    # Local variables

    local($return_val, $sub_name);
    local($dbg2, $dbg3);
    local($prior_to_dollar, $after_dollar, $dollar_sign, $env_var, $remainder);

    # set defaults

    $subname="expandEnvVar";
    $return_val=0;
    $expanded_string=$string;

    # Debugging

    $dbg2=0;
    $dbg3=0;
    if ($debug == 2) {
	$dbg2=1;
    }
    if ($debug == 3) {
	$dbg2=1;
	$dbg3=1;
    }

    # Return if not an env var

    if ($string !~ /\$\(/) {
	$return_val=1;
	return($return_val, $string);
    }

    # Okay, we have a $() structure

    # First split on the $

    ($prior_to_dollar, $after_dollar) = split('\$', $string);

    # Add the dollar back on. This is needed for the next split to work correctly

    $after_dollar="\$" . $after_dollar;
    
    # Okay now split on the left paren

    ($dollar_sign, $env_var, $remainder) = split(/\(|\)/, $after_dollar);

    if ($dbg3) {
	print(STDERR "$subname: env_var: $env_var, remainder: $remainder\n");
    }

    if (defined $ENV{$env_var}) { 
	$expanded_env_var=$ENV{$env_var};
	$expanded_string=$prior_to_dollar . $expanded_env_var . $remainder;
	$return_val=1;
    }
    else {
	print(STDERR "WARNING: $sub_name: the environment variable $string is NOT defined\n");
	$return_val=0;
    }

    # Debug

    if ($dbg2) {
	print(STDERR "$subname: string: $string, expanded_string: $expanded_string\n");
    }

    # Return

    return($return_val, $expanded_string);
}



#---------------------------------------------------------------------
# Subroutine: printElapsedTime
#
# Usage:      printElapsedTime($ustart, $uend)
#
# Function:   Print the elapsed time between the end and start times
#
# Input:      $ustart        UNIX start time
#             $uend          UNIX end time
#
# Output:     prints the elapsed time to STDERR
# 

sub printElapsedTime {
    local ($ustart, $uend) = @_;
    
    local($hours, $mins, $secs, $elapsed_secs);

    $hours = int(($uend - $ustart)/(60*60));
    $mins = int(($uend - $ustart - $hours*60*60)/60);
    $secs = int(($uend - $ustart - $hours*60*60 - $mins*60));
    $elapsed_secs = $uend-$ustart;

    print(STDERR "Ended at unixtime: $uend\n");
    print(STDERR "Elapsed secs: $elapsed_secs\n");
    printf(STDERR "\t%02d:%02d:%02d\n", $hours, $mins, $secs);
}

#------------------------------------- EOF ------------------------------
