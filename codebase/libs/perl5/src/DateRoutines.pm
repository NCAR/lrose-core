# *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* */
# ** Copyright UCAR (c) 1992 - 2010 */
# ** University Corporation for Atmospheric Research(UCAR) */
# ** National Center for Atmospheric Research(NCAR) */
# ** Research Applications Laboratory(RAL) */
# ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA */
# ** 2010/10/7 23:12:43 */
# *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* */
package DateRoutines;

require "$ENV{'CVS_SRC_DIR'}/libs/perl5/src/TimeRoutines.pm";

# Contents:
#  conventionalToJulian
#  julianToConventional
#  isLeapYear
#  getCurrentDate
#  getCurrentHour
#  getCurrentGmtDate
#  getCurrentGmtHour
#  validDateCheck
#  getPreviousDay
#  getNextDay
#  getOffsetDateTime

##################################################################
#                                                                #
#                SUBROUTINE: CONVENTIONALTOJULIAN                #
#                                                                #
##################################################################
# conventionalToJulian subroutine

# Purpose:
#  Converts a date of conventional format YYYYMMDD (e.g., 20021025) to a
#   Julian format YYYYJJJ (e.g., 2002298).
sub conventionalToJulian
   {
   $module_name = "DateRoutines::conventionalToJulian";
   $bad_exit_code = -9999;

   @non_leap_year_start_days = (1,32,60,91,121,152,182,213,244,274,305,335);
   @leap_year_start_days =     (1,32,61,92,122,153,183,214,245,275,306,336);

   if(length($_[0]) == 8)
      {
      # Assume that input date is represented as a single YYYYMMDD argument.
      $yyyymmdd = $_[0];
      $year  = substr($yyyymmdd,0,4);
      $month = substr($yyyymmdd,4,2);
      $day   = substr($yyyymmdd,6,2);
      }
   else
      {
      # Assume that input date is represented as separate year, month and day arguments.
      $year = $_[0];
      $month = $_[1];
      $day = $_[2];
      }

   if(validDateCheck($year,$month,$day) != 1)
      {
      printf("%s: Invalid Date\n", $module_name);
      exit;
      }

   if (isLeapYear($year) == 1)
      {
      # Leap year
      $index = $month-1;
      $julian_day = @leap_year_start_days[$index] + $day - 1;
      }
   else
      {
      # Non-leap year
      $index = $month-1;
      $julian_day = @non_leap_year_start_days[$index] + $day - 1;
      }

   # Pad Julian day with leading zeroes if necessary
   if($julian_day < 10)
      {
      $julian_day = '00' . $julian_day;
      }
   elsif($julian_day < 100)
      {
      $julian_day = '0' . $julian_day;
      }
   $julian_day_and_year = $year . $julian_day;
   return $julian_day_and_year;
   }


##################################################################
#                                                                #
#                SUBROUTINE: JULIANTOCONVENTIONAL                #
#                                                                #
##################################################################
# julianToConventional subroutine

# Purpose:
#  Converts a date of Julian format YYYYJJJ (e.g., 2002298) to a
#   conventional format YYYYMMDD (e.g., 20021025).

