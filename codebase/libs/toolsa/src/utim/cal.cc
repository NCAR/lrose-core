// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* 
// ** Copyright UCAR (c) 1990 - 2016                                         
// ** University Corporation for Atmospheric Research (UCAR)                 
// ** National Center for Atmospheric Research (NCAR)                        
// ** Boulder, Colorado, USA                                                 
// ** BSD licence applies - redistribution and use in source and binary      
// ** forms, with or without modification, are permitted provided that       
// ** the following conditions are met:                                      
// ** 1) If the software is modified to produce derivative works,            
// ** such modified software should be clearly marked, so as not             
// ** to confuse it with the version available from UCAR.                    
// ** 2) Redistributions of source code must retain the above copyright      
// ** notice, this list of conditions and the following disclaimer.          
// ** 3) Redistributions in binary form must reproduce the above copyright   
// ** notice, this list of conditions and the following disclaimer in the    
// ** documentation and/or other materials provided with the distribution.   
// ** 4) Neither the name of UCAR nor the names of its contributors,         
// ** if any, may be used to endorse or promote products derived from        
// ** this software without specific prior written permission.               
// ** DISCLAIMER: THIS SOFTWARE IS PROVIDED "AS IS" AND WITHOUT ANY EXPRESS  
// ** OR IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED      
// ** WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.    
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* 

////////////////////////////////////////////////////////////////////////
// CAL.C  Calandar functions
// Randy Bullock (c) 2003  RAP/NCAR
//
// Note:  Functions use C++ variable References. -  Compile with g++


#include <cstdlib>
#include <cmath>
#include <iostream>
#include <toolsa/cal.hh>
using namespace std;

////////////////////////////////////////////////////////////////////////


   //
   //  definitions with external linkage
   //


const char *day_name[] = {

   "Sunday",    //  0 - not used

   "Monday",    //  1
   "Tuesday",   //  2
   "Wednesday", //  3
   "Thursday",  //  4
   "Friday",    //  5
   "Saturday",  //  6
   "Sunday"     //  7

 };


const char *short_day_name[] = {

   "Sun",   //  0 - not used

   "Mon",   //  1
   "Tue",   //  2
   "Wed",   //  3
   "Thu",   //  4
   "Fri",   //  5
   "Sat",   //  6
   "Sun"    //  7

 };


const char *month_name[] = {

   "December",      //   0 - not used

   "January",       //   1
   "February",      //   2
   "March",         //   3
   "April",         //   4
   "May",           //   5
   "June",          //   6
   "July",          //   7
   "August",        //   8
   "September",     //   9
   "October",       //  10
   "November",      //  11
   "December"       //  12

};


const char *short_month_name[] = {

   "Dec",    //   0 - not used

   "Jan",    //   1
   "Feb",    //   2
   "Mar",    //   3
   "Apr",    //   4
   "May",    //   5
   "Jun",    //   6
   "Jul",    //   7
   "Aug",    //   8
   "Sep",    //   9
   "Oct",    //  10
   "Nov",    //  11
   "Dec"     //  12

};


////////////////////////////////////////////////////////////////////////


   //
   //  definitions with static linkage
   //

struct DstInfo {

   int month_start;
   int day_start;

   int month_stop;
   int day_stop;

};


static DstInfo dst_info[] = {

  { 4, 2, 10, 29 },    //   0
  { 4, 1, 10, 28 },    //   1

  { 4, 7, 10, 27 },    //   2
  { 4, 6, 10, 26 },    //   3

  { 4, 5, 10, 25 },    //   4
  { 4, 4, 10, 31 },    //   5

  { 4, 3, 10, 30 },    //   6
  { 4, 1, 10, 28 },    //   7

  { 4, 7, 10, 27 },    //   8
  { 4, 6, 10, 26 },    //   9

  { 4, 5, 10, 25 },    //  10
  { 4, 4, 10, 31 },    //  11

  { 4, 3, 10, 30 },    //  12
  { 4, 2, 10, 29 }     //  13

};


////////////////////////////////////////////////////////////////////////


   //
   //  Code for 32-bit stuff
   //


////////////////////////////////////////////////////////////////////////


int mdyhms_to_unix(int month, int day, int year, int hour, int minute, int second)

