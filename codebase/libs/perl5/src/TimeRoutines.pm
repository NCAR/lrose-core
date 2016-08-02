# *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* */
# ** Copyright UCAR (c) 1992 - 2010 */
# ** University Corporation for Atmospheric Research(UCAR) */
# ** National Center for Atmospheric Research(NCAR) */
# ** Research Applications Laboratory(RAL) */
# ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA */
# ** 2010/10/7 23:12:43 */
# *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* */
package TimeRoutines;

require "$ENV{'CVS_SRC_DIR'}/libs/perl5/src/DateRoutines.pm";

# Contents:
#  getCurrentHHMMSSPaddedStr
#  hhmmssFromSeconds
#  getCurrentGmtUnixTime
#  unix_time_from_datetime
#  datetime_from_unix_time

##################################################################
#                                                                #
#             SUBROUTINE: getCurrentHHMMSSPaddedStr              #
#                                                                #
##################################################################
# subroutine getCurrentHHMMSSPaddedStr
sub getCurrentHHMMSSPaddedStr
   {
   ($second, $minute, $hour, $day, $month, $year, $weekday, $julian_day, $daylight_savings) = localtime(time);

   # Adjust for perl time and date quirks
   $year_adjusted  = 1900 + $year;
   $month_adjusted = 1 + $month;
 
   # Pad dates and times if necessary
   if($month_adjusted < 10)
      {
      $month_adjusted = '0' . $month_adjusted;
      } # End of if($month_adjusted < 10) conditional.
 
   if($day < 10)
      {
      $day = '0' . $day;
      } # End of if($day < 10) conditional.
 
   if($hour < 10)
      {
      $hour = '0' . $hour;
      } # End of if($hour < 10) conditional.
 
   if($minute < 10)
      {
      $minute = '0' . $minute;
      } # End of if($minute < 10) conditional.
 
   if($second < 10)
      {
      $second = '0' . $second;
      } # End of if($second < 10) conditional.
 
   $yyyymmdd = $year_adjusted . $month_adjusted . $day;
   $hhmmss   = $hour . $minute . $second;

   return $hhmmss;
   } # End of getCurrentHHMMSSPaddedStr

##################################################################
#                                                                #
#                 SUBROUTINE: hhmmssFromSeconds                  #
#                                                                #
##################################################################
# subroutine hhmmssFromSeconds
sub hhmmssFromSeconds
   {
   $input_seconds = $_[0];

   $NBR_SECONDS_IN_MIN  = 60;
   $NBR_SECONDS_IN_HOUR = 3600;

   $seconds = $input_seconds%$NBR_SECONDS_IN_MIN;
   $remaining_seconds = $input_seconds - $seconds;
   $minutes = $remaining_seconds/$NBR_SECONDS_IN_MIN;
   $remaining_seconds = $remaining_seconds - ($minutes*$NBR_SECONDS_IN_MIN);
   $hours   = $remaining_seconds/$NBR_SECONDS_IN_HOUR;

   @return_array = ($hours, $minutes, $seconds);

   return @return_array;
   } # End of hhmmssFromSeconds


##################################################################
#                                                                #
#              SUBROUTINE: getCurrentGMTUnixTime                 #
#                                                                #
##################################################################
# subroutine getCurrentGmtUnixTime
sub getCurrentGmtUnixTime
   {
   $current_gmt_unix_time = `date -u '+%s'`;
   return $current_gmt_unix_time;
   } # End of getCurrentGmtUnixTime


##################################################################
#                                                                #
#              SUBROUTINE: unix_time_from_datetime               #
#                                                                #
##################################################################
# unix_time_from_datetime
#
# Calculates UNIX time from a conventional date and time string,
#  or a Julian date and time string.
#
# Usage: unix_time_from_datetime YYYYMMDD HHMMSS 
# Usage: unix_time_from_datetime '20010101' '210912'
# Usage: unix_time_from_datetime YYYYJJJ HHMMSS 
# Usage: unix_time_from_datetime '2001001' '210912'
#

