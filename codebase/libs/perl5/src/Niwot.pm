# *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* */
# ** Copyright UCAR (c) 1992 - 2010 */
# ** University Corporation for Atmospheric Research(UCAR) */
# ** National Center for Atmospheric Research(NCAR) */
# ** Research Applications Laboratory(RAL) */
# ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA */
# ** 2010/10/7 23:12:42 */
# *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* */
################################################################################
#
# Niwot library for managing project control
#
# Terri L. Betancourt, RAP, NCAR, Boulder, CO, 80307, USA
# March 2002
#
# $Id: Niwot.pm,v 1.18 2016/01/08 22:13:09 dettling Exp $
################################################################################

package Niwot;
require Exporter;
@ISA = qw(Exporter);
@EXPORT = qw( setMsgStderr setMsgConsoleHost setMsgTime setMsgLogFile postMsg 
              postError postWarning postInfo postDebug postFatal 
              setHostEnv getHostList getHostRoles isLocalHost areHostsEqual
              setModeEquiv getModeEquiv 
              setRadarEnv getRadarId getRadarEntry setProcessInstance
              trim appendFile);

Env::import();
use Env;
use Socket;
use Net::Domain qw(hostfqdn);
use Sys::Hostname;

#
# Set unbuffered IO
#
$| = 1;

#
# Get the program basename.
#
($prog = $0) =~ s|.*/||;

#
# Get the local hostname
#
$localHost = hostname();

1;

#===============================================================================
# MESSAGING
#===============================================================================

#-------------------------------------------------------------------------------
# Set option to print messages to STDERR
#    Arguments:      none
#    Prerequisites:  none
#    Return:         none
#
sub setMsgStderr
{
   $postStderr = 1;
}

#-------------------------------------------------------------------------------
# Set console option for messaging
#    Arguments:      hostname
#    Prerequisites:  none
#    Return:         none
#
sub setMsgConsoleHost
{
   ( $postConsoleHost ) = @_;
}

#-------------------------------------------------------------------------------
# Turn on option for including time of message
#    Arguments:      none
#    Prerequisites:  none
#    Return:         none
#
sub setMsgTime
{
   $postTime = 1;
}

#-------------------------------------------------------------------------------
# Open the messaging log file
#    Arguments:      logFileExpr
#    Prerequisites:  none
#    Return:         none
#    Notes:          See perl open() function for definition of file expression
#                    to specify open modes
#
sub setMsgLogFile
{
   sub postMsg;

   my( $logFileExpr ) = @_;
   unless ( open( LOG_FILE, $logFileExpr )) {
      postMsg( "Cannot open log file $logFileExpr", ERROR );
      return;
   }

   $postLog = 1;
}

#-------------------------------------------------------------------------------
# Post library messages 
#    Arguments:      messageText, [severe]
#    Prerequisites:  none
#    Return:         none
#    Notes:          to modify the behavior of the messaging, use the functions 
#                    setMsgStderr(), setMsgConsoleHost(), setMsgTime(), 
#                    setMsgLogFile()
#

sub postMsg
{

   my( $msg, $severe ) = @_;
   my( $date, $msgText );

   #
   # Append the current time, if requested
   #
   if ( $postTime ) {
      $date = scalar localtime;
      $msgText = "$msg ($date)";
   }
   else {
      $msgText = $msg;
   }

   #
   # Post to stderr, if requested
   #
   if ( $postStderr || defined( $severe ) ) {
      print STDERR "$prog: $msgText\n";
   }

   #
   # Post to log file, if requested
   #
   if ( $postLog ) {
      print LOG_FILE "$prog: $msgText\n";
   }

# console messaging does not currently work w/ timeMsg set
#    if ( $postConsoleHost ) {
#       if ( areHostsEqual( $postConsoleHost, $localHost )) {
#          $cmd = "echo \"$prog: $msgText\" > /dev/console";
#       }
#       else {
#          $cmd = "ssh -x $postConsoleHost \"echo "
#               . "\"$prog: $msgText\" > /dev/console\"";
#       }
#       system( $cmd );
#    }

}

#-------------------------------------------------------------------------------
# Post an error message
#    Arguments:      messageText
#    Prerequisites:  none
#    Return:         none
#    Notes:          to modify the behavior of the messaging, use the functions 
#                    setMsgStderr(), setMsgConsoleHost(), setMsgTime(), 
#                    setMsgLogFile()
#
sub postError
{
    my( $msg ) = @_;

    #
    # Append "error" message to message text
    #
    $msgTxt = "ERROR:    $msg";

    #
    # Send it off to postMsg, assume this is severe
    #
    postMsg( $msgTxt, ERROR );
}