{

int b, g, mjd, answer;


b = (month - 14)/12;

g = year + 4900 + b;

b = month - 2 - 12*b;

mjd = (1461*(g - 100))/4 + (367*b)/12 - (3*(g/100))/4 + day - 2432076;

answer = 86400*(mjd - 40587) + 3600*hour + 60*minute + second;


return ( answer );

}


////////////////////////////////////////////////////////////////////////


void unix_to_mdyhms(int u, int &month, int &day, int &year, int &hour, int &minute, int &second)

{

int i, j, n, l, d, m, y;


n = u/86400;

u -= 86400*n;

if ( u < 0 )  { u += 86400; --n; }

l  = n + 2509157;

n  = (4*l)/146097;

l -= (146097*n + 3)/4;

i  = (4000*(l + 1))/1461001;

l += 31 - (1461*i)/4;

j  = (80*l)/2447;

d  = l - (2447*j)/80;

l  = j/11;

m  = j + 2 - 12*l;

y  = 100*(n - 49) + i + l;

month   = m;
day     = d;
year    = y;

hour    =      u/3600;
minute  = (u%3600)/60;
second  =        u%60;

return;

}


////////////////////////////////////////////////////////////////////////


int is_dst(int standard_time)

{

int j, k;
int month, day, year, hour, minute, second;


unix_to_mdyhms(standard_time, month, day, year, hour, minute, second);

   //
   //  find day of the month for the
   //    first sunday in april
   //

switch ( day_of_week(4, 1, year) )  {

   case 1:   j = 7;  break;   //  monday
   case 2:   j = 6;  break;   //  tuesday
   case 3:   j = 5;  break;   //  wednesday
   case 4:   j = 4;  break;   //  thursday
   case 5:   j = 3;  break;   //  friday
   case 6:   j = 2;  break;   //  saturday
   case 7:   j = 1;  break;   //  sunday

   default:
     //      cerr << "\n\n  is_dst() -> bad weekday (a)\n\n";
      exit ( 1 );
      break;

}

k = mdyhms_to_unix(4, j, year, 2, 0, 0);

if ( standard_time < k )  return ( 0 );

   //
   //  find day of the month for the
   //    last sunday in october
   //

switch ( day_of_week(10, 1, year) )  {

   case 1:   j = 28;  break;   //  monday
   case 2:   j = 27;  break;   //  tuesday
   case 3:   j = 26;  break;   //  wednesday
   case 4:   j = 25;  break;   //  thursday
   case 5:   j = 31;  break;   //  friday
   case 6:   j = 30;  break;   //  saturday
   case 7:   j = 29;  break;   //  sunday

   default:
     // cerr << "\n\n  is_dst() -> bad weekday (a)\n\n";
      exit ( 1 );
      break;

}

k = mdyhms_to_unix(10, j, year, 2, 0, 0);

if ( standard_time > k )  return ( 0 );

return ( 1 );

}


////////////////////////////////////////////////////////////////////////


int doy_to_unix(int doy, int year, int hour, int minute, int second)

{

int jd, month, day, answer;

jd = date_to_mjd(1, 0, year) + doy;
mjd_to_date(jd, month, day, year);
answer = mdyhms_to_unix(month, day, year, hour, minute, second);

return ( answer );

}


////////////////////////////////////////////////////////////////////////


   //
   //  Code for 64-bit stuff
   //


////////////////////////////////////////////////////////////////////////


unixtime mdyhms_to_unix_64(int month, int day, int year, int hour, int minute, int second)

{

unixtime b, g, mjd, answer;


b = (month - 14)/12;

g = year + 4900 + b;

b = month - 2 - 12*b;

mjd = (1461*(g - 100))/4 + (367*b)/12 - (3*(g/100))/4 + day - 2432076;

answer = 86400*(mjd - 40587) + 3600*hour + 60*minute + second;


return ( answer );

}


////////////////////////////////////////////////////////////////////////


void unix_to_mdyhms_64(unixtime u, int &month, int &day, int &year, int &hour, int &minute, int &second)