sub unix_time_from_datetime
   {
   $nbr_secs_in_minute = 60;
   $nbr_secs_in_hour   = 60*$nbr_secs_in_minute;
   $nbr_secs_in_day = 24*$nbr_secs_in_hour;
   $nbr_secs_in_nonleap_year = 365*$nbr_secs_in_day;
   $nbr_secs_in_leap_year    = 366*$nbr_secs_in_day;

   $date = $_[0];
   $time = $_[1];

   $year   = substr($date,0,4);
   $hour   = substr($time,0,2);
   $minute = substr($time,2,2);
   $second = substr($time,4,2);

   if($year<1970)
      {
      #print("Invalid date (must be after 1970)\n", $year);
      $output_str = sprintf("Invalid date (%s) must be after 1970\n", $year);
      print($output_str);
      exit;
      }

   $date_size = length($date);

   if($date_size==7)
      {
      $date_type  = 'JULIAN';
      $julian_day = substr($date,4,3);
      }
   elsif($date_size==8)
      {
      $date_type = 'CONVENTIONAL';
      $month     = substr($date,4,2);
      $day       = substr($date,6,2);
      }
   else
      {
      print("Invalid date\n");
      exit;
      }

   # If date is conventional, convert to Julian
   if($date_type eq 'CONVENTIONAL')
      {
      $julian_date = DateRoutines::conventionalToJulian($year,$month,$day);
      $julian_day  = substr($julian_date,4,3);
      }

   $unix_time = 0;

   # Loop through years
   for($year_index=1970;$year_index<2038;$year_index++)
      {
      if($year_index==$year)
         {
         # This is the input year. Stop incrementing seconds by year and add
         #  seconds assocaited with month, day, hour, minute and second
         $unix_time = $unix_time + ($julian_day-1)*$nbr_secs_in_day;
         $unix_time = $unix_time + $hour*$nbr_secs_in_hour;
         $unix_time = $unix_time + $minute*$nbr_secs_in_minute;
         $unix_time = $unix_time + $second;
         return $unix_time;
         }
      else
         {
         # This is less than the input year. Add number of seconds in this year to total.
         if(DateRoutines::isLeapYear($year_index))
            {
            # Leap year
            $unix_time = $unix_time + $nbr_secs_in_leap_year;
            }
         else
            {
            # Non-leap year
            $unix_time = $unix_time + $nbr_secs_in_nonleap_year;
            }
         }
      }
   print("Error: UNIX time dates cannot exceed 2038\n");
   exit;
   } # End of unix_time_from_datetime