#-------------------------------------------------------------------------------
# Post a warning message
#    Arguments:      messageText
#    Prerequisites:  none
#    Return:         none
#    Notes:          to modify the behavior of the messaging, use the functions 
#                    setMsgStderr(), setMsgConsoleHost(), setMsgTime(), 
#                    setMsgLogFile()
#
sub postWarning
{
    my( $msg ) = @_;

    #
    # Append "warning" message to message text
    #
    $msgTxt = "WARNING:  $msg";

    #
    # Send it off to postMsg, assume this is NOT severe
    #
    postMsg( $msgTxt );
}

#-------------------------------------------------------------------------------
# Post an info message
#    Arguments:      messageText
#    Prerequisites:  none
#    Return:         none
#    Notes:          to modify the behavior of the messaging, use the functions 
#                    setMsgStderr(), setMsgConsoleHost(), setMsgTime(), 
#                    setMsgLogFile()
#
sub postInfo
{
    my( $msg ) = @_;

    #
    # Append "info" message to message text
    #
    $msgTxt = "INFO:     $msg";

    #
    # Send it off to postMsg, assume this is NOT severe
    #
    postMsg( $msgTxt );
}

#-------------------------------------------------------------------------------
# Post a debug message
#    Arguments:      messageText
#    Prerequisites:  none
#    Return:         none
#    Notes:          to modify the behavior of the messaging, use the functions 
#                    setMsgStderr(), setMsgConsoleHost(), setMsgTime(), 
#                    setMsgLogFile()
#
sub postDebug
{
    my( $msg ) = @_;

    #
    # Append "debug" message to message text
    #
    $msgTxt = "DEBUG:    $msg";

    #
    # Send it off to postMsg, assume this is NOT severe
    #
    postMsg( $msgTxt );
}

#-------------------------------------------------------------------------------
# Post a fatal error message
#    Arguments:      messageText
#    Prerequisites:  none
#    Return:         none
#    Notes:          to modify the behavior of the messaging, use the functions 
#                    setMsgStderr(), setMsgConsoleHost(), setMsgTime(), 
#                    setMsgLogFile()
#
sub postFatal
{
    my( $msg ) = @_;

    #
    # Append "fatal error" message to message text
    #
    $msgTxt = "FATAL:    $msg";

    #
    # Send it off to postMsg, assume this is severe
    #
    postMsg( $msgTxt, FATAL );
}

#===============================================================================
# HOST NAME / HOST ROLE LOOKUP
#===============================================================================

#-------------------------------------------------------------------------------
# See if the specified host is the local host
#    Arguments:      hostname
#    Prerequisites:  none
#    Return:         1 true, 0 false
#
sub isLocalHost
{
   my( $specifiedHost ) = @_;

   #
   # See if the specified host name is equal to the local host
   #
   return( areHostsEqual( $specifiedHost, $localHost ));
}

#-------------------------------------------------------------------------------
# See two hostnames refer to the same host machine
#    Arguments:      hostname1, hostname2
#    Prerequisites:  none
#    Return:         1 true, 0 false
#
sub areHostsEqual
{
   my( $host1, $host2 ) = @_;

   #
   # Check for 'localhost' keyword
   #
   if ( $host1 eq "localhost" ) {
      $host1 = $localHost;
   }
   if ( $host2 eq "localhost" ) {
      $host2 = $localHost;
   }

   #
   # Expand environment variables, if necessary
   #
   if ( $host1 =~ m/\$(\w+)/ ) {
      $host1 = $ENV{$1};  
   }
   if ( $host2 =~ m/\$(\w+)/ ) {
      $host2 = $ENV{$1};
   }

   #
   # Get the fully qualified domains
   #
   $address1 = gethostbyname( $host1 );
   $fqdn1    = gethostbyaddr( $address1, AF_INET );

   $address2 = gethostbyname( $host2 );
   $fqdn2    = gethostbyaddr( $address2, AF_INET );

   #
   # Say something if we could not resolve the names, and exit.
   # Exiting is a fairly brutal option, but
   # otherwise behavior in this case is very puzzling and
   # leads to a lot of debugging. Niles.
   #
   if (!($fqdn1)) {
       print STDERR "Failed to resolve hostname : $host1\n";
       die "Either remove $host1 from host list or resolve it.\n";
   }

   if (!($fqdn2)) {
       print STDERR "Failed to resolve hostname : $host2\n";
       die "Either remove $host2 from host list or resolve it.\n";
   }


   if ( $fqdn1 eq $fqdn2 ) {
      return 1;
   }
   else {
      return 0;
   }
}

