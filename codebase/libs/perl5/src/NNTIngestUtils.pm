##---------------------------------------------------------------------------##
# CVS ID:     $Id: NNTIngestUtils.pm,v 1.1 2010/02/05 05:24:21 jaimi Exp $
# CVS SOURCE: $Source: /cvs/libs/perl5/src/NNTIngestUtils.pm,v $
##---------------------------------------------------------------------------##
##                                                                           ##
##  Copyright UCAR (c) 2007.                                                 ##
##  University Corporation for Atmospheric Research (UCAR),                  ##
##  National Center for Atmospheric Research (NCAR),                         ##
##  Research Applications Laboratory (RAL),                                  ##
##  P.O. Box 3000, Boulder, Colorado, 80307-3000, USA.                       ##
##                                                                           ##
##---------------------------------------------------------------------------##
#==============================================================================
#
# Module:  IngestUtils.pm
#
# Purpose: To provide reusable PERL subroutines for the MDS ingesting scripts.
#
# Author:  Shane Swartz
#
# Change History:
#==============================================================================
# 20070323 : Shane Swartz : Created
#------------------------------------------------------------------------------
#==============================================================================
#
# Set main package
#
package IngestUtils;
#
# Include files and pragmas
#
   require Exporter;
   our @ISA         = qw(Exporter);
   our @EXPORT      = qw(help putLogEntry runSystemCommand isInt isFloat
                         isValidFileName isValidDirName showVersion
                         $SCRIPT $DEBUG $USAGE $SUCCESS_EXIT_VALUE
			 $SCRIPT_VERSION $SCRIPT_VERSION_DATE
			 $SCRIPT_CREATED_YEAR $SCRIPT_LAST_MOD_YEAR
			 $ERROR_EXIT_VALUE %LOG_TYPES
			);
   our @EXPORT_OK   = qw(decodeXMLEntry moveFile putDirectory setCycle
                         getFileViaWeb localSignalHandler $scriptCanBeStopped 
			 $ctrlcAlready $ctrlzAlready
			);
   our %EXPORT_TAGS = (signalHandler => [qw(localSignalHandler
                                            $scriptCanBeStopped $ctrlcAlready
					    $ctrlzAlready)]
                      ); 

   our @EXPORT_FAIL = qw(setCopyright onTTY);
   our $VERSION;
#
# Get the needed PERL modules
#
   use strict;
   use IO::File;
   use IO::Handle;
   use POSIX;
   use Time::Local;
   use File::Basename;
#
# Declare global variables
#
   our $SCRIPT;
   our $USAGE;
   our $DEBUG;
   our $SUCCESS_EXIT_VALUE;
   our $ERROR_EXIT_VALUE;
   our $SCRIPT_VERSION;
   our $SCRIPT_VERSION_DATE;
   our $SCRIPT_CREATED_YEAR;
   our $SCRIPT_LAST_MOD_YEAR;
   our $scriptCanBeStopped;
   our $ctrlcAlready;
   our $ctrlzAlready;
   our %LOG_TYPES;
#
# Declare local variables
#
   my  @B_SHELLS;
   my  @C_SHELLS;
#
# Declare subroutines
#
   sub decodeXMLEntry;
   sub help;
   sub moveFile;
   sub putDirectory;
   sub putLogEntry;
   sub runSystemCommand;
   sub setCopyright;
   sub setCycle;
   sub showVersion;
   sub isInt;
   sub isFloat;
   sub isValidFileName;
   sub isValidDirName;
   sub onTTY;
   sub localSignalHandler;
   sub getFileViaWeb;
#
# Set auto flushing of standard out and standard error
#
   STDOUT->autoflush(1);
   STDERR->autoflush(1);
#
# Initialize variables
#
# Version and version date from CVS
#
   $VERSION = q$Revision: 1.1 $;
   $VERSION =~ s/Revision: (\d+\.\d+)/$1/;
   $VERSION =~ s/\s+$//;

   %LOG_TYPES = ( 'error'   => 'ERROR',
		  'info'    => 'INFO',
		  'debug'   => 'DEBUG',
		  'warning' => 'WARNING' );
#
# The following exit values are set for UNIX/LINUX shell
# success and failure interpretation
#
   $SUCCESS_EXIT_VALUE = 0;
   $ERROR_EXIT_VALUE   = 1;
#
   @B_SHELLS = ('ash', 'bash', 'ksh', 'sash', 'sh', 'zsh'); 
   @C_SHELLS = ('csh', 'tcsh'); 
