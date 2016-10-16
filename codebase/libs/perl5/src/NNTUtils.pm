# *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* */
# ** Copyright UCAR (c) 1992 - 2010 */
# ** University Corporation for Atmospheric Research(UCAR) */
# ** National Center for Atmospheric Research(NCAR) */
# ** Research Applications Laboratory(RAL) */
# ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA */
# ** 2010/10/7 23:12:43 */
# *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* */
package    NNTUtils;
require    Exporter;
@ISA = qw(Exporter);
@EXPORT = qw(initTrigger waitForTrigger waitForTriggerMin findClosest getNowAsUtime expandPath checkExes checkDidssEnvVars advanceHour);

#
# Externals
#
# The sys_wait_h is required to get the correct return codes from the system() calls.

use POSIX 'sys_wait_h'; 
use Env;
use Cwd;
Env::import();
use Time::Local;                   # make 'date' available

# Local RAP libs

use lib "$ENV{RAP_SHARED_LIB_DIR}/perl5/";
use lib "$ENV{RAP_LIB_DIR}/perl5/";
use Toolsa;
use Niwot;

1;   # Perl requires this

# =============================== SUBROUTINES ===========================
#
#
# Subroutine: initTrigger
#
# Usage:      initTrigger( $regInt, $dbg, $cronLikeTime )
#
# Function:   Sets a list of times to be used in the following calls
#             to wait for the next time. The list will be sorted in 
#             this subroutine and will be a "global" or "static" 
#             variable. In addition, the index will be set to indicate
#             that we don't yet know where we are in the list of times.
#             Also sets the registration interval and debug flag.
#
# Input:      $regInt        = Registration interval in seconds
#
#             $dbg           = Debug flag. 0 - off, 1 - on
#
#             $cronTime      = String containing times in cron
#                              like format
#
# Output:     0 on success and -1 on failure
#
########################################################################

sub initTrigger
{
    my ($regInt, $dbg, $cronTime) = @_;

    #
    # Initialize values
    #             
    $regInterval   = $regInt;
    $debug         = $dbg;
    $hourIdex      = -1;
    $minuteIdex    = -1;
    $numHours      = 0;
    $numMinutes    = 0;
    $wrapping      = 0;
    $tolerance     = 60;

    #
    # Check for weirdnesses in the cron time string
    #
    if( cronQC( $cronTime ) ) {
      Niwot::postError( "Time format incorrect" );
      return( -1 );
    }
    
    #
    # Parse the time string
    #   First split the times on white space - this should
    #   give us the fields, i.e. minutes, hours, etc.
    #
    my @fields = split /\s+/, $cronTime;

    #
    # Parse the minute field
    #
    if( $debug ) {
       Niwot::postDebug( "Minutes:");
    }

    ($retVal, @minuteList) = parseCronField( $fields[0], 0, 59 );
    if( $retVal ) {
        return( -1 );
    }

    #
    # Parse the hour field
    #
    if( $debug ) {
       Niwot::postDebug( "Hours:");
    }

    ($retVal, @hourList) = parseCronField( $fields[1], 0, 23 );
    if( $retVal ) {
        return( -1 );
    }

    #
    # If there are any other fields that are not set to *,
    # warn the user that we won't be doing anything about those
    # settings.
    #
    for( $i = 2; $i <= $#fields; $i++ ) { 
        if( $fields[$i] ne "*" ) {
          Niwot::postWarning( "Day, Month, Day of Week fields not supported" );
        }
    }

    $numMinutes = $#minuteList + 1;
    $numHours   = $#hourList + 1;

    return( 0 );
}

##################################################################
#
# Subroutine: cronQC
#
# Usage:      cronQC( $cronTime ) 
#
# Function:   Checks the cron time field for some weirdnesses
#
# Input:      $cronTime = string containing cron time field
#
# Output:     $retVal   = 0 on success and -1 on failure
#
##################################################################

sub cronQC {

    my( $cronTime ) = @_;

    if( $cronTime =~ /,\s+/ ) {
        return( -1 );
    }    

    if( $cronTime =~ /-\s+/ ) {
        return( -1 );
    }

    return( 0 );

}