##################################################################
#                                                                #
#            SUBROUTINE: datetime_from_unix_time                 #
#                                                                #
##################################################################
sub datetime_from_unix_time
   {
   $nbr_secs_in_minute = 60;
   $nbr_secs_in_hour   = 60*$nbr_secs_in_minute;
   $nbr_secs_in_day = 24*$nbr_secs_in_hour;
   $nbr_secs_in_nonleap_year = 365*$nbr_secs_in_day;
   $nbr_secs_in_leap_year    = 366*$nbr_secs_in_day;

   $unix_time = $_[0];
   if($unix_time<0)
      {
      print("Invalid unix time input\n");
      exit;
      }

   $accumulated_seconds = 0;

   ######################
   #                    #
   # YEAR DETERMINATION #
   #                    #
   ######################
   # Loop through years
   for($year_index=1970;$year_index<2038;$year_index++)
      {
      if(DateRoutines::isLeapYear($year_index))
         {
         # Leap year
         #if($accumulated_seconds + $nbr_secs_in_leap_year>=$unix_time)
         if($accumulated_seconds + $nbr_secs_in_leap_year>$unix_time)
            {
            $year = $year_index;
            $remaining_seconds = $unix_time - $accumulated_seconds;
            last;
            }
         else
            {
            $accumulated_seconds = $accumulated_seconds + $nbr_secs_in_leap_year;
            }
         }
      else
         {
         # Non-leap year
         #if($accumulated_seconds + $nbr_secs_in_nonleap_year>=$unix_time)
         if($accumulated_seconds + $nbr_secs_in_nonleap_year>$unix_time)
            {
            $year = $year_index;
            $remaining_seconds = $unix_time - $accumulated_seconds;
            last;
            }
         else
            {
            $accumulated_seconds = $accumulated_seconds + $nbr_secs_in_nonleap_year;
            }
         }
      }

   # Year has been determined.
   ############################
   #                          #
   # JULIAN DAY DETERMINATION #
   #                          #
   ############################
   # Loop through days to determine Julian day
   if(DateRoutines::isLeapYear($year))
      {
      $last_day = 366;
      }
   else
      {
      $last_day = 365;
      }

   $accumulated_seconds = 0;
   for($day_index=1;$day_index<=$last_day;$day_index++)
      {
      if($accumulated_seconds + $nbr_secs_in_day > $remaining_seconds)
         {
         $julian_day = $day_index;
         $remaining_seconds = $remaining_seconds - $accumulated_seconds;
         last;
         }
      else
         {
         $accumulated_seconds = $accumulated_seconds + $nbr_secs_in_day;
         }
      }

   if($julian_day<10)
      {
      $julian_day = '00'.$julian_day;
      }
   elsif($julian_day<100)
      {
      $julian_day = '0'.$julian_day;
      }

   ######################
   #                    # 
   # HOUR DETERMINATION #
   #                    # 
   ######################
   
   $accumulated_seconds = 0;
   # Loop through hours
   for($hour_index=0;$hour_index<=23;$hour_index++)
      {
      #if($accumulated_seconds + $nbr_secs_in_hour >= $remaining_seconds)
      if($accumulated_seconds + $nbr_secs_in_hour > $remaining_seconds)
         {
         $hour = $hour_index;
         $remaining_seconds = $remaining_seconds - $accumulated_seconds;
         last;
         }
      else
         {
         $accumulated_seconds = $accumulated_seconds + $nbr_secs_in_hour;
         }
      }

   if($hour<10)
      {
      $hour = '0'.$hour;
      }

   ########################
   #                      # 
   # MINUTE DETERMINATION #
   #                      # 
   ########################
   
   $accumulated_seconds = 0;
   # Loop through minutes 
   for($minute_index=0;$minute_index<=59;$minute_index++)
      {
      if($accumulated_seconds + $nbr_secs_in_minute >= $remaining_seconds)
         {
         $minute = $minute_index;
         $remaining_seconds = $remaining_seconds - $accumulated_seconds;
         last;
         }
      else
         {
         $accumulated_seconds = $accumulated_seconds + $nbr_secs_in_minute;
         }
      }

   if($minute<10)
      {
      $minute = '0'.$minute;
      }

   # The number of seconds is equivalent to $remaining_seconds
   $second = $remaining_seconds;
   if($second<10)
      {
      $second = '0'.$second;
      }
   elsif($second == 60)
      {
      $second = '00';
      $minute = $minute + 1;
      if($minute < 10)
         {
         $minute = '0'.$minute;
         }
      elsif($minute == 60)
         {
         $minute = '00';
         $hour = $hour + 1;
         if($hour < 10)
            {
            $hour = '0'.$hour;
            }
         elsif($hour == 24)
            {
            $hour = '00';
            $julian_day = $julian_day + 1;
            if(DateRoutines::isLeapYear($year))
               {
               if($julian_day > 366)
                  {
                  $julian_day = '001';
                  $year = $year + 1;
                  }
               }
            else
               {
               if($julian_day > 365)
                  {
                  $julian_day = '001';
                  $year = $year + 1;
                  }
               } # End of leap year conditional
            } # End of if($hour == 24) conditional
         } # End of if($min == 60) conditional
      } # End of $seconds conditional

   $conventional_date = DateRoutines::julianToConventional($year,$julian_day);
   $conventional_time = $hour.$minute.$second;
   @return_array = ($conventional_date,$conventional_time);
   } # End of datetime_from_unix_time