sub julianToConventional
   {
   $module_name = "DateRoutines::julianToConventional";
   $bad_exit_code = -9999;

   @non_leap_year_start_days = (1,32,60,91,121,152,182,213,244,274,305,335);
   @leap_year_start_days =     (1,32,61,92,122,153,183,214,245,275,306,336);

   if(length($_[0]) == 7)
      {
      # Assume that input date is represented as single YYYYJJJJ argument.
      $year       = substr($_[0],0,4);
      $julian_day = substr($_[0],4,3);
      }
   else
      {
      # Assume that input date is represented as separate year and day arguments.
      $year = $_[0];
      $julian_day = $_[1];
      }

   # Verify bounds on Julian date
   if (($julian_day > 366) || ($julian_day < 1))
      {
      print "Julian day must be >= 1 and <= 366 (<= 365 for non-leap year)\n";
      return $bad_exit_code;
      }

   if(isLeapYear($year) == 1)
      {
      for($month=1; $julian_day>=@leap_year_start_days[$month] && $month<=11; $month++) {}
      $day = $julian_day - @leap_year_start_days[$month-1] + 1;
      # Pad day and month with leading zero if necessary
      if($day < 10)
         {
         $day = '0' . $day;
         }
      if($month < 10)
         {
         $month = '0' . $month;
         }
      $month_and_day = $month . $day;
      $year_month_and_day = $year . $month_and_day;
      return $year_month_and_day;
      }
   else
      {
      for($month=1; $julian_day>=@non_leap_year_start_days[$month] && $month<=11; $month++) {}
      $day = $julian_day - @non_leap_year_start_days[$month-1] + 1;
      if($day < 10)
         {
         $day = '0' . $day;
         }
      if($month < 10)
         {
         $month = '0' . $month;
         }
      $month_and_day = $month . $day;
      $year_month_and_day = $year . $month_and_day;
      return $year_month_and_day;
      }
   }


##################################################################
#                                                                #
#                     SUBROUTINE: ISLEAPYEAR                     #
#                                                                #
##################################################################
# isLeapYear subroutine

# Purpose:
#  Returns '1' if input argument is a leap year, '0' otherwise.
sub isLeapYear
   {
   $year = $_[0];
   if (($year%400 == 0) || (($year%4 == 0) && ($year%100 != 0)))
      {
      # Leap year
      return 1;
      }
   else
      {
      # Non-leap year
      return 0;
      }
   }


##################################################################
#                                                                #
#                  SUBROUTINE: GETCURRENTDATE                    #
#                                                                #
##################################################################
# getCurrentDate subroutine

# Purpose:
#  Returns the current date in YYYYMMDD format.

sub getCurrentDate
   {
   $year_current  = `date '+%Y'`;
   chop($year_current);
   $month_current = `date '+%m'`;
   chop($month_current);
   $day_current   = `date '+%d'`;
   chop($day_current);
   $yyyymmdd_current = $year_current . $month_current . $day_current;
   return $yyyymmdd_current;
   }


##################################################################
#                                                                #
#                  SUBROUTINE: GETCURRENTHOUR                    #
#                                                                #
##################################################################
# getCurrentHour subroutine

# Purpose:
#  Returns the current hour in HH format (pads with leading zero for single-digit times).

sub getCurrentHour
   {
   $hour_current = `date '+%H'`;
   chop($hour_current);
   if(length($hour_current) == 1)
      {
      $hour_current = '0' . $hour_current;
      }
   return $hour_current;
   }


##################################################################
#                                                                #
#                SUBROUTINE: GETCURRENTGMTDATE                   #
#                                                                #
##################################################################
# getCurrentGmtDate subroutine

# Purpose:
#  Returns the current GMT date in YYYYMMDD format.

sub getCurrentGmtDate
   {
   $year_gmt  = `date -u '+%Y'`;
   chop($year_gmt);
   $month_gmt = `date -u '+%m'`;
   chop($month_gmt);
   $day_gmt   = `date -u '+%d'`;
   chop($day_gmt);
   $yyyymmdd_gmt = $year_gmt . $month_gmt . $day_gmt;
   return $yyyymmdd_gmt;
   }


##################################################################
#                                                                #
#                SUBROUTINE: GETCURRENTGMTHOUR                   #
#                                                                #
##################################################################
# getCurrentGmtHour subroutine

# Purpose:
#  Returns the current GMT hour in HH format (pads with leading zero for single-digit times).

sub getCurrentGmtHour
   {
   $hour_gmt = `date -u '+%H'`;
   chop($hour_gmt);
   if(length($hour_gmt) == 1)
      {
      $hour_gmt = '0' . $hour_gmt;
      }
   return $hour_gmt;
   }


##################################################################
#                                                                #
#                  SUBROUTINE: GETPREVIOUSDAY                    #
#                                                                #
##################################################################
# getPreviousDay subroutine
# Returns the date corresponding to the day before the argument date
# Note that this is a non-trivial operation due to the possiblity of
#  month and year boundaries
#
# Input argument must be single-string conventional date (YYYYMMDD format)