##################################################################
#
# Subroutine: parseCronField
#
# Usage:      parseCronField( $field, $minVal, $maxVal, @timeList ) 
#
# Function:   Parses the $field string to pull out relavant times
#             and put into an array. The $field string in in a 
#             cron time format.
#
# Input:      $field    = minute or hour field pulled from
#                         cron-like format string
#
#             $minVal   = minimum value allowed for this field
#
#             $maxVal   = maximum value allowed for this field
#
# Output:     $retVal   = 0 on success and -1 on failure
#             
#             @timeList = resulting list of times 
#
# NOTE:       This subroutine uses source code developed by
#             David Loren Parsons <http://www.pell.portland.or.us/~orc>
#             as a reference. His code was written for a slightly different
#             purpose and in C. However, although the following code is
#             significantly different from the original, many of the ideas 
#             for how to parse a cron-like field came from this code.
#
########################################################################

sub parseCronField
{
    my ($field, $minVal, $maxVal) = @_;

    #
    # Declare variables
    #
    my $range;
    my @skips;
    my @nums;
    my @timeList;

    #
    # Initialize
    #
    my $idex    = 0;
    my $skip    = 1;
    my $forever = 1;

    #
    # Split field into a list of numbers or ranges
    #
    my @ranges  = split /,/, $field;

    foreach $range (@ranges) {

        #
        # Are we dealing with a range? A star is treated like
        # a range, where the lower end of the range is the min allowed
        # and the upper end of the range is the max allowed
        #
        if( $range =~ /-/ || $range =~ /\*/ ) { 

            #
            # The cron format allows a /(number) to indicate
            # a skip through a range. In other words, a */2,
            # for example, would indicate every two hours in
            # the full range. Strip that off if it is there
            # and retain the value.
            #
            @skips = split /\//, $range;

            #
            # If there are more than two substrings after
            # splitting on '/', that means there is more than
            # one '/' which is an error
            #
            if( $#skips > 1 ) {
               Niwot::postError( "Time format incorrect" );
               return( -1, @timeList );
            } 

            #
            # Otherwise, if there is more than one substring
            # it means tht there is a skip indicated, save
            # this value. It will necessarily be the second
            # substring - if not there is an error.
            #
            if ( $#skips == 1 ) {
                $skip = $skips[1];

                if( $skip == 0 ) {
                   Niwot::postError( "Cannot skip by zero" );
                   return( 1, @timeList );
                }

                if( $debug ) {
                   Niwot::postDebug( "  Skip = $skip" );
                }
            }

            #
            # If the first substring contains a '*', handle
            # that case
            #
            if( $skips[0] =~ /\*/ ) {

                #
                # Make sure that all that is left is a '*'.
                # Otherwise we have an error.
                #
                if( $skips[0] ne "*" ) {
                    Niwot::postError( "Time format incorrect" );
                    return( -1, @timeList );
                }

                if( $debug ) {
                   Niwot::postDebug( "  Use full range" );
                }

                #
                # Set our "range" to the min through the max
                #
                $nums[0] = $minVal;
                $nums[1] = $maxVal;

            } else {

                #
                # Now we necessarily have a '-' in our first
                # substring after splitting off any '/', but
                # make sure it isn't just a '-'
                #
                if( $skips[0] eq "-" ) {
                   Niwot::postError( "Time format incorrect" );
                   return( -1, @timeList );
                }

                @nums = split /-/, $skips[0];

                # 
                # Error out if there are more than two substrings,
                # or if the numbers specifying the range are out of order,
                # or if there is anything but digits in the range
                #
                if( $#nums > 1 || 
                    $nums[0] >= $nums[1] || 
                    $nums[0] =~ /\D/ || 
                    $nums[1] =~ /\D/ ) {
                   Niwot::postError( "Time format incorrect" );
                   return( -1, @timeList );
                }

                if( $debug ) {
                   Niwot::postDebug( "  Use $nums[0] through $nums[1]" );
                }


            }

            #
            # We are still dealing with a range of numbers, so check
            # to see that the range is not out of order with what we've
            # already put into our time list
            #
            if( $idex > 0 && $timeList[$idex] >= $nums[0] ) {
               Niwot::postError( "Times out of order" );
               return( -1, @timeList );
            }

            #
            # Make sure we are within min and max values
            #
            if( $nums[0] < $minVal || $nums[0] > $maxVal ||
                $nums[1] < $minVal || $nums[1] > $maxVal ) {
                Niwot::postError( "Times out of range" );
                return( -1, @timeList );
            }
            
            # 
            # Use the range and skip to fill in the time list
            #
            for( $i = $nums[0]; $i <= $nums[1]; $i += $skip, $idex++ ) {
                $timeList[$idex] = $i;
            }

        } else {

           #
           # We have a single number. Make sure it is actually a number
           # and not some strange characters
           #
           if( $range =~ /\D/ ) {
               Niwot::postError( "Time format incorrect" );
               return( -1, @timeList );
           }

           #
           # Is the number in range?
           #
           if( $range < $minVal || $range > $maxVal ) {
               Niwot::postError( "Time out of range" );
               return( -1, @timeList );
           }

           #
           # Make sure this number is in order with what is already
           # in the time list
           #
           if( $idex > 0 && $timeList[$idex] >= $range ) {
               Niwot::postError( "Times out of order" );
               return( -1, @timeList );
           }

           if( $debug ) {
               Niwot::postDebug( "  Use $range" );
           }

           #
           # Set the time list value and increment the index
           #
           $timeList[$idex] = $range;
           $idex++;

       }
    }

    return( 0, @timeList );
}