#-------------------------------------------------------------------------------
# Set the host environment
#    Arguments:      hostEnvFile
#    Prerequisites:  none
#    Return:         0 on success, -1 on failure
#
sub setHostEnv
{
   my( $hostEnvFile ) = @_;

   #
   # Make sure the host env file exists
   #
   if ( ! -e "$hostEnvFile" ) {
      postMsg( "Host env file '$hostEnvFile' does not exist.", ERROR );
      return -1;
   }

   #
   # Open the host env file
   #
   unless (open(HOST_FILE, $hostEnvFile)) {
      postMsg( "Cannot open the file $hostEnvFile.", ERROR );
      return -1;
   }

   #
   # Read the host environment into a table of host role entries
   #
   @hostRoleTable = <HOST_FILE>;

   return 0;
}

#-------------------------------------------------------------------------------
# Determine unique list of system hosts, i.e., hostnames are not repeated
#    Arguments:      none
#    Prerequisites:  setHostEnv()
#    Return:         @hostlist
#
sub getHostList
{
   my( $host, @hosts );
   my( $item, %seen );

   if ( !( @hostRoleTable) ) {
      postMsg( "Unable to access host env table.", ERROR );
      return;
   }

   #
   # Scan the host table to see which machines are in the system
   #
   foreach $entry ( @hostRoleTable ) {
 
      #
      # Skip comment and blank lines
      #
      next if ( $entry =~ m/^\s*#/ );
      next if ( $entry =~ m/^\s*$/ );

      #
      # Split the line into the expected tokens
      #
      ($setenv, $role, $host) = split ' ', $entry;

      #
      # Skip environment variable references
      #
      next if ( $host =~ m/(\$\w+)/ );

      #
      # Add this host name to our list
      #
      push( @hosts, $host );
   }

   #
   # Get the unique host name list
   #
   %seen = ();
   foreach $item (@hosts) {
      $seen{$item}++;
   }

   return keys %seen;
}

#-------------------------------------------------------------------------------
# Determine list of host roles
#    Arguments:      none
#    Prerequisites:  setHostEnv()
#    Return:         @hostRoles
#
sub getHostRoles
{
   my( $host, @hosts );
   my( $item, %seen );

   if ( !( @hostRoleTable) ) {
      postMsg( "Unable to access host env table.", ERROR );
      return;
   }

   #
   # Scan the host table to see what roles this host plays
   #
   %seen = ();
   foreach $entry ( @hostRoleTable ) {
 
      #
      # Skip comment and blank lines
      #
      next if ( $entry =~ m/^\s*#/ );
      next if ( $entry =~ m/^\s*$/ );

      #
      # Split the line into the expected tokens
      #
      ($setenv, $role, $host) = split ' ', $entry;

      #
      # Check environment variable to see if it refers to this host
      #
      if ( $host =~ m/\$(\w+)/ ) {
         $hostEnv = $1;
         $seen{$hostEnv} and $seen{ $role }++;
      }
      elsif ( isLocalHost( $host )) {
         #
         # Add this role to our hash
         #
         $seen{ $role }++;
       }
   }

   #
   # Set the unique role list plus EVERY_HOST
   #
   return ("EVERY_HOST", keys %seen);
}

#===============================================================================
# MODE EQUIVALENCE LOOKUP
#===============================================================================

#-------------------------------------------------------------------------------
# Set the mode equivalent lookup file
#    Arguments:      modeEquivFilePath
#    Prerequisites:  none
#    Return:         0 on success, -1 on failure
#
sub setModeEquiv
{
   my( $modeEquivFile ) = @_;

   #
   # Make sure the host env file exists
   #
   if ( ! -e "$modeEquivFile" ) {
      postMsg( "Mode equivalence file '$modeEquivFile' does not exist.", ERROR );
      return -1;
   }

   #
   # Open the host env file
   #
   unless (open(EQUIV_FILE, $modeEquivFile)) {
      postMsg( "Cannot open the file $modeEquivFile.", ERROR );
      return -1;
   }

   #
   # Read the mode equivalencies into a table of mode entries
   #
   @modeEquivTable = <EQUIV_FILE>;

   return 0;
}

#-------------------------------------------------------------------------------
# Get the list of equivalent file extensions for a specified mode
#    Arguments:      mode
#    Prerequisites:  setModeEquiv()
#    Return:         @equivList
#

sub getModeEquiv 
{
   my( $mode ) = @_;
   my( $entry, @equivList );

   if ( !( @modeEquivTable) ) {
      #
      # The mode itself and the keywork 'always' are our only equivalency
      #
      @equivList = ($mode,"always");
      return @equivList;
   }

   #
   # Scan the mode equivalence table to see what strings map 
   # to the specified mode
   #
   foreach $entry ( @modeEquivTable ) {

      #
      # Skip comment and blank lines
      #
      next if ( $entry =~ m/^\s*#/ );
      next if ( $entry =~ m/^\s*$/ );

      #
      # Build up a list of all equivalent file extensions
      #
      if ( $entry =~ m/^\s*$mode\s*(.+)$/ ) {
         $equivalents = $1;
         @equivList = (trim(split /,/, $equivalents),"always");
      }
   }

   return @equivList;
}

#===============================================================================
# RADAR INFORMATION LOOKUP
#===============================================================================

#-------------------------------------------------------------------------------
# Set the radar environment
#    Arguments:      radarEnvFile
#    Prerequisites:  none
#    Return:         0 on success, -1 on failure
#
sub setRadarEnv
{
   my( $radarEnvFile ) = @_;

   #
   # Make sure the radar env file exists
   #
   if ( ! -e "$radarEnvFile" ) {
      postMsg( "Radar env file '$radarEnvFile' does not exist.", ERROR );
      return -1;
   }

   #
   # Open the radar env file
   #
   unless (open(RADAR_FILE, $radarEnvFile)) {
      postMsg( "Cannot open the file $radarEnvFile.", ERROR );
      return -1;
   }

   #
   # Read the radar environment into a table of radar function entries
   #
   @radarFcnTable = <RADAR_FILE>;

   return 0;
}

#-------------------------------------------------------------------------------
# Get the radar ID for a specified function
#    Arguments:      function
#    Prerequisites:  setRadarEnv()
#    Return:         @radarIdList -or- RadarId[last]
#
sub getRadarId
{
   my( $specifiedFcn ) = @_;
   my( @radarIdList );

   #
   # Look up the radar Id from the radar function table
   #
   foreach $entry ( @radarFcnTable ) {

      #
      # Skip comment and blank lines
      #
      next if ( $entry =~ m/^\s*#/ );
      next if ( $entry =~ m/^\s*$/ );

      #
      # Split the line into the expected tokens
      #
      ($radarId, $host, $function) = split ' ', $entry;

      #
      # Skip entries which are not of the requested function type
      #
      next if ( $function ne $specifiedFcn );

      #
      # If we got this far, we have found what we are looking for
      # Save the radar Id
      #
      push( @radarIdList, $radarId );
   }

   #
   # Return the last radarId or the whole list, depending on context
   #
   return wantarray ? @radarIdList : $radarIdList[$#radarIdList];
}
#-------------------------------------------------------------------------------
# Get the radar entry (radarId and radarHost) for a specified function
# and optional entry index (1-based, i.e., first, second, etc.)
# If an index is not specified, the last radar entry of the specified
# function type is returned
#    Arguments:      function, [index]
#    Prerequisites:  setRadarEnv()
#    Return:         radarHostPair[last] -or- radarHostPair[index]
#
sub getRadarEntry
{
   my( $specifiedFcn, $specifiedIndex ) = @_;
   my( $index, $found );

   #
   # Look up the radar entry from the radar function table
   #
   $index = 1;
   foreach $entry ( @radarFcnTable ) {

      #
      # Skip comment and blank lines
      #
      next if ( $entry =~ m/^\s*#/ );
      next if ( $entry =~ m/^\s*$/ );

      #
      # Split the line into the expected tokens
      #
      ($radarId, $host, $function) = split ' ', $entry;

      #
      # Skip entries which are not of the requested function type
      #
      next if ( $function ne $specifiedFcn );

      #
      # We have the specified function, see if an index was specified
      #
      if ( defined( $specifiedIndex )) {
         #
         # if we are at the specified index, we're done
         #
         if ( $index == $specifiedIndex ) {
            $found = true;
            $foundRadar = $radarId;
            $foundHost  = $host;
            last;
         }
         else {
            $index++;
         }
      }
      else {
         $found = true;
         $foundRadar = $radarId;
         $foundHost  = $host;
      }
   }

   #
   # Return the specified indexed radarId, if requested
   #
   if ( $found ) {
      return $foundRadar, $foundHost;
   }
   else {
      return;
   }
}

#-------------------------------------------------------------------------------
# Interpolate the process instance with radarId, substitutions made in place
#    Arguments:      processListFile
#    Prerequisites:  setRadarEnv()
#    Return:         none
#
sub setProcessInstance 
{
   my( $processListFile ) = @_;
   my( $host, $oldLine, $newLine );

   unless ( open( OLD_PROCESS_FILE, "<$processListFile" )) {
      postMsg( "Cannot open process list file $processListFile", ERROR );
      return;
   }
   unless ( open( NEW_PROCESS_FILE, ">$processListFile.new" )) {
      postMsg( "Cannot open process list file $processListFile.new", ERROR );
      return;
   }


   while( $oldLine = <OLD_PROCESS_FILE> ) {
      chomp $oldLine;

      #
      # Write out comment and blank lines
      #
      if ( $oldLine =~ m/^\s*#/  ||  $oldLine =~ m/^\s*$/ ) {
         print NEW_PROCESS_FILE "$oldLine\n";   
         next;
      }

      #
      # Split the line into the expected tokens
      #
      ( $process, $instance, 
        $startScript, $killScript, 
        $host, $priority ) = split ' ', $oldLine;

      #
      # For now we are only going to interpolate the instance
      # with a radarId lookup
      #
      if ( $instance =~ m/(.*)\?(.+)\?(.*)/ ) {
         $function = $2;
         $radar = getRadarId( $function );
         if ( !defined( $radar )) {
            postMsg( "No radar found for '$function'", ERROR );
            return;
         }
         $newInstance = "$1$radar$3";
         $newLine = "$process \t$newInstance \t$startScript \t$killScript \t$host \t$priority";
         print NEW_PROCESS_FILE "$newLine\n";
        
      }
      else {
         print NEW_PROCESS_FILE "$oldLine\n";
      }
   }

   #
   # Move the new file into place
   #
   close( OLD_PROCESS_FILE );
   close( NEW_PROCESS_FILE );
   rename( "$processListFile.new", $processListFile );
}

#===============================================================================
# General utilities
#===============================================================================

#-------------------------------------------------------------------------------
# Append source file to destination file
#    Arguments:      dest, file to append to; source, additions.
#    Prerequisites:  None
#    Return:         None
#
sub appendFile 
{
   my ($source, $dest) = @_;

#
# Get the sizes of the files involved. This is useful for checking
# the result. If the result is not the size we'd expect, experience
# suggests that there are some things we should check.
#
   my $destSize = 0;
   if ( -e $dest ){
     $destSize = -s $dest;
   }

   my $sourceSize = 0;
   if ( -e $source){
     $sourceSize = -s $source;
   }

   my $expectedSize = $destSize + $sourceSize;

   if ( -e $source ) {
      $cmd = "cat $source >> $dest";
      system( $cmd );
#
# Get the size of the result and test it. Trivial errors in this
# concatenation process (full disk, permissions issues) have proven
# very expensive to debug in the past, so give a hint about what may
# be wrong.
#
      my $actualSize = 0;
      if ( -e $dest ) {
	  $actualSize = -s $dest;
      }
      if ($actualSize != $expectedSize) {
	  print STDERR "\nERROR appending $source to $dest :\n";
	  print STDERR "   $dest was $destSize bytes, $source was $sourceSize bytes\n";
          print STDERR "   Expected result to be $expectedSize bytes, got $actualSize bytes\n";
          print STDERR "   Check if disk is full or permissions are an issue.\n\n";
      }
  } else {
    print STDERR "\nWARNING appending $source to $dest but\n";
    print STDERR "      $source does not exist.\n\n";
  }
}

#-------------------------------------------------------------------------------
# Trim leading and trailing blanks -- straight from the Perl Cookbook
#    Arguments:      listOfTokens
#    Prerequisites:  none
#    Return:         @listOfTokens -or- firstToken
#
sub trim {
   my @out = @_;
   for( @out ) {
      s/^\s+//;
      s/\s+$//;
   }
   return wantarray ? @out : $out[0];
}