sub getPreviousDay
   {
   $today = $_[0];

   # If the date is Jan 1, a year boundary has been crossed
   if(substr($today,4,4) eq '0101')
      {
      # Year boundary has been crossed. Previous day must be Dec 31 of previous year.
      $year = substr($today,0,4);
      $previous_year = $year - 1;
      $previous_day  = '1231';
      $previous_date = $previous_year . $previous_day;
      }
   else
      {
      # No year boundary complications. Convert to julian format, subtract one,
      #  then convert back to conventional format.
      $today_julian = conventionalToJulian($today);
      $yesterday_julian = $today_julian - 1;
      $previous_date = julianToConventional($yesterday_julian);
      }
   return $previous_date;
   }


##################################################################
#                                                                #
#                  SUBROUTINE: GETNEXTDAY                        #
#                                                                #
##################################################################
# getNextDay subroutine
# Returns the date corresponding to the day after the argument date
# Note that this is a non-trivial operation due to the possiblity of
#  month and year boundaries
#
# Input argument must be single-string conventional date (YYYYMMDD format)

sub getNextDay
   {
   $today = $_[0];

   # If the date is Dec 31, a year boundary has been crossed
   if(substr($today,4,4) eq '1231')
      {
      # Year boundary has been crossed. Next day must be Jan 1 of previous year.
      $year = substr($today,0,4);
      $next_year = $year + 1;
      $next_day  = '0101';
      $next_date = $next_year . $next_day;
      }
   else
      {
      # No year boundary complications. Convert to julian format, add one,
      #  then convert back to conventional format.
      $today_julian = conventionalToJulian($today);
      $tomorrow_julian = $today_julian + 1;
      $next_date = julianToConventional($tomorrow_julian);
      }
   return $next_date;
   }


##################################################################
#                                                                #
#                SUBROUTINE: GETOFFSETDATETIME                   #
#                                                                #
##################################################################
# getOffsetDateTime subroutine
# Returns the date and time corresponding to a specified number of
#  before the argument date and time.
# Note that this is a non-trivial operation due to the possiblity of
#  day, month and year boundaries.
#
# Input argument must be single-string conventional base date
#  (YYYYMMDD format), base time (HHMMSS format), and offset time
#  (HHMMSS format).
#
# Note - negative offset time indicates past, positive offset time
#  indicates future.