########################################################################
#
# Subroutine: waitForTrigger
#
# Usage:      $retVal = waitForTrigger()
#
# Function:   Sleeps until the next time in the trigger time array
#             occurs, then return.
#
# Input:      none
#
# Output:     0 on success and -1 on failure
#
########################################################################

sub waitForTrigger
{
  #
  # If the index isn't set, find the closest time in the list
  #
  if ( $minuteIdex < 0 || $hourIdex < 0) {
      if( findIdex() ) {
          return( -1 );
      }
  }
   
  #
  # Check current time against the next time in the list. If we are
  # within a tolerance of that time (in the list) return. Otherwise
  # sleep and try again.
  #
  my $currentUTime   = 0;
  my $wantUTime      = 0;
  my $timeDiff       = -1;
  my $sleepSecs      = 0;
  my $totalSleepSecs = 0;
  my $stillWaiting   = 1;

  my ( $sec, $min, $hour, $day, $mon, $year, $wday, $yday, $isdst );

  #
  # Find the current time values
  #
  $currentUTime = time;

  ($sec,$min,$hour,$day,$mon,$year,$wday,$yday,$isdst) = gmtime(time());
  $year = $year + 1900;

  #
  # Find the wanted time
  #
  $wantMinute = $minuteList[$minuteIdex];
  $wantHour   = $hourList[$hourIdex];
  $wantUTime  = timegm( 0, $wantMinute, $wantHour, $day, $mon, $year ); 

  #
  # See if we should be wrapping to the next day
  #         
  if( $wrapping ) {
     $wantUTime += (24 * 60 * 60);
     $wrapping = 0;
  }

  while ( $stillWaiting ) {

      #
      # Find the current utime
      #
      $currentUTime = time;
      
      #
      # Calculate the time differece for the current time and the time
      # we want based upon the above calculations
      #
      $timeDiff = $wantUTime - $currentUTime;

      if ( $timeDiff <= 0 ) {

          #
          # The current time has passed the wanted time
          #
          if ( $currentUTime - $wantUTime > $tolerance ) {

              #
              # We are too far past the wanted time. We must 
              # have skipped over one of the times in 
              # the list. Return with an error message, 
              # but reset the index so that if we come in 
              # here again, we can start from scratch.
              #
              Niwot::postError( "Current time too far in the past for " .
                  "desired time" );
              Niwot::postError( "Current utime = $currentUTime" );
              Niwot::postError( "Desired utime = $wantUTime" );

              $hourIdex   = -1;
              $minuteIdex = -1;

              return( -1 );

          } elsif ( $currentTime - $wantUTime <= $tolerance ) {

              #
              # We are within tolerance. We can trigger now
              #
              if( $debug ) {
                 $msg = sprintf( "Triggering for %.2d:%.2d", 
                                 $hourList[$hourIdex], 
                                 $minuteList[$minuteIdex] );
                 Niwot::postDebug( $msg );
              }
              
              #
              # Increment the index (and take care of wrapping)
              # for next time
              #
              $minuteIdex++;

              if ( $minuteIdex >= $numMinutes ) {

                  if( $debug ) {
                     Niwot::postDebug( "minute index = $minuteIdex, " .
                                       "num minutes = $numMinutes, " .
                                       "minute idex reset to zero " );
                  }

                  $minuteIdex = 0;
                  $hourIdex++;

                  if( $hourIdex >= $numHours ) {

                     if( $debug ) {
                        Niwot::postDebug( "hour index = $hourIdex, " .
                                          "num hours = $numHours, " .
                                          "hour idex reset to zero " );
                     }

                     $hourIdex = 0;
                     $wrapping = 1;
                  }

              }

              #
              # We're outta here...
              #
              return( 0 );
          }

      } else {

          #
          # The wanted time is still in the future. We need to 
          # do some sleeping and waiting for a bit.
          #
          if( $debug ) {
            Niwot::postDebug( "Time difference = $timeDiff" );
          }

          #
          # Register before we begin sleeping to ensure we won't get
          # killed in the middle of our sleep
          #
          Toolsa::PMU_force_register( "Sleeping" );

          #
          # Sleep over the entire time difference, but stop to 
          # register each registration interval. The time 
          # difference will probably not be an even multiple
          # of the registration interval, so check for the time
          # remaining in the time difference and take the minimum
          # of that and the registration interval as the time
          # to sleep. Once we are all done sleeping, try again.
          #
          $totalSleepSecs = 0;
          while( $totalSleepSecs < $timeDiff ) {

              $sleepSecs = $regInterval;

              if( $timeDiff - $totalSleepSecs < $regInterval ) { 
                $sleepSecs = $timeDiff - $totalSleepSecs;
              }
           
              if( $debug ) {
                Niwot::postDebug( "Slept $totalSleepSecs seconds so far, " .
                    "sleeping $sleepSecs, reg int = $regInterval" );       
              }

              sleep( $sleepSecs );
              Toolsa::PMU_force_register( "Still Sleeping" );
              $totalSleepSecs += $sleepSecs;
         }
      }
  }

  return( 0 );

}