{

unixtime i, j, n, l, d, m, y;


n = u/86400;

u -= 86400*n;

if ( u < 0 )  { u += 86400; --n; }

l  = n + 2509157;

n  = (4*l)/146097;

l -= (146097*n + 3)/4;

i  = (4000*(l + 1))/1461001;

l += 31 - (1461*i)/4;

j  = (80*l)/2447;

d  = l - (2447*j)/80;

l  = j/11;

m  = j + 2 - 12*l;

y  = 100*(n - 49) + i + l;

month   = m;
day     = d;
year    = y;

hour    =      u/3600;
minute  = (u%3600)/60;
second  =        u%60;

return;

}


////////////////////////////////////////////////////////////////////////


int is_dst(int month, int day, int year, int hour, int minute, int second)

{

int mjd0, ly, year_type;
unixtime T, dst_start, dst_stop;


ly = is_leap_year(year);

mjd0 = date_to_mjd(1, 1, year);

year_type = 7*ly + (mjd0 + 3)%7;


T = mdyhms_to_unix_64(month, day, year, hour, minute, second);


dst_start = mdyhms_to_unix_64(dst_info[year_type].month_start,
                              dst_info[year_type].day_start,
                              year, 2, 0, 0);

if ( T < dst_start )  return ( 0 );



dst_stop  = mdyhms_to_unix_64(dst_info[year_type].month_stop,
                              dst_info[year_type].day_stop,
                              year, 2, 0, 0);

if ( T > dst_stop )  return ( 0 );



return ( 1 );

}


////////////////////////////////////////////////////////////////////////


int is_dst(unixtime T)

{

int a;
int month, day, year, hour, minute, second;

unix_to_mdyhms_64(T, month, day, year, hour, minute, second);

a = is_dst(month, day, year, hour, minute, second);

return ( a );

}


////////////////////////////////////////////////////////////////////////


   //
   //  Code for misc routines
   //


////////////////////////////////////////////////////////////////////////


int day_of_week(int month, int day, int year)

{

int w = date_to_mjd(month, day, year);

if ( w < 0 )  w -= 7*(w/7 - 1);

w = 1 + (w + 2)%7;

   //
   //  1 = Mon, 2 = Tue, 3 = Wed, 4 = Thu, 5 = Fri, 6 = Sat, 7 = Sun
   //

return ( w );

}


////////////////////////////////////////////////////////////////////////


int day_dif(int month1, int day1, int year1, int month2, int day2, int year2)

{

return ( date_to_mjd(month2, day2, year2) - date_to_mjd(month1, day1, year1) );

}


////////////////////////////////////////////////////////////////////////


int date_to_mjd(int month, int day, int year)

{

int g, b, answer;


b = (month - 14)/12;

g = year + 4900 + b;

b = month - 2 - 12*b;

answer = (1461*(g - 100))/4 + (367*b)/12 - (3*(g/100))/4 + day - 2432076;


return ( answer );

}


////////////////////////////////////////////////////////////////////////


void mjd_to_date(int mjd, int &month, int &day, int &year)

{

int i, j, n, l, d, m, y;


l  = mjd + 2468570;

n  = (4*l)/146097;

l -= (146097*n + 3)/4;

i  = (4000*(l + 1))/1461001;

l += 31 - (1461*i)/4;

j  = (80*l)/2447;

d  = l - (2447*j)/80;

l  = j/11;

m  = j + 2 - 12*l;

y  = 100*(n - 49) + i + l;

month = m;
day   = d;
year  = y;

return;

}


////////////////////////////////////////////////////////////////////////


void easter(int &month, int &day, int year)

{

int c, d, i, j, k, l, m, n, y;


y  = year;

c  = y/100;

n  = y - 19*(y/19);

k  = (c - 17)/25;

i  = c - c/4 - (c - k)/3 + 19*n + 15;

i -= 30*(i/30);

i -= (i/28)*(1 - (i/28)*(29/(i + 1))*((21 - n)/11));

j  = y + y/4 + i + 2 - c + c/4;

j -= 7*(j/7);

l  = i - j;

m  = 3 + (l + 40)/44;

d  = l + 28 - 31*(m/4);

month = m;
day   = d;

return;

}


////////////////////////////////////////////////////////////////////////


int is_leap_year(int year)

{

year = abs(year);

if ( year%4 )  return ( 0 );

if ( year%100 )  return ( 1 );

if ( year%400 )  return ( 0 );

return ( 1 );

}


////////////////////////////////////////////////////////////////////////