sub getOffsetDateTime
   {
   $NBR_SECONDS_IN_MIN  = 60;
   $NBR_SECONDS_IN_HOUR = 3600;

   $module_name = "DateRoutines::conventionalToJulian";

   $base_date   = $_[0];
   $base_time   = $_[1];
   $offset_time = $_[2];

   # Validate input base year
   if(validDateCheck($base_date) != 1)
      {
      printf("\n%s: Invalid base date\n", $module_name);
      exit;
      }

   # Validate input base time
   if(length($base_time) != 6)
      {
      printf("\n%s: Invalid base time (excessive length)\n", $module_name);
      exit;
      }

   $base_hour = substr($base_time,0,2);
   $base_min  = substr($base_time,2,2);
   $base_sec  = substr($base_time,4,2);
   if(($base_hour < 0) || ($base_hour > 23))
      {
      printf("\n%s: Invalid base time (hour is out of bounds)\n", $module_name);
      exit;
      }
   if(($base_min < 0) || ($base_min > 59))
      {
      printf("\n%s: Invalid base time (minute is out of bounds)\n", $module_name);
      exit;
      }
   if(($base_sec < 0) || ($base_sec > 59))
      {
      printf("\n%s: Invalid base time (second is out of bounds)\n", $module_name);
      exit;
      }

   # Validate input offset time
   if(length($offset_time) == 7)
      {
      if(substr($offset_time,0,1) ne '-')
         {
         printf("\n%s: Invalid offset time (first character of seven-digit time must be negative sign)\n", $module_name);
         exit;
         }
      else
         {
         $offset_direction = 'PAST';
         $offset_hour = substr($offset_time,1,2);
         $offset_min  = substr($offset_time,3,2);
         $offset_sec  = substr($offset_time,5,2);
         }
      }
   elsif(length($offset_time) == 6)
      {
      $offset_direction = 'FUTURE';
      $offset_hour = substr($offset_time,0,2);
      $offset_min  = substr($offset_time,2,2);
      $offset_sec  = substr($offset_time,4,2);
      }
   else
      {
      printf("\n%s: Invalid base time (invalid length)\n", $module_name);
      exit;
      }

   if(($offset_min < 0) || ($offset_min > 59))
      {
      printf("\n%s: Invalid offset time (minute is out of bounds)\n", $module_name);
      exit;
      }
   if(($offset_sec < 0) || ($offset_sec > 59))
      {
      printf("\n%s: Invalid offset time (second is out of bounds)\n", $module_name);
      exit;
      }

   # Determine offset date and time as follows
   #  1) Convert base date and time to unix time
   #  2) Convert offset time to seconds
   #  3) Add (or subtract) offset seconds to (or from) unix time
   #  4) Convert result to conventional date and time

   $base_time_unix = TimeRoutines::unix_time_from_datetime($base_date,$base_time);
   $offset_in_secs = $NBR_SECONDS_IN_HOUR*$offset_hour + $NBR_SECONDS_IN_MIN*$offset_min + $offset_sec;

   if($offset_direction eq 'PAST')
      {
      $offset_time_unix = $base_time_unix - $offset_in_secs;
      }
   else
      {
      $offset_time_unix = $base_time_unix + $offset_in_secs;
      }

   @offset = TimeRoutines::datetime_from_unix_time($offset_time_unix);
   return @offset;
   }


##################################################################
#                                                                #
#                  SUBROUTINE: VALIDDATECHECK                    #
#                                                                #
##################################################################
# validDateCheck subroutine
# Date can be entered as year, month, day or YYYYMMDD
# validDateCheck(2000,12,31) or validDateCheck(20001231)
sub validDateCheck
   {
   if(length($_[0]) == 8)
      {
      # Date submitted as single YYYYMMDD value
      $yyyymmdd = $_[0];
      $year  = substr($yyyymmdd,0,4);
      $month = substr($yyyymmdd,4,2);
      $day   = substr($yyyymmdd,6,2);
      }
   else
      {
      # Assume date submitted as separate YYYY, MM, and DD values
      $year  = $_[0];
      $month = $_[1];
      $day   = $_[2];
      }

   # Initialize return code
   $return_code = 1;

   # Check bounds on number of digits
   if(length($year)!=4)
      {
      $return_code = 0;
      return $return_code;
      }
   if(length($month)!=2)
      {
      $return_code = 0;
      return $return_code;
      }
   if(length($day)!=2)
      {
      $return_code = 0;
      return $return_code;
      }

   # Remove padding, if necessary
   if(substr($month,0,1) eq '0')
      {
      $month = substr($month,1,1);
      }
   if(substr($day,0,1) eq '0')
      {
      $day = substr($day,1,1);
      }

   @nbr_days_in_month = (31,28,31,30,31,30,31,31,30,31,30,31);

   # Check for leap year
   if (($year%400 == 0) || (($year%4 == 0) && ($year%100 != 0)))
      {
      $leap_year_flag = 1;
      }
   else
      {
      $leap_year_flag = 0;
      }

   if(($month<1) || ($month>12))
      {
      $return_code = 0;
      return $return_code;
      }

   if($day<1)
      {
      $return_code = 0;
      return $return_code;
      }

   if(($leap_year_flag == 1) && ($month==2))
      {
      if($day<30)
         {
         $return_code = 1;
         return $return_code;
         }
      }

   if($day > $nbr_days_in_month[$month-1])
      {
      $return_code = 0;
      return $return_code;
      }

   return $return_code;
   }

1;