#######################################################################
#
# Subroutine: findIdex
#
# Usage:      $retVal = findIdex()
#
# Function:   Finds the indeces in the time arrays for the time closest
#             to now that is still in the future
#
# Input:      none
#
# Output:     0 on success, -1 on failure
#
#######################################################################
sub findIdex
{
  #
  # Get current time values 
  #
  my ( $sec, $min, $hour, $day, $mon, $year, $wday, $yday, $isdst );

  ($sec,$min,$hour,$day,$mon,$year,$wday,$yday,$isdst) = gmtime(time());
  $year = $year + 1900;

  my $utimeNow = time;

  #
  # Debug statments
  #
  if ( $debug ) {
      my $timeStr = sprintf "%d/%d/%4d %.2d:%.2d:%.2d", $mon+1, $day, $year,
          $hour, $min, $sec;
      Niwot::postDebug( "Current time = $timeStr" );
  }

  #
  # Look through the ordered time list. Find the index of the first
  # time that is in the future.
  #
  my $wantMin      = 0;
  my $wantHour     = 0;
  my $utimeWant    = 0;
  my $timediff     = 0;
  my $i            = 0;
  my $iHour        = 0;
  my $iMinutes     = 0;
  my ($hourVal, $minuteVal);

  foreach $hourVal (@hourList) {

      $iMinutes = 0;

      foreach $minuteVal (@minuteList) {

         #
         # Calculated the wanted utime
         #
         $utimeWant = timegm( 0, $minuteVal, $hourVal, $day, $mon, $year );

         #
         # Find the time difference
         #
         $timediff = $utimeWant - $utimeNow;

         #
         # Debug statements
         #
         if( $debug ) {
            Niwot::postDebug( "Finding Index: hour = $hourVal, " .
                              "minute = $minuteVal, " .
                              "time diff = $timediff" );
         }

         # 
         # If the "wanted" time is in the future, we've found our
         # starting point. Set the indeces and get out.
         #
         if ( $timediff >= -1 * $tolerance ) {

             $minuteIdex = $iMinutes;
             $hourIdex   = $iHour;

             if( $debug ) {
                Niwot::postDebug( "Indeces found. minuteIdex = $minuteIdex" .
                        " hourIdex = $hourIdex" );
             }

             return 0;
         }

         #
         # Increment the minute counter
         #
         $iMinutes++;
     }
    
     $iHour++;
  }

  #
  # If we didn't find a time in the list that is "greater" than
  # the current time, we need to wrap around to the first time in the
  # next day. In this case we will deal with adding 24 hours in a different 
  # function. But in this case, we need to go to the beginning of the ordered 
  # hour list and the ordered minute list.
  #
  if( $hourIdex < 0 || $minuteIdex < 0 ) {
      $hourIdex   = 0;
      $minuteIdex = 0;
      $wrapping   = 1;

      if( $debug ) {
        Niwot::postDebug( "Off the end of the list, wrap to beginning" );
      }      
  } 

  return( 0 );
}