#
#=============================================================================#
#                                                                             #
########################## S U B R O U T I N E S ##############################
#                                                                             #
#=============================================================================#
#
# Subroutine putDirectory - Checks for existence of a given directory.  Creates
#                           it if it does not exist and sets the permissions
#                           and ownership.
sub putDirectory {
#
# Global variables used
# ================================
#
#  %LOG_TYPES
#  $ERROR_EXIT_VALUE
#
# ================================
#
# Subroutines or PERL built-in functions called
# =============================================
#
#  putLogEntry
#  getpwnam
#  getgrnam
#  mkdir
#  chown
#  chmod
#
# =============================================
#
# Declare local variables
#
   my  $NUM_REQUIRED_ARGS;
   my  $dirToCheck;
   my  $dirOwnerName;
   my  $dirGroupName;
#   my  $dirOwnerUID;
#   my  $dirGroupGID;
   my  $logMessage;
#
# Initialize variables
#
   $NUM_REQUIRED_ARGS = 3;
   if ( scalar(@_) == $NUM_REQUIRED_ARGS ) {
      ($dirToCheck, $dirOwnerName, $dirGroupName) = @_;
   }
   else {
      $logMessage = "Invalid number of arguments passed to subroutine ".
                    "<putDirectory>";
      putLogEntry($LOG_TYPES{'error'}, \$logMessage, $ERROR_EXIT_VALUE);
   }
#
# Get the UID and GID for the given user name and group name
#
#   $dirOwnerUID = (getpwnam("$dirOwnerName"))[2];
#   $dirGroupGID = (getgrnam("$dirGroupName"))[2];
#
# Verify that the directory exists.  If not, create it.
#
   if ( ! -d $dirToCheck ) {
      print "Making dir $dirToCheck\n";
      if (mkdir($dirToCheck)) {
         if ( chmod(0775, $dirToCheck) ) {
#            if ( defined($dirOwnerUID) &&
#                 defined($dirGroupGID) ) {
#               if ( ! chown($dirOwnerUID, $dirGroupGID, $dirToCheck) ) {
#                  $logMessage = "Failed to change owner and group for the ".
#                                "local directory <$dirToCheck>";
#                  putLogEntry($LOG_TYPES{'error'}, \$logMessage,
#                              $ERROR_EXIT_VALUE);
#               }
#            }
#            else {
#               $logMessage = "Failed to get the uid and gid to set the ".
#                             "ownership and group for the local directory ".
#                             "<$dirToCheck>";
#               putLogEntry($LOG_TYPES{'error'}, \$logMessage,
#                              $ERROR_EXIT_VALUE);
#            }
#         }
#         else {
#            $logMessage = "Failed to change permissions on the local ".
#                          "directory <$dirToCheck>";
#            putLogEntry($LOG_TYPES{'error'}, \$logMessage, $ERROR_EXIT_VALUE);
         }
      }
      else {
         $logMessage = "Failed to create the directory <$dirToCheck>";
         putLogEntry($LOG_TYPES{'error'}, \$logMessage, $ERROR_EXIT_VALUE);
      }
   }
   elsif ( ! -x $dirToCheck || ! -w $dirToCheck ) {
      $logMessage = "Directory <$dirToCheck> is not accessible or ".
                    "writable";
      putLogEntry($LOG_TYPES{'error'}, \$logMessage, $ERROR_EXIT_VALUE);
   }
}
#
# Sub putLogEntry - Prints an entry to STDOUT or STDERR, both of which are
#                   considered the log
#
sub putLogEntry {
#
# Global variables used
#==========================
#
# %LOG_TYPES 
# $ERROR_EXIT_VALUE
# $SCRIPT
#
#==========================
#
# Subroutines called
#==========================
#
# "strftime" from the module POSIX
#
#==========================
#
# Declare local variables
#
# Declare private variables
#
   my  $logType;               # Type of entry (INFO, DEBUG, ERROR, WARNING)
   my  $refLogMessage;         # Reference to the log message
   my  $logMessage;            # Message to print to the log
   my  $MIN_NUM_ARGS;          # Minimum number of required arguments
   my  $MAX_NUM_ARGS;          # Maximum number of required arguments
   my  $numArgs;               # Number of arguments passed to this subroutine
   my  $currentDateTime;       # Current date and time
   my  $errorExitValue;        # Exit value to use when the log entry type
                               # is ERROR 
#
# Initialize variables
#
   $currentDateTime = strftime("%b %d %Y %H:%M:%S GMT", gmtime(time()));
   $MIN_NUM_ARGS = 2;
   $MAX_NUM_ARGS = 3;
   $numArgs = scalar(@_);
#
   if ( ! defined($SCRIPT) || length($SCRIPT) == 0 ) {
      $SCRIPT = 'NULL_SCRIPT_NAME';
   }
#
   if ( $numArgs >= $MIN_NUM_ARGS && $numArgs <= $MAX_NUM_ARGS ) {
      ($logType, $refLogMessage, $errorExitValue) = @_;
   }
   else {
      $logMessage = "Invalid number of arguments passed to subroutine ".
                     "<putLogEntry>";
      STDERR->printf ("%s: %23s: %-7s: %s - EXITING!\n",
                      $SCRIPT, $currentDateTime, $LOG_TYPES{'error'},
		      $logMessage);
      exit $ERROR_EXIT_VALUE;
   }
#
# Verify that the reference to the logMessage is a reference and of the 
# correct type
#
   if ( ref($refLogMessage) && ref($refLogMessage) eq 'SCALAR' ) {
      $logMessage = $$refLogMessage;
   }
   else {
      $logMessage = "Invalid argument for the log message passed to ".
                    "the subroutine <putLogEntry>";
      STDERR->printf ("%s: %23s: %-7s: %s - EXITING!\n",
		      $SCRIPT, $currentDateTime, $LOG_TYPES{'error'},
		      $logMessage);
      exit $ERROR_EXIT_VALUE
   }
#
# Convert the log type to upper case
#
   $logType =~ s/\w/\u$&/g;
#
# Print the message to STDOUT if it is not of type "ERROR",
# otherwise print to STDERR
# Both STDOUT and STDERR are considered to be the log file
# Exit with the errorExitValue if the log type is "ERROR"
#
   if ( $logType ne "$LOG_TYPES{'error'}" ) {
      STDOUT->printf ("%s: %23s: %-7s: %s\n", $SCRIPT,
                      $currentDateTime, $logType, $logMessage);
   }
   else {
      STDERR->printf ("%s: %23s: %-7s: %s - EXITING!\n", $SCRIPT,
                      $currentDateTime, $logType, $logMessage);
      exit ($errorExitValue);
   }
}
#
# Subroutine runSystemCommand - Run a requested system command using the 
#                               builtin PERL system call and traps the
#                               return status
sub runSystemCommand {
#
# Global variables used
#===================================
#
# %LOG_TYPES
# $ERROR_EXIT_VALUE
#
#===================================
#
# Subroutines or PERL functions called
#===================================
#
# putLogEntry
# "basename" from the module Filename::Basename
# system
#
#===================================
#
#
# Declare variables
#
   my $refCommand;             # Reference to the command to run
   my $command;                # Command to run using the PERL system call
   my $logMessage;             # Log message to print if command fails
   my $NUM_REQUIRED_ARGS;      # Required number of arguments
   my $exitValue;              # Exit value from using the command "system"
   my $shell;                  # User's shell
   my $redirectFound;          # Boolean variable used to state if the
                               # incoming command to run contains a redirect
#
# Initialize variables
#
   $NUM_REQUIRED_ARGS = 1;
   if ( scalar(@_) == $NUM_REQUIRED_ARGS ) {
      ($refCommand) = @_; 
#
# Verify that the refCommand and refLogMessage are references of the 
# correct type
#
      if ( ref($refCommand) && ref($refCommand) eq 'SCALAR' ) {
         $command = $$refCommand;
      }
      else {
         $logMessage = "Invalid argument for the command to run passed to ".
		       "the subroutine <runSystemCommand>";
         putLogEntry($LOG_TYPES{'error'}, \$logMessage, $ERROR_EXIT_VALUE);
      }
   }
   else {
      $logMessage = "Incorrect number of arguments provided to the ".
                    "subroutine <runSystemCommand>";
      putLogEntry($LOG_TYPES{'error'}, \$logMessage, $ERROR_EXIT_VALUE);
   } 
#
# Run the system command
#
   $exitValue = -999;
   if ( $command =~ /\>/ ) {
      $redirectFound = 1;
   }
   else {
      $redirectFound = 0;
   }
   if ( defined ($ENV{'SHELL'}) && ! $redirectFound ) {
      if ( $DEBUG ) {
         $logMessage = "Running the system command <$command> ".
	               "with added redirects";
         putLogEntry($LOG_TYPES{'debug'}, \$logMessage);
      }
      $shell = basename($ENV{'SHELL'});
      if ( scalar(grep(/$shell/, @C_SHELLS)) > 0 ) {
         system ("$command >& /dev/null");
      }
      elsif ( scalar(grep(/$shell/, @B_SHELLS)) > 0 ) {
         system ("$command > /dev/null 2>&1");
      }
   }
   else {
      if ( $DEBUG ) {
         $logMessage = "Running the system command <$command> ".
	               "with specified redirects";
         putLogEntry($LOG_TYPES{'debug'}, \$logMessage);
      }
      system ("$command");
   }
   ( WIFEXITED($?) ) ? $exitValue  = WEXITSTATUS($?) : $exitValue  = $? >> 8;
   if ( $DEBUG ) {
      $logMessage = "Finished running the system command - ".
                    "exit value <$exitValue>";
      putLogEntry($LOG_TYPES{'debug'}, \$logMessage);
   }
   if ( $exitValue != 0 ) { 
      $logMessage = "Exit value <$exitValue> from running ".
                    "command <$command>";
      putLogEntry($LOG_TYPES{'warning'}, \$logMessage);
      return 0;
   }
   return 1;
}
#
# Subroutine setCycle - Gets the cycle time to download based on the current
#                       time and a delay offset time
sub setCycle {
#
# Global variables used
#===================================
#
# %LOG_TYPES
# $ERROR_EXIT_VALUE
#
#===================================
#
# Subroutines called or PERL builtin functions called
#===================================
#
# putLogEntry
# gmtime
# "timegm" from the module Time::Local
#
#===================================
#
# Declare local variables
#
   my $NUM_REQUIRED_ARGS;
   my $cycle;  
   my $cycleInterval;
   my $firstDailyCycle;
   my $cycleTime;  
   my $requestHour;
   my $requestHourInt;
   my $SECS_PER_HOUR;
   my $requestedDateBeginTime;
   my $downloadOffsetHours;
   my $year;       # Expects 4 digit year
   my $month;      # Expects month number in range 1-12
   my $monthDay;   # Expects day in the month in the range 1-$DAYS_IN_MONTH{$month}
   my $sec;
   my $min;
   my $hour;
   my $start;
   my $stop;
   my $index;
   my $logMessage;
   my $cycleMonthDay;
   my $cycleMonth;
   my $cycleYear;
   my $cycleYearMonthDay;
   my $timeNow;
   my $dataPullDateTime;
   my @nonAdjustedCycles;
   my @adjustedCycles;
   my $midnightRequestTime;
#
# Initialize variables
#
   $NUM_REQUIRED_ARGS = 9;
   if ( scalar(@_) == $NUM_REQUIRED_ARGS ) {
      ($year, $month, $monthDay, $requestHour, $downloadOffsetHours,
       $cycleInterval, $firstDailyCycle, $timeNow, $dataPullDateTime) = @_;
   }
   else {
      $logMessage = "Invalid number of arguments passed to subroutine ".
                    "<setCycle>";
      putLogEntry($LOG_TYPES{'error'}, \$logMessage, $ERROR_EXIT_VALUE);
   }
   $SECS_PER_HOUR = 3600;
   $sec           = 0;
   $min           = 0;
#
# Get the epoch time (GM-based) for the start of the requested date and
# hour (MM:SS = 00:00)
# Note: The month is set back one day to use the range 0-11 and the year
#       is set back by 1900.  Both must be done to provide timegm the
#       expected values.
#
   $requestedDateBeginTime = timegm($sec, $min, $requestHour, $monthDay,
                                    $month-1, $year-1900); 
#
# Verify that this is a valid date and time
#
   if ( $requestedDateBeginTime > $timeNow ) {
      $logMessage = "Requested forecast date and time <$dataPullDateTime> ".
                    "is in the future";
      putLogEntry($LOG_TYPES{'error'}, \$logMessage, $ERROR_EXIT_VALUE);
   }
#
# Determine if the value of the variable downloadOffsetTime needs to be set to 
# zero.  This only happens when the difference between the requested pull
# date and time and current time is greater than the downloadOffsetTime.
#
   if ( $downloadOffsetHours != 0 ) {
      if ( ($timeNow - $requestedDateBeginTime) >
           $downloadOffsetHours*$SECS_PER_HOUR ) {
         $logMessage = "Resetting configuration variable DownloadOffsetHours ".
                       "to zero for calculating the cycle to use since the ".
                       "requested forecast date and time is far enough in ".
                       "the past to have the data available";
         putLogEntry($LOG_TYPES{'info'}, \$logMessage);
         $downloadOffsetHours = 0;
      }
   }

#
# The code in the following section works only for calculating the 
# appropriate cycle when the first daily cycle is zero.
#------------------------------------------------------------------------------
# Calculate the cycle to use
#
#  $cycleTime = $requestedDateBeginTime -
#               ($downloadOffsetHours * $SECS_PER_HOUR);
#  while ( ($cycleTime / $SECS_PER_HOUR) % $cycleInterval != 0) {
#     $cycleTime -= $SECS_PER_HOUR;
#  }
#------------------------------------------------------------------------------
#
# The code in the following sections works when the first daily cycle
# is any value.  This is the code that will be used for the MDS data gathering.
#------------------------------------------------------------------------------
#
# Load the array holding the cycle numbers with no adjustments for delay and
# a non-zero first daily cycle
#
   for ($hour=0; $hour<24; $hour++) {
      $cycle = int($hour / $cycleInterval)*$cycleInterval + $firstDailyCycle;
      push(@nonAdjustedCycles, $cycle);
      if ( $DEBUG ) {
         $logMessage = "Hour <$hour> - not adjusted cycle <$cycle>";
         putLogEntry($LOG_TYPES{'debug'}, \$logMessage);
      }
   }
#
# Load the array holding the cycles numbers adjusted for delay and a
# non-zero first daily cycle.  This, also,  works when the first daily
# cycle is zero.
#
   $start = 24 - $firstDailyCycle - $downloadOffsetHours;
   for ($index=$start; $index<24; $index++ ) {
      push(@adjustedCycles, $nonAdjustedCycles[$index]);
   }
#
   $stop = $start;
   for ($index=0; $index<$start; $index++ ) {
      push(@adjustedCycles, $nonAdjustedCycles[$index]);
   }
#
   if ( $DEBUG ) {
      for ($hour=0; $hour<24; $hour++) {
         $logMessage = "Hour <$hour> - adjusted cycle <$adjustedCycles[$hour]>";
         putLogEntry($LOG_TYPES{'debug'}, \$logMessage);
      }
   }
#
# Get the epic time for midnight of the requested Date.  This can be any
# time from 00:00:00 to 00:59:59.  The script is only concerned with the hour.
#
   $midnightRequestTime = $requestedDateBeginTime - 
                          ($requestHour * $SECS_PER_HOUR);
#
   if ( $DEBUG ) {
      $logMessage = "requestHour <$requestHour> - adjustedCycle ".
                    "<$adjustedCycles[$requestHour]>";
      putLogEntry($LOG_TYPES{'debug'}, \$logMessage);
   }
#
   $requestHourInt = int("$requestHour");
   if ( $requestHourInt >= int("$adjustedCycles[$requestHourInt]") ) {
      $cycleTime = $midnightRequestTime +
                   $adjustedCycles[$requestHourInt] * $SECS_PER_HOUR;
      if ( $DEBUG ) {
         $logMessage = "Getting cycle - loop part 1 - midnightRequestTime ".
                       "<$midnightRequestTime> - cycleTime <$cycleTime>";
         putLogEntry($LOG_TYPES{'debug'}, \$logMessage);
      }
   }
   else {
      $cycleTime = $midnightRequestTime -
                   (24 - $adjustedCycles[$requestHourInt]) * $SECS_PER_HOUR;
      if ( $DEBUG ) {
         $logMessage = "Getting cycle - loop part 2 - midnightRequestTime ".
                       "<$midnightRequestTime> - cycleTime <$cycleTime>";
         putLogEntry($LOG_TYPES{'debug'}, \$logMessage);
      }
   } 
#
#------------------------------------------------------------------------------
#   
# Calculate the cycle and cycle date information based on the 
# calculated cycle time
#
   ($cycle, $cycleMonthDay, $cycleMonth, $cycleYear) =
      (gmtime($cycleTime))[2,3,4,5];
#
   $cycleMonth       += 1;
   $cycleYear        += 1900;
   $cycleMonth        = '0' . $cycleMonth    if ( $cycleMonth < 10 );
   $cycleMonthDay     = '0' . $cycleMonthDay if ( $cycleMonthDay < 10 );
   $cycleYearMonthDay = "${cycleYear}${cycleMonth}${cycleMonthDay}";
#
# Return the cycle and cycleYearMonthDay
#
   return ($cycle, $cycleYearMonthDay);
}
#
# Subroutine decodeXMLEntry - Decodes a given XML entry that is contained
#                             as an array 
sub decodeXMLEntry {
#
# Global variables used
# =====================================
#
# %LOG_TYPES
# $ERROR_EXIT_VALUE
#
# =====================================
#
# Declare local variables
#
   my $logMessage;
   my $NUM_REQUIRED_ARGS;
   my $expectedRefType;
   my $returnRefType;
   my $refTmpHash;
   my %tmpHash;
   my $hashKey;
   my $refHashValue;
   my $EMPTY_STRING;
   my @VALID_RETURN_TYPES;
   my $arrayValue;
   my $modelToUse;
   my $refType;
#
# Initialize variables
#
   $EMPTY_STRING      = '';
   $NUM_REQUIRED_ARGS = 4;
   if ( scalar(@_) == $NUM_REQUIRED_ARGS ) {
      ($refTmpHash, $hashKey, $expectedRefType, $modelToUse) = @_;
   }
   else {
      $logMessage = "Invalid number of arguments passed to subroutine ".
                    "<decodeXMLEntry>";
      putLogEntry($LOG_TYPES{'error'}, \$logMessage, $ERROR_EXIT_VALUE);
   }
   @VALID_RETURN_TYPES = ('SCALAR', 'ARRAY');
#
# Verify that the provided reference to the XMLConfig HASH information is valid
#
   if ( ref($refTmpHash) && ref($refTmpHash) eq 'HASH' ) {
      %tmpHash = %$refTmpHash;
   }
   else {
      if ( $DEBUG ) {
         $refType = ref($refTmpHash);
         $logMessage = "Reference type <$refType> for refTmpHash";
         putLogEntry($LOG_TYPES{'debug'}, \$logMessage);
      }
      $logMessage = "Invalid reference type provided to the subroutine ".
                    "<decodeXMLEntry> for the XML configuration information";
      putLogEntry($LOG_TYPES{'error'}, \$logMessage, $ERROR_EXIT_VALUE);
   }
#
# Convert the expected return type to upper case
#
   $expectedRefType =~ s/\w/\u$&/g;
#
# Verify that the expected return type is valid
#
   if ( scalar(grep(/$expectedRefType/, @VALID_RETURN_TYPES)) == 0 ) {
      $logMessage = "Invalid return type <$expectedRefType> requested ".
                    "from the subroutine <decodeXMLEntry>";
      putLogEntry($LOG_TYPES{'error'}, \$logMessage, $ERROR_EXIT_VALUE);
   }
#
# Obtain the reference to the value(s) for the requested data
#
   $refHashValue = $tmpHash{"$hashKey"};
#
# The refHashValue should be a reference to an ARRAY
#
   if ( $DEBUG ) {
      if ( defined($refHashValue) ) {
         $refType = ref($refHashValue);
         $logMessage = "Reference type <$refType> for hashKey <$hashKey>";
      }
      else {
         $logMessage = "HashKey <$hashKey> does not provide a valid ".
                       "reference - refHashValue <$refHashValue>";
         putLogEntry($LOG_TYPES{'debug'}, \$logMessage);
         my $key;
         foreach $key (keys(%tmpHash)) {
            $logMessage = "tmpHash key <$key>";
            putLogEntry($LOG_TYPES{'debug'}, \$logMessage);
         }
      }
   }

   if ( ref($refHashValue) && ref($refHashValue) eq 'ARRAY' ) {
      if ( $expectedRefType eq 'SCALAR' ) {
         if ( $DEBUG ) {
	    if ( defined(@$refHashValue[0]) ) {
               $logMessage = "$hashKey <@$refHashValue[0]> for ".
                             "model <$modelToUse>";
            }
            else {
               $logMessage = "$hashKey <> for ".
                             "model <$modelToUse>";
	    }
            putLogEntry($LOG_TYPES{'debug'}, \$logMessage);
         }
	 if ( ! ref(@$refHashValue[0]) ) {
            return (@$refHashValue[0]);
         }
         else {
	    $returnRefType = ref(@$refHashValue[0]);
	    if ( $DEBUG ) {
	       if ( $returnRefType eq 'HASH' ) {
                  $logMessage = "Invalid value in the XML configuration - ".
		                "Expected reference type <$expectedRefType> ".
	                        "but got reference type <$returnRefType> ".
	                        "for $hashKey <@$refHashValue[0]> for ".
                                "model <$modelToUse>";
                  putLogEntry($LOG_TYPES{'error'}, \$logMessage,
		              $ERROR_EXIT_VALUE);
                  return ("$EMPTY_STRING");
               }
	    }
	 }
      }
      elsif ( $expectedRefType eq 'ARRAY' ) {
         if ( $DEBUG ) {
            $logMessage = "$hashKey <@$refHashValue> for ".
                          "model <$modelToUse>";
            putLogEntry($LOG_TYPES{'debug'}, \$logMessage);
         }
#
# Verify that no value in the array is a reference
# Print an error message and stop if any values are invalid
#
         foreach $arrayValue (@$refHashValue) {
	    if ( ref($arrayValue) ) {
	       $logMessage = "Invalid array value <$arrayValue> for $hashKey ".
	                     "in the XML configuration";
               putLogEntry($LOG_TYPES{'error'}, \$logMessage,
	                   $ERROR_EXIT_VALUE);
	    }
	 }
#
         return (@$refHashValue);
      }
   }
   else {
      $logMessage = "Invalid XML configuration value type for requested ".
                    "data type <$hashKey>.  Expected the raw XML input to ".
		    "be of variable type <ARRAY>";
      putLogEntry($LOG_TYPES{'error'}, \$logMessage, $ERROR_EXIT_VALUE);
   }
}
#
# Subroutine moveFile - Moves the requested file from the current
#                       directory to another directory and uses
#                       a temporary file name in the target directory
#                       as the file to which to move before then
#                       renaming it in the new directory.
sub moveFile {
#
#==============================================================================
#
# Usage:  1) $booleanVar = moveFile($currentDir, $currentFileName,
#                                   $targetDir, $targetFileName);
#         or 
#
#         2) $booleanVar = moveFile($currentFileName, $targetFileName);
#
# Returns: Scalar value 0=failed to move file, 1=Successfully moved file
#
#==============================================================================
#
# Include files and pragmas
#
   use File::Temp qw(:mktemp);
#
# Global variables used
#==========================
#
# %LOG_TYPES 
# $ERROR_EXIT_VALUE
# $DEBUG
#
#==========================
#
# Declare variables
#
   my $tempFileName;
   my $templateFileName;
   my $MIN_NUM_REQUIRED_VARS;
   my $MAX_NUM_REQUIRED_VARS;
   my $numArgs;
   my $currentDir;
   my $currentFileName;
   my $targetDir;
   my $targetFileName;
   my $finalTargetFileName;
   my $currentFileNameToMove;
   my $commandToRun;
   my $returnValue;
   my $template;
   my $MV_CMD;
   my $logMessage;
#
# Initialize variables
#
   $MIN_NUM_REQUIRED_VARS = 2;
   $MAX_NUM_REQUIRED_VARS = 4;
   $numArgs = scalar(@_);
#
   if ( $numArgs == $MIN_NUM_REQUIRED_VARS ||
        $numArgs == $MAX_NUM_REQUIRED_VARS ) {
      if ( $numArgs == $MIN_NUM_REQUIRED_VARS ) {
         ($currentFileName, $targetFileName) = @_;
	 $currentDir            = dirname($currentFileName);
	 $currentFileNameToMove = basename($currentFileName);
	 $targetDir             = dirname($targetFileName);
	 $finalTargetFileName   = basename($targetFileName);
      }
      elsif ( $numArgs == $MAX_NUM_REQUIRED_VARS ) {
         ($currentDir, $currentFileName, $targetDir, $targetFileName) = @_;
	 $currentFileNameToMove = $currentFileName;
      }
   }
   else {
      $logMessage = "Incorrect number of arguments provided to the ".
                    "subroutine <moveFile>";
      putLogEntry($LOG_TYPES{'error'}, \$logMessage, $ERROR_EXIT_VALUE);
   }
#
   $MV_CMD           = '/bin/mv';
   $templateFileName = 'tempFile.XXXXXXXX';
   $template         = "${targetDir}/${templateFileName}";
#
# First move the current file to a temporary file name in the 
# requested target directory
#
   $tempFileName = mktemp($template);
   if ( ! -e $tempFileName ) {
      if ( $DEBUG ) {
         $logMessage = "Attempting to move file ".
	               "<${currentDir}/${currentFileNameToMove}> ".
		       "to temporary file <$tempFileName>";
         putLogEntry($LOG_TYPES{'debug'}, \$logMessage);
      }
      $commandToRun = "$MV_CMD ${currentDir}/${currentFileNameToMove} ".
                      "$tempFileName";
      $returnValue  = runSystemCommand(\$commandToRun); 
      if ( $returnValue != 1 ) {
         $logMessage = "Failed to move the file ".
	               "<${currentDir}/${currentFileNameToMove}> to the ".
		       "temporary file <$tempFileName>";
         putLogEntry($LOG_TYPES{'warning'}, \$logMessage);
         return 0;
      }
      elsif ( $DEBUG ) {
         $logMessage = "Successfully moved file ".
	               "<${currentDir}/${currentFileNameToMove}> ".
		       "to temporary file <$tempFileName>";
         putLogEntry($LOG_TYPES{'debug'}, \$logMessage);
      }
   }
   else {
      $logMessage = "Failed to move the file ".
                    "<${currentDir}/${currentFileNameToMove}> ".
		    "because the temporary file <$tempFileName> ".
		    "already exists";
      putLogEntry($LOG_TYPES{'warning'}, \$logMessage);
      return 0;
   }
#
# Rename the temporary file to the target file name
#
   $commandToRun = "$MV_CMD $tempFileName ".
                   "${targetDir}/${finalTargetFileName}";
   $returnValue  = runSystemCommand(\$commandToRun); 
   if ( $returnValue == 1 ) {
      if ( ! chmod(0664, "${targetDir}/${finalTargetFileName}") ) {
         $logMessage = "Failed to set permissions on the file ".
	               "<${targetDir}/${finalTargetFileName}> to 0664";
	 putLogEntry($LOG_TYPES{'warning'}, \$logMessage);
      }
      if ( $DEBUG ) {
         $logMessage = "Successfully renamed the temporary file ".
		       "<$tempFileName> to the target file ".
		       "<${targetDir}/${finalTargetFileName}>";
         putLogEntry($LOG_TYPES{'debug'}, \$logMessage);
      }
      return 1;
   }
   else {
      $logMessage = "Failed to move the temporary file <$tempFileName> ".
                    "to the target file <${targetDir}/${finalTargetFileName}";
      putLogEntry($LOG_TYPES{'warning'}, \$logMessage);
      return 0;
   }
}
#
# Subroutine help - Prints the usage information
#
sub help {
#
# Global variables used
#
#===============================
#
# $USAGE
# $SCRIPT
# $SUCCESS_EXIT_VALUE
# $SCRIPT_VERSION
# $SCRIPT_VERSION_DATE
#
#===============================
#
# Declare variables
#
   my $copyright;
#
# Get the copyright
#
   $copyright = setCopyright;
#
# Show the help information is on a terminal
#
   if ( onTTY ) {
#
# Print the help information
#
      if ( (defined($USAGE) && length($USAGE) > 0) &&
           (defined($SCRIPT_VERSION) && length($SCRIPT_VERSION) > 0) &&
   	   (defined($SCRIPT_VERSION_DATE) && length($SCRIPT_VERSION_DATE) > 0) &&
	   (defined($copyright) && length($copyright) > 0) &&
	   (defined($SCRIPT) && length($SCRIPT) > 0) ) {
         STDOUT->print ("\n\*\* Script: $SCRIPT, ".
                        "Version $SCRIPT_VERSION dated ".
                        "$SCRIPT_VERSION_DATE\n".
                        "$copyright\n\n".
		        "$USAGE\n\n");
      }
      else {
         if ( defined($SCRIPT) && length($SCRIPT) == 0 ) {
            STDERR->print ("\n$SCRIPT: ERROR! Usage, ".
                           "\tSCRIPT_VERSION, SCRIPT_VERSION_DATE, or ".
                           "\tcopyright Not defined\n\n");
         }
         else {
            STDERR->print ("\nERROR! SCRIPT variable not defined and ".
                           "\n       possibly USAGE, SCRIPT_VERSION, ".
			   "\n       SCRIPT_VERSION_DATE, or copyright ".
			   "\n       not defined\n\n");
         }
         exit ($ERROR_EXIT_VALUE);
      }
   }
   exit ($SUCCESS_EXIT_VALUE);
}
#
# Subroutine showVersion - Prints the version information
#
sub showVersion {
#
# Global variables used
#
#===============================
#
#  $SCRIPT
#  $SUCCESS_EXIT_VALUE
#  $SCRIPT_VERSION
#  $SCRIPT_VERSION_DATE
#  $COPYRIGHT
#
#===============================
#
# Declare variables
#
   my $copyright;
#
# Get the copyright
#
   $copyright = setCopyright;
#
# Show the help information is on a terminal
#
   if ( onTTY ) {
      if ( (defined($SCRIPT_VERSION) && length($SCRIPT_VERSION) > 0 ) &&
	   (defined($SCRIPT_VERSION_DATE) && length($SCRIPT_VERSION_DATE) > 0) &&
	   (defined($copyright) && length($copyright) > 0) &&
	   (defined($SCRIPT) && length($SCRIPT) > 0) ) {
         STDOUT->print ("\n\*\* Script: $SCRIPT, ".
                        "Version $SCRIPT_VERSION dated ".
                        "$SCRIPT_VERSION_DATE\n\n".
                        "$copyright\n\n");
      }
      else {
         if ( defined($SCRIPT) && length($SCRIPT) == 0 ) {
            STDERR->print ("\n$SCRIPT: ERROR! SCRIPT_VERSION, ".
	                   "\n\tSCRIPT_VERSION_DATE, or ".
	   	           "\n\tcopyright not defined\n\n");
         }
         else {
            STDERR->print ("\nERROR! SCRIPT variable not defined and ".
	                   "\n\tpossibly SCRIPT_VERSION, ".
			   "\n\tSCRIPT_VERSION_DATE, ".
			   "\n\tor copyright not defined\n\n");
         }
         exit ($ERROR_EXIT_VALUE);
      }
   }
   exit ($SUCCESS_EXIT_VALUE);
}
#
# Subroutine setCopyright - Sets the value for the COPYRIGHT variable
#
sub setCopyright {
#
# Global variables used
#
#===============================
#
# $SCRIPT
# $SCRIPT_CREATED_YEAR
# $SCRIPT_LAST_MOD_YEAR
# %LOG_TYPES
# $ERROR_EXIT_VALUE
#
#===============================
#
# Declare local variables
#
   my $copyright;
   my $copyrightBottom;
   my $logMessage;
#
# Initialize variables
#
   $copyright = '';
   $copyrightBottom = "** University Corporation for Atmospheric Research (UCAR),\n".
	   	      "** National Center for Atmospheric Research (NCAR),\n".
		      "** Research Applications Laboratory (RAL),\n".
		      "** P.O. Box 3000, Boulder, Colorado, 80307-3000, USA";
#
# Create the value for the variable "copyright"
#
   if ( defined($SCRIPT_LAST_MOD_YEAR) && length($SCRIPT_LAST_MOD_YEAR) > 0 ) {
      if ( defined($SCRIPT_CREATED_YEAR) && length($SCRIPT_CREATED_YEAR) > 0 ) {
         if ( $SCRIPT_LAST_MOD_YEAR > $SCRIPT_CREATED_YEAR ) {
            $copyright = "** Copyright UCAR (c) ${SCRIPT_CREATED_YEAR}-".
	                 "${SCRIPT_LAST_MOD_YEAR}\n";
         }
         else {
            $copyright = "** Copyright UCAR (c) ${SCRIPT_LAST_MOD_YEAR}\n";
         }
         $copyright .= "$copyrightBottom";
      }
      else {
         $copyright = "** Copyright UCAR (c) ${SCRIPT_LAST_MOD_YEAR}\n".
                      "$copyrightBottom";
      }
   }
   else {
      $logMessage = "Subroutine setCopyright: The variable ".
                    "[SCRIPT_LAST_MOD_YEAR] is not defined";
      putLogEntry($LOG_TYPES{'error'}, \$logMessage, $ERROR_EXIT_VALUE);
   }
#
# Return the value of the copyright
#
   return $copyright;
}
#
# Subroutine isInt - Verifies that the passed argument is an integer
#
sub isInt {
#
# Global variables used
#============================
#
#  $ERROR_EXIT_VALUE;
#  %LOG_TYPES;
#
#============================
#
# Declare variables
#
   my $NUM_REQUIRED_ARGS;
   my $numToChk;
   my $logMessage;
#
# Initialize variables
#
   $NUM_REQUIRED_ARGS = 1;
   if ( scalar(@_) == $NUM_REQUIRED_ARGS ) {
      ($numToChk) = @_;
   }
   else {
      $logMessage = "Invalid number of arguments passed to subroutine ".
                    "<isInt>";
      putLogEntry($LOG_TYPES{'error'}, \$logMessage, $ERROR_EXIT_VALUE);
   }
#
# Check the number
#
   return 0 if ( $numToChk =~ /^\D*$/ || "$numToChk" =~ /^-0*$/ ||
                 $numToChk =~ /^\D*0+\D*\d+\D*/ );
   ( $numToChk =~ /^-?\d+$/ ) ? return 1 : return 0; 
}
#
# Subroutine isFloat - Verifies that the passed argument is an float
#
sub isFloat {
#
# Global variables used
#============================
#
#  $ERROR_EXIT_VALUE;
#  %LOG_TYPES;
#
#============================
#
# Declare variables
#
   my $NUM_REQUIRED_ARGS;
   my $numToChk;
   my @valuesToChk;
   my $valueToChk;
   my $logMessage;
#
# Initialize variables
#
   $NUM_REQUIRED_ARGS = 1;
   if ( scalar(@_) == $NUM_REQUIRED_ARGS ) {
      ($numToChk) = @_;
   }
   else {
      $logMessage = "Invalid number of arguments passed to subroutine ".
                    "<isFloat>";
      putLogEntry($LOG_TYPES{'error'}, \$logMessage, $ERROR_EXIT_VALUE);
   }
#
# Check the number
#
   return 0 if ( "$numToChk" =~ /^\D*0+\D*\d+\D*\.?\D*\d*\D*$/ );

   return 0 if ( "$numToChk" =~ /\.+\D*\d*\.+/ );

   ( $numToChk =~ /^-?\d+\.?\d*/ ) ? return 1 : return 0;
#
# Return that this is a valid float
#
   return 1;
}
#
# Subroutine isValidFileName - Verifies that passed string is a valid file
#                              name or part of a file name
#
sub isValidFileName {
#
# Global variables used
#============================
#
#  $ERROR_EXIT_VALUE;
#  %LOG_TYPES;
#
#============================
#
# Declare variables
#
   my $NUM_REQUIRED_ARGS;
   my $stringToChk;
   my $stringLength;
   my $index;
   my $logMessage;
#
# Initialize variables
#
   $NUM_REQUIRED_ARGS = 1;
   if ( scalar(@_) == $NUM_REQUIRED_ARGS ) {
      ($stringToChk) = @_;
   }
   else {
      $logMessage = "Invalid number of arguments passed to subroutine ".
                    "<isValidFileName>";
      putLogEntry($LOG_TYPES{'error'}, \$logMessage, $ERROR_EXIT_VALUE);
   }
#
# Check the string
# It seems that only NULL strings and strings with
# the directory separator '/' are invalid
#
   if ( defined($stringToChk) ) {
      $stringLength = length($stringToChk);
      if ( $stringLength > 0 ) { 
         for ($index=0; $index<$stringLength; $index++) {
            return 0 if ( substr($stringToChk, $index, 1) eq '/' );
         }
      }
      else {
         return 0;
      }
   }
   else {
      return 0;
   }
#
   return 1;
}
#
# Subroutine isValidDirName - Verifies that passed string is a valid directory
#                             name or part of a directory name
#
sub isValidDirName {
#
# Declare variables
#
   my $NUM_REQUIRED_ARGS;
   my $stringToChk;
   my $stringLength;
   my $logMessage;
   my @dirNames;
   my $dirName;
#
# Initialize variables
#
   $NUM_REQUIRED_ARGS = 1;
   if ( scalar(@_) == $NUM_REQUIRED_ARGS ) {
      ($stringToChk) = @_;
   }
   else {
      $logMessage = "Invalid number of arguments passed to subroutine ".
                    "<isValidDirName>";
      putLogEntry($LOG_TYPES{'error'}, \$logMessage, $ERROR_EXIT_VALUE);
   }
#
# Check the string
# It seems that only NULL strings and strings with
# only the '.' or '..' are invalid
#
   if ( defined($stringToChk) ) {
      $stringLength = length($stringToChk);
      if ( $stringLength > 0 ) { 
         undef @dirNames;
	 @dirNames = split(/\//, $stringToChk);
	 foreach $dirName (@dirNames) {
	    return 0 if ( $dirName eq '.' || $dirName eq '..' );
	 }
      }
      else {
         return 0;
      }
   }
   else {
      return 0;
   }
#
   return 1;
}
#
# Subroutine localSignalHandler
#
sub localSignalHandler {
#
#-------------------------------------------------------------------------------
#
# Purpose: To provide capability to trap signals and prevent the user from 
#          breaking out of a PERL script.
#
# Usage:   Used in conjunction with the PERL module "sigtrap"
#
# Note:    The variable "scriptCanBeStopped" must be set in the calling
#          script to allow certain portions of the localSignalHandler
#          to stop the script.  Set "scriptCanBeStopped=1" to allow the
#          user to stop the script or "scriptCanBeStopped=0" to not
#          allow the user to stop the script.
#
# Note:    The variable "ctrlcAlready" must be set in the calling
#          script to allow certain portions of the localSignalHandler
#          to stop the script.  Set "ctrlcAlready=0" in the main
#          package of the main script.
#
# Note:    The variable "ctrlzAlready" must be set in the calling
#          script to allow certain portions of the localSignalHandler
#          to stop the script.  Set "ctrlzAlready=0" in the main
#          package of th  script.
#
#-------------------------------------------------------------------------------
#
# Global variables used
#-------------------------------------------------------------------------------
#
#  $SCRIPT
#  $scriptCanBeStopped
#  $ctrlcAlready
#  $ctrlzAlready
#  $ERROR_EXIT_VALUE
#
#-------------------------------------------------------------------------------
#
# Local variables
#
   my $sig;
   my $readReturn;
   my $yesNoReply;
   my $exitValue;
   my $TTY_COMMAND;
   my $runningOnTTY;
   my $logMessage;
#
# Initialize variables
#
   $yesNoReply  = 'ZZZZZzzzzzZZZZZ';
   $sig         = shift;
#
# Determine if on a tty or not
# ------------------------------------------
# Exit value of the system command:
#   0 if standard input is a terminal
#   1 if standard input is not a terminal
#   2 if given incorrect arguments
#   3 if a write error occurs
# ------------------------------------------
#
   $exitValue = -999;
   ( onTTY ) ? $runningOnTTY = 1 : $runningOnTTY = 0; 
#
# Trap signals and perform different actions based on
# running on a terminal or not
#
# Interrupt signal
#
   if ( $sig eq 'INT' ) {
      $logMessage = "Caught attempt to use CTRL-C to interrupt the script";
      putLogEntry($LOG_TYPES{'warning'}, \$logMessage);
      if ( ! $runningOnTTY ) {
	 $logMessage = "Exiting script before completion";
	 putLogEntry($LOG_TYPES{'error'}, \$logMessage, $ERROR_EXIT_VALUE);
      }
      else {
         if ( $scriptCanBeStopped ) { 
            STDOUT->print ("\n\n$SCRIPT: subroutine localSignalHandler: Caught ".
                           "attempt to use CTRL-C to interrupt $SCRIPT\n");
            if ( ! $ctrlcAlready ) {
               until ( $yesNoReply eq 'n' || $yesNoReply eq 'y' ) {
                  $ctrlcAlready=1;
                  STDOUT->print ("\nDo you want to allow $SCRIPT to ".
		                 "terminate gracefully? (y/n): ");
                  chomp ($yesNoReply=<STDIN>);
                  $yesNoReply =~ tr/A-Z/a-z/;
               }
               if ( $yesNoReply eq 'y') {
	          $logMessage = "Exiting script before completion";
		  putLogEntry($LOG_TYPES{'error'}, \$logMessage,
		              $ERROR_EXIT_VALUE);
               }
               elsif ($yesNoReply eq 'n') {
                  $ctrlcAlready=0;
               }
            }
	 }
         else {
	    $logMessage = "Script not interrupted at this time";
	    putLogEntry($LOG_TYPES{'info'}, \$logMessage);
         }
         return;
      }
   }
#
# Suspend or Stop signal
#
   elsif ( $sig eq 'TSTP' || $sig eq 'STOP' ) {
      $logMessage = "Caught attempt to use CTRL-Z to suspend the script";
      putLogEntry($LOG_TYPES{'warning'}, \$logMessage);
      if ( ! $runningOnTTY ) {
	 $logMessage = "Ignoring request to suspend script before completion";
	 putLogEntry($LOG_TYPES{'info'}, \$logMessage);
      }
      else {
         STDOUT->print ("\n\n$SCRIPT: subroutine localSignalHandler: Caught ".
                        "attempt to use CTRL-Z to suspend $SCRIPT\n");
         if ( ! $ctrlzAlready ) {
            $ctrlzAlready = 1;
         }
	 $logMessage = "Ignoring request to suspend script before completion";
	 putLogEntry($LOG_TYPES{'info'}, \$logMessage);
      }
      return;
   }
#
# Terminate signal
#
   elsif ( $sig eq 'TERM' ) {
      $logMessage = "Caught attempt to send a terminate signal to the ".
                    "process running the script";
      putLogEntry($LOG_TYPES{'warning'}, \$logMessage);
      if ( ! $runningOnTTY ) {
	 $logMessage = "Script terminated before completion";
	 putLogEntry($LOG_TYPES{'error'}, \$logMessage, $ERROR_EXIT_VALUE);
      }
      else {
         if ( $scriptCanBeStopped ) {
            STDOUT->print ("\n\n$SCRIPT: subroutine localSignalHandler: ".
                           "Caught attempt to send a terminate signal to ".
                           "the process running $SCRIPT\n");
            if ( $runningOnTTY ) {
               until ( $yesNoReply eq 'n' || $yesNoReply eq 'y' ) {
                  STDOUT->print ("Do you want to allow $SCRIPT to terminate ".
	                         "gracefully? (y/n): ");
                  chomp ($yesNoReply=<STDIN>);
                  $yesNoReply =~ tr/A-Z/a-z/;
               }
               if ( $yesNoReply eq 'y') {
	          $logMessage = "Script terminated before completion";
		  putLogEntry($LOG_TYPES{'error'}, \$logMessage,
		              $ERROR_EXIT_VALUE);
               }
            }
         }
         else {    
	    $logMessage = "Ignoring request to terminate the script";
	    putLogEntry($LOG_TYPES{'info'}, \$logMessage);
         }
         return;
      }
   }
#
# Quit signal
#
   elsif ( $sig eq 'QUIT' ) {
      $logMessage = "Caught attempt to send a quit signal to the ".
                    "process running the script";
      putLogEntry($LOG_TYPES{'warning'}, \$logMessage);
      if ( ! $runningOnTTY ) {
	 $logMessage = "Script quit before completion";
	 putLogEntry($LOG_TYPES{'error'}, \$logMessage, $ERROR_EXIT_VALUE);
      }
      else {
         if ( $scriptCanBeStopped ) {
            STDOUT->print ("\n\n$SCRIPT: subroutine localSignalHandler: ".
                           "Caught attempt to send a quit signal to the ".
                           "process running $SCRIPT\n");
            until ( $yesNoReply eq 'n' || $yesNoReply eq 'y' ) {
               STDOUT->print ("Do you want to allow $SCRIPT to quit? (y/n): ");
               chomp ($yesNoReply=<STDIN>);
               $yesNoReply =~ tr/A-Z/a-z/;
            }
            if ( $yesNoReply eq 'y') {
	       $logMessage = "Script quit before completion";
	       putLogEntry($LOG_TYPES{'error'}, \$logMessage,
	                   $ERROR_EXIT_VALUE);
            }
         }
         else {    
	    $logMessage = "Ignoring request to quit the script";
	    putLogEntry($LOG_TYPES{'info'}, \$logMessage);
         }
         return;
      }
   }
#
# Abort signal
#
   elsif ( $sig eq 'ABRT' ) {
      $logMessage = "Caught attempt to send an abort signal to the ".
                    "process running the script";
      putLogEntry($LOG_TYPES{'warning'}, \$logMessage);
      if ( ! $runningOnTTY ) {
	 $logMessage = "Script aborted before completion";
	 putLogEntry($LOG_TYPES{'error'}, \$logMessage, $ERROR_EXIT_VALUE);
      }
      else {
         if ( $scriptCanBeStopped ) {
            STDOUT->print ("\n\n$SCRIPT: subroutine localSignalHandler: ".
                           "Caught attempt to send an abort signal to ".
                           "the process running $SCRIPT\n");
            if ( $runningOnTTY ) {
               until ( $yesNoReply eq 'n' || $yesNoReply eq 'y' ) {
                  STDOUT->print ("Do you want to allow $SCRIPT to abort? (y/n): ");
                  chomp ($yesNoReply=<STDIN>);
                  $yesNoReply =~ tr/A-Z/a-z/;
               }
               if ( $yesNoReply eq 'y') {
	          $logMessage = "Script aborted before completion";
	          putLogEntry($LOG_TYPES{'error'}, \$logMessage,
		              $ERROR_EXIT_VALUE);
               }
            }
         }
         else {    
	    $logMessage = "Ignoring request to abort the script";
	    putLogEntry($LOG_TYPES{'info'}, \$logMessage);
         }
         return;
      }
   }
}
#
# Subroutine getFileViaWeb - Uses the getstore method from the PERL module
#                            LWP::Simple to retrieve a file via the web.
#                            The protocols HTTP, HTTPS and FTP can be used.
sub getFileViaWeb {
#
# Usage
#================================
#
# $returnValue = getFileViaWeb(REMOTE_WEB_FILE, LOCAL_FILE)
#    or
# $returnValue = getFileViaWeb(REMOTE_WEB_FILE, LOCAL_FIlE, FTP_PASSIVE_SETTING)
#
# The second form is for when the file will be retrieved using the 
# FTP protocol.  When the FTP protocol is used then the REMOTE_WEB_FILE
# should include the userID and password in the form:
#
#    ftp://$USER_ID:$PASSWORD@$URL/$DIR/$FILE_NAME
#
# The value for the returnCode is an integer and is PERL based meaning:
#
#    Success = 1
#    Failure = 0
#
#================================
#
# Global variables used
#================================
#
# %LOG_TYPES
# $ERROR_EXIT_VALUE
# $ENV{'FTP_PASSIVE'} 
# $DEBUG
#
#================================
#
# Subroutines or PERL built-in functions called
#=============================================
#
# putLogEntry
# getstore   - From the LWP::Simple module
# is_success - From the LWP::Simple module
# is_error   - From the LWP::Simple module
#
#=============================================
#
# Include files and pragmas
#
   use LWP::Simple;
#
# Declare variables
#
   my $MIN_NUM_REQUIRED_ARGS;
   my $MAX_NUM_REQUIRED_ARGS;
   my $numArgs;
   my $remoteFile;
   my $localFile;
   my $ftpPassiveValue;
   my $returnCode;
   my @validProtocols;
   my $protocol;
   my $logMessage;
   my $returnCodeMessage;
#
# Initialize variables
#
   $MIN_NUM_REQUIRED_ARGS = 2;
   $MAX_NUM_REQUIRED_ARGS = 3;
   $numArgs = scalar(@_);
   if ( $numArgs == $MIN_NUM_REQUIRED_ARGS ) {
      ($remoteFile, $localFile) = @_
   }
   elsif ( $numArgs == $MAX_NUM_REQUIRED_ARGS ) {
      ($remoteFile, $localFile, $ftpPassiveValue) = @_
   }
   else {
      $logMessage = "Invalid number of arguments passed to subroutine ".
                    "<readXML>";
      putLogEntry($LOG_TYPES{'error'}, \$logMessage, $ERROR_EXIT_VALUE);
   }
   @validProtocols = ('http', 'https', 'ftp');
#
# Validate that the value for the remoteFile contains a valid protocol
#
   $remoteFile =~ /^([a-zA-Z]{3,5}):\/\/.*$/;
   if ( defined($1) ) {
#
# Get a copy of the protocol and convert it to lower case
#
      $protocol = $1;
      $protocol =~ s/\w/\l$&/g;
      if ( scalar(grep(/^$protocol$/i, @validProtocols)) != 1 )  {
         $logMessage = "Invalid protocol <$1> provided with the remote ".
	               "file <$remoteFile>";
         putLogEntry($LOG_TYPES{'warning'}, \$logMessage);
	 return 0;
      }
   }
   else {
      $logMessage = "Invalid remote file <$remoteFile>";
      putLogEntry($LOG_TYPES{'warning'}, \$logMessage);
      return 0;
   }
#
# Verify for FTP protocol that if a userId and password was provided
# they have valid values
#
   if ( $protocol eq 'ftp' ) {
      $remoteFile =~ /^\[a-zA-Z]:\/\/(\w+):(.*)\@.*$/;
      if ( defined($1) ) {
         if ( "$1" ne 'anonymous' ) {
	    if ( ! defined($2) ) {
	       $logMessage = "FTP requests that not using the userID ".
	                     "<anonymous> require a password - No ".
			     "password was provided in the string for ".
			     "the remote file <$remoteFile>";
               putLogEntry($LOG_TYPES{'warning'}, \$logMessage);
               return 0;
	    }
	 }
      }
#
# For protocol = ftp set the environment variable "FTP_PASSIVE", which is
# used way down under the hood in Net::FTP.  If FTP_PASSIVE is set to
# non-zero then the passive mode is used, otherwise, the active mode is used.
# NOTE:  This is only needed if using the transfer mode <FTP> inside
# of the LWP getstore.
#
      if ( isInt($ftpPassiveValue) ) {
         if ( $DEBUG ) {
	    $logMessage = "Setting FTP passive mode to <$ftpPassiveValue>";
            putLogEntry($LOG_TYPES{'DEBUG'}, \$logMessage);
         }
         $ENV{'FTP_PASSIVE'} = $ftpPassiveValue;
      }
      else {
         $logMessage = "Invalid value <$ftpPassiveValue> for the ".
	               "FTP passive mode";
         putLogEntry($LOG_TYPES{'warning'}, \$logMessage);
         return 0;
      }
   }
#
# Attempt to download the file
#
   if ( $DEBUG ) {
      $logMessage = "Attempting to retrieve the remote file ".
                    "<$remoteFile> and store it in the local file ".
		    "<$localFile> using getstore";
      putLogEntry($LOG_TYPES{'debug'}, \$logMessage);
   }
#
   undef $returnCode; 
   $returnCode = getstore("$remoteFile", "$localFile");
   #
   if ( $DEBUG ) {
      $logMessage = "Getstore returnCode <$returnCode> for retrieving remote ".
                    "file <$remoteFile>";
      putLogEntry($LOG_TYPES{'debug'}, \$logMessage);
   }
#
# Check the returnCode
#
   if ( is_success($returnCode) ) {
      $logMessage = "Successfully retrieved remote file <$remoteFile> ".
                    "and stored it in the file <$localFile>";
      putLogEntry($LOG_TYPES{'info'}, \$logMessage);
      return 1;
   }
   elsif ( is_error($returnCode) ) {
      $returnCodeMessage = status_message($returnCode);
      $logMessage = "Failed to retrieve remote file <$remoteFile> - ".
	            "message <$returnCode - $returnCodeMessage>";
      putLogEntry($LOG_TYPES{'warning'}, \$logMessage);
      return 0;
   }
   else {
      $logMessage = "Unknown return code <$returnCode> for ".
                    "protocol <$protocol>";
      putLogEntry($LOG_TYPES{'warning'}, \$logMessage);
      return 0;
   }
}
#
# Subroutine onTTY - Checks to see if the script is being run on a terminal
#                    (i.e., not from cron or in the background)
sub onTTY {
#
# Declare variables
#
   my $TTY_COMMAND;
   my $returnValue;
   my $commandToRun;
#
# Initialize variables
#
   $TTY_COMMAND = '/usr/bin/tty';
#
# Determine if on a tty or not
# ------------------------------------------
# Exit value of the PERL system command:
#   0 if standard input is a terminal
#   1 if standard input is not a terminal
#   2 if given incorrect arguments
#   3 if a write error occurs
# ------------------------------------------
#
#  $returnValue = -999;
#  if ( -x "$TTY_COMMAND" ) {
#     $commandToRun = "$TTY_COMMAND -s";
#     $returnValue = runSystemCommand(\$commandToRun); 
#     ( $returnValue == 1 ) ? return 1 : return 0;
#  }
#  else {
#
# Assume running on a terminal
#
#     return 1;
#  }
   ( -t STDOUT ) ? return 1 : return 0;
}
1;