########################################################################
#
# Subroutine: waitForTriggerMin
#
# Usage:      ($return_val) = waitForTriggerMin($sleep_secs, $arr_mins, 
#                                               $do_reg, $test, $dbg)
#
# Function:   Sleep until the next minute in the trigger time array occurs
#             then return.
#
# Input:      $sleep_secs      seconds to sleep
#             $arr_mins        array of trigger times, as either minutes
#                              of every hour e.g., [10, 20, 50] or as 
#                              hours:mins e.g., [21:15, 22:30]
#             $narr            size of $arr_mins
#             $do_reg          flag to do PMU_auto_register (1=on, 0=off),
#                              you must have already used PMU_auto_init()
#                              in the calling script
#             $test            test flag, do not actually sleep
#             $dbg             debug flag 
#
# Output:     $return_val      1 if trigger time has occurred or 0 on error
# 
# Overview:
#

sub waitForTriggerMin
{
  my ($sleep_secs, $arr_mins, $narr, $do_reg, $test, $dbg) = @_;

  # Local variables

  my($subname, $return_val);
  my($dbg2, $dbg3);
  my($i, $found_time, $still_waiting, $now, $udate, $cmd);
  my($sec,$min,$hour,$mday,$mon,$year,$wday,$yday,$isdst, $day);
  my($is_time, $new_sleep_secs, $want_min, $is_ok, $do_sleep);

  # Set defaults

  $subname="waitForTriggerMin";
  $return_val=0;

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
  
  # Verbose debug

  if ($dbg2) {
      print(STDERR "$subname: Input...\n");
      print(STDERR "\tsleep_secs: $sleep_secs, narr: $narr, do_reg: $do_reg, test: $test\n");
      for ($i=0; $i<$narr; $i++) {
	  print(STDERR "\tarr_mins at i: $i, is: $arr_mins->[$i]\n");
      }
  }

  # Loop to check for trigger time, sleep and try again

  $found_time=0;
  $still_waiting = 1;
  $do_sleep=1;
  while ($still_waiting > 0) {

      ($is_time, $new_sleep_secs)=findClosest($sleep_secs, $arr_mins, $narr,
					      $test, $dbg);
      
      if ($is_time > 0) {
	  $found_time=1;
      }

      # Did we find a trigger time?
      # If so, need to get out of this loop
      # In test mode, need to get out of this loop

      if ($found_time > 0) {
	  if ($dbg > 0) {
	      print(STDERR "$subname: found a trigger time, new_sleep_secs: $new_sleep_secs\n");
	  }
	  $still_waiting=0;
	  $do_sleep=0;
      }

      if ($test > 0) {
	  $still_waiting=0;
      }

      # Sleep

      if ($do_sleep) {

	  if ($do_reg > 0) {
	      Toolsa::PMU_force_register("Sleeping");
	  }

	  if ($new_sleep_secs != $sleep_secs) {
	      if ($test > 0) {
		  print(STDERR "$subname: Would sleep new_sleep_secs: $new_sleep_secs\n");
	      } else {
		  if ($dbg2) {
		      print(STDERR "$subname: sleeping $new_sleep_secs\n");
		  }
		  sleep($new_sleep_secs);
	      }
	  } else {
	      if ($test > 0) {
		  print(STDERR "$subname: Would sleep sleep_secs: $sleep_secs\n");
	      } else {
		  if ($dbg2) {
		      print(STDERR "$subname: sleeping $sleep_secs\n");
		  }
		  sleep($sleep_secs);
	      }
	  }
      }

  } #endwhile
      
  # Done

  if ($found_time > 0) {
      $return_val=1;
  }

  return($return_val);

}


#-----------------------------------------------------------------------------------
# Subroutine: findClosest
#
# Usage:      ($return_val, $new_sleep_secs) = findClosest($sleep_secs, $arr, $narr,
#						   $test, $dbg);
#
# Function:   Find the closest time to now
#
# Input:      $arr             array of wanted minutes
#             $narr            size of $arr
#             $test            test flag
#             $dbg             debug flag 
#
# Output:     $return_val      1 if found a close enough time or 0 on error
#             $new_sleep_secs  how much to sleep next time to get closer to closest time
# 
# Overview:
#

sub findClosest
{
  my ($sleep_secs, $arr, $narr, $test, $dbg) = @_;

  # Local variables

  my($subname, $return_val, $new_sleep_secs);
  my($dbg2, $dbg3);
  my($found_idx, $i, $utime_now, $utime_want, $found, $nsecs, $timediff);
  my($want_time, $want_min, $is_time, $time_is_hrs_mins);
  my($sec,$min,$hour,$day,$mon,$year,$wday,$yday,$isdst);

  # Set defaults

  $subname="findClosest";
  $return_val=0;
  $new_sleep_secs=$sleep_secs;

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

  # Debug

  if ($dbg2) {
      print(STDERR "$subname: Inputs...\n");
      print(STDERR "\tsleep_secs: $sleep_secs, narr: $narr\n");
  }

  # Get time values so can add the want min to them

  ($sec,$min,$hour,$day,$mon,$year,$wday,$yday,$isdst)=gmtime(time());
  $year=$year+1900;

  # Get now as utime

  $utime_now=&getNowAsUtime;

  # Find trigger time closest to now but still in future, use utimes
  
  $found=0;
  $nsecs=-1;
  $found_idx=-1;

  for ($i=0; $i<$narr; $i++) {

      # Parse the time, is either a minute (e.g., 10) or hour:min (e.g., 23:10)
      
      $time_is_hrs_mins=0;
      $want_time=$arr->[$i];
      if ($want_time =~ /\:/) {
	  ($want_hour, $want_min)=split(":", $want_time);
	  $time_is_hrs_mins=1;
      } else {
	  $want_hour=$hour;
	  $want_min=$want_time;
      }

      if ($dbg3) {
	  print(STDERR "$subname: want_time; $want_time, want_hour: $want_hour, want_min: $want_min\n");
	  print(STDERR "$subname: current hour: $hour, current_min: $min\n");
      }

      # Because of hour-rollover, or mins and day-rollover for hours:mins
      # need to handle want minutes/hours:mins less than the current time 
      # by adding the minutes plus an hour (or 24 hours) to the current time

      if ($time_is_hrs_mins > 0) {
	  $utime_want=timegm($sec, $want_min, $want_hour, $day, $mon, $year);
	  if ($want_hour < $hour) {
	      $utime_want=$utime_want + (60 * 60 * 24);
	  }
      } else {
	  if ($want_min >= $min) {
	      $utime_want=timegm($sec, $want_min, $hour, $day, $mon, $year);
	  } else {
	      $utime_want=$utime_now + ($want_min * 60) + (60 * 60);
	  }
      }

      $timediff=$utime_want - $utime_now;

      if ($dbg3) {
	  print(STDERR "$subname: timediff: $timediff, nsecs: $nsecs, utime_want: $utime_want, utime_now: $utime_now\n");
      }
      
      if ((($timediff >= 0) && ($nsecs < 0)) ||
	  (($timediff >= 0) && ($timediff < $nsecs))) {
	  $nsecs=$timediff;
	  $found_idx=$i;
	  $found=1;
      }
  }

  if ($dbg2) {
      print(STDERR "$subname: found: $found, found_idx: $found_idx, nsecs: $nsecs\n");
  }
  
  # The times are within a minute. Found a time!
  # Is next closest time shorter than sleep_secs away?

  if (($found) && ($nsecs <= 60)) {
      if ($dbg3) {
	  print(STDERR "$subname: found time, times are within a minute\n");
      }
      $return_val=1;
  }

  # The closest time is in the future and is more than the 
  # sleep time away. Not a found time.

  if (($nsecs > $sleep_secs)) {
      if ($dbg3) {
	  print(STDERR "$subname: not time, want time more than sleep secs away\n");
      }
  }

  # More complicated, are the times close-enough?
  # If the sleep is long enough, we may not hit the exact minute
  # Need to reset the sleep to be shorter

  if (($found) && (($sleep_secs > $nsecs) && ($nsecs > 60))) {
      if ($dbg3) {
	  print(STDERR "$subname: closest time closer than sleep_secs away: $nsecs\n");
      }
      $new_sleep_secs = $nsecs;
  }

  return($return_val, $new_sleep_secs);
}


#-----------------------------------------------------------------------------------
# Subroutine: getNowAsUtime
#
# Usage:      $utime=getNowAsUtime
#
# Function:   Get now as Unix time
#
# Input:      none
#
# Output:     $utime    now in Unix time
# 

sub getNowAsUtime
{
    my ($dbg) = @_;

    # Local variables

    my($utime);
    my($sec,$min,$hour,$day,$mon,$year,$wday,$yday,$isdst);

    # Get the current time

    ($sec,$min,$hour,$day,$mon,$year,$wday,$yday,$isdst)=gmtime(time());
    $utime=timegm($sec, $min, $hour, $day, $mon, $year);
  
    return($utime);
}

#-----------------------------------------------------------------------------------
# Subroutine: expandPath
#
# Usage:      $outputPath = processDir($pathText)
#
# Function:   Processes path text, usually retrieved from an XML parameter file,
#             in order to prepend the path with RAP_DATA_DIR (or DATA_DIR if 
#             RAP_DATA_DIR does not exist) if necessary and to expand environment 
#             variables representing directory names within the path. Note that if 
#             the path text begins with . or / the value of RAP_DATA_DIR will not be 
#             prepended to the text. In addition, environment variables are assumed to 
#             represent directory names. Also note that if RAP_DATA_DIR is not defined
#             and DATA_DIR is not defined and the path does not start with . or /
#             the path will remain relative to the current location.
#
# Input:      $pathText = text representing the path
#
# Output:     $outputPath = output path which has been prepended with value of 
#             RAP_DATA_DIR and for which all environent variables have been
#             replaced with their values
#----------------------------------------------------------------------------------
sub expandPath 
{ 
    my( $path ) = @_;

    #
    # Local variables
    #
    my( $outputDir );
    my( $pathPieces, $section, $envVar, $rap_data_dir);

    #
    # Look for environment variables in the path, and expand these
    # for the local file path. It is assumed that the entire directory 
    # name is the environment variable. That is, the environment variable 
    # cannot be embedded in the directory name.
    #
    $outputDir  = "";
    @pathPieces = split /(\/)/, $path;
    foreach $section (@pathPieces) {
      if ( $section =~ /^\$/ ) {
        $envVar = substr( $section, 1 );
        $outputDir = $outputDir . $ENV{$envVar};
      }
      else {
        $outputDir = $outputDir . $section;
      }
    }

    #
    # If the output directory path does not begin with '.' or '/'
    # it is assumed to be relative to $RAP_DATA_DIR, so prepend
    # path with $RAP_DATA_DIR in that case
    #
    $rap_data_dir = $ENV{'RAP_DATA_DIR'};
    if ( $outputDir !~ /^\./ && $outputDir !~ /^\// ) {
	if ( !defined($rap_data_dir) ) {
            $rap_data_dir = $ENV{'DATA_DIR'};

            if ( !defined( $rap_data_dir) ) {
                return( $outputDir );
            }
        }

        $outputDir = $rap_data_dir . '/' . $outputDir;
    }

    #
    # Replace a '.' or a './' with a fully qualified path
    #
    if ( $outputDir =~ /^\./ ) {
      if ( $outputDir =~ /^\.\// ) {
	substr( $outputDir, 0, 2 ) = getcwd();
      }
      else {
	substr( $outputDir, 0, 1 ) = getcwd();
      }
    }
    

    return( $outputDir );
}

#-------------------------------------------------------------------
# Subroutine: checkExes
#
# Usage:      ($return_val) = checkExes($exes_arr, $dbg)
#
# Function:   Check that all the exes listed in the $exes_arr are in the
#             path
#
# Input:      $exes_arr        array of exes to check
#             $dbg             debug flag 
#
# Output:     $return_val      1 if all exes found, 0 on error
# 
# Overview:
#

sub checkExes
{
  my ($exes_arr, $dbg) = @_;

  # Local variables

  my($subname, $return_val);
  my ($exe, $check, $nerror);

  # Set defaults

  $subname="checkExes";
  $return_val=0;

  # Do check

  $nerror=0;

  for $exe (@{$exes_arr}) {
      $check=`which $exe`;
      if (!$check) {
	  print(STDERR "ERROR: needed exe is not in your path: $exe\n");
	  $nerror++;
      } else {
	  chop($check);
	  print(STDERR "Info: Will use exe: $exe from your path. This is: $check\n");
      }
  }

  if ($nerror > 0) {
      $return_val=0;
  } else {
      $return_val=1;
  }

  return($return_val);
}

#-------------------------------------------------------------------
# Subroutine: checkDidssEnvVars
#
# Usage:      $return_val=checkDidssEnvVars($dbg)
#
# Function:   Check that all the DIDSS env vars are defined
#
# Input:      $dbg             debug flag 
#
# Output:     $return_val      1 if all defined, 0 if any not defined
# 
# Overview:
#

sub checkDidssEnvVars
{
  my ($dbg) = @_;

  # Local variables

  my($subname, $return_val);
  my($procmap_host, $dmap_active, $rap_data_dir);

  # Set defaults

  $subname="checkDidssEnvVars";
  $return_val=1;

  # Do check

  $procmap_host=$ENV{PROCMAP_HOST};
  $dmap_active=$ENV{DATA_MAPPER_ACTIVE};
  $rap_data_dir=$ENV{RAP_DATA_DIR};

  if (!defined($procmap_host)) {
      print(STDERR "PROCMAP_HOST env var is not defined, cannot register with procmap\n");
      $return_val=0;
  }
  if (!defined($dmap_active)) {
      print(STDERR "DATA_MAPPER_ACTIVE env var is not defined, cannot register with DataMapper\n");
      $return_val=0;
  }
  if (!defined($rap_data_dir)) {
      print(STDERR "RAP_DATA_DIR env var is not defined\n");
      $return_val=0;
  }

  return($return_val);
}

##################################################################
#
# Subroutine: advanceHour
#
# Usage:      advanceHour( $ccyymmddhh, $hrs )
#
# Function:   Take the specified date string, advance it $hrs hours
#             and return date string in same format
#
# Input:      $ccyymmddhh = date string in ccyymmddhh format
#             $hrs        = number of hours to advance time;
#                           note that this value can be negative to
#                           decriment hours.
#
# Output:     $retVal     = new date string with the following format
#                           on success: ccyymmddhh
#
#                         = -1 on failure
#
##################################################################
sub advanceHour 
{
    my( $ccyymmddhh, $hrs ) = @_;

    my( $yr, $mm, $dd, $hh, $min, $sec );
    my( $utime, $advUtime, $timeStr );
    my( $asec, $amin, $ahr, $aday, $amon, $ayr, $wday, $yday, $isdst );

    #
    # Get year, month, day and hour from the input time string
    #
    $yr = substr( $ccyymmddhh, 0, 4 );
    $mm = substr( $ccyymmddhh, 4, 2 );
    $dd = substr( $ccyymmddhh, 6, 2 );
    $hh = substr( $ccyymmddhh, 8, 2 );

    #
    # Minute and seconds will always be zero
    #
    $min = 0;
    $sec = 0;

    #
    # Get unix time from the values obtained above
    #   Note that timegm expects the month to be in the
    #   range of 0-11, but the user will be entering
    #   a month in the range of 1-12
    #
    $utime = timegm( $sec, $min, $hh, $dd, $mm-1, $yr );

    #
    # Increase (or decrease if hrs is negative) time by specified
    # number of hours
    #
    $advUtime = $utime + ($hrs*60*60);

    if( $advUtime < 0 ) {
        Niwot::postError( "Subtracting off too many hours\n" );
        return( -1 );
    }

    #
    # Get new time values
    #   
    ($asec,$amin,$ahr,$aday,$amon,$ayr,$wday,$yday,$isdst)=gmtime($advUtime);
    $ayr += 1900;

    #
    # Set up new time string
    #   Note again that gmtime returns a month value between 0 and 11,
    #   but the user will be expecting a value between 1 and 12
    #
    $timeStr = sprintf( "%04d%02d%02d%02d", $ayr, $amon+1, $aday, $ahr );

    return( $timeStr );

}

#========================================= EOF =====================================
