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
/* 	$Id: dd_utils.cc,v 1.6 2016/03/03 18:45:06 dixon Exp $	 */


# include <stdio.h>
# include <stdlib.h>
# include <unistd.h>
# include <string.h>
# include <sys/types.h>
# include <math.h>

# include <radar/dorade/dd_utils.hh>

extern int LittleEndian;

extern "C"
{
    int dd_tokens(char *, char **);
    int dd_tokenz(char *, char **, const char *);
}
// c---------------------------------------------------------------------------

double AngDiff(float a1, float a2)
{
    double d = a2 - a1;

    if( d < -270. ) { return(d + 360.); }
    if( d >  270. ) { return(d - 360.); }
    return(d);
}

// c---------------------------------------------------------------------------

inline int inSector(float ang, float ang1, float ang2)
{
    // assumes sector defined from ang1 clockwise to ang2

    if(ang1 > ang2)
	{ return(ang >= ang1 || ang < ang2); }

    return(ang >= ang1 && ang < ang2);
}

// c---------------------------------------------------------------------------

int piraqPktXfer::Xfer( char *srs, char *dst, int nbytes )
{
    // this routine transfers and swaps whole 16-bit words
    // even if the number of bytes is odd

    int n16 = 0;
    int nb = nbytes;
    unsigned char *aa = (unsigned char *)srs;
    unsigned char *bb = (unsigned char *)dst;
    if( residual) {
	nb--;
	n16++;
	if (LittleEndian)
	{
	// needs to be swapped since piraq is Little Endian
	    *bb++ = *aa++;
	    *bb++ = residualVal;
	}
	else
	{
	    *bb++ = residualVal;
	    *bb++ = *aa++;
	}
    }
    for(; nb > 1; nb -= 2, n16++, aa += 2) {
	if (LittleEndian)
	{
	    *bb++ = *(aa +1);
	    *bb++ = *aa;
	}
	else
	{
	    *bb++ = *aa;
	    *bb++ = *(aa +1);
	}
    }
    if(nb) {
	residualVal = *aa;
	residual = 1;
    }
    else {
	residual = 0;
    }
    return n16;
}

// c---------------------------------------------------------------------------


// DD_Time class


// c---------------------------------------------------------------------------

void DD_Time::reset()
{
    the_time_stamp = 0;
    the_additional_seconds = 0;
    the_julian_day = 0;
    the_year = 0;
    the_month = 0;
    the_day = 0;
    the_hour = 0;
    the_minute = 0;
    the_second = 0;
    the_millisecond = 0;
}

// c---------------------------------------------------------------------------

DD_Time::DD_Time()
{
    int ii = 0;
    int t28=2419200, t29=2505600, t30=2592000, t31=2678400; // secs in months
    
    /*
     * Brute force accumulated seconds per month initialization
     */
    reg_year[ii] = leap_year[ii] = 0;
    ii++;			/* jan */
    reg_year[ii] = reg_year[ii-1] + t31;
    leap_year[ii] = leap_year[ii-1] + t31;
    ii++;			/* feb */
    reg_year[ii] = reg_year[ii-1] + t28;
    leap_year[ii] = leap_year[ii-1] + t29;
    ii++;			/* mar */
    reg_year[ii] = reg_year[ii-1] + t31;
    leap_year[ii] = leap_year[ii-1] + t31;
    ii++;			/* apr */
    reg_year[ii] = reg_year[ii-1] + t30;
    leap_year[ii] = leap_year[ii-1] + t30;
    ii++;			/* may */
    reg_year[ii] = reg_year[ii-1] + t31;
    leap_year[ii] = leap_year[ii-1] + t31;
    ii++;			/* jun */
    reg_year[ii] = reg_year[ii-1] + t30;
    leap_year[ii] = leap_year[ii-1] + t30;
    ii++;			/* jul */
    reg_year[ii] = reg_year[ii-1] + t31;
    leap_year[ii] = leap_year[ii-1] + t31;
    ii++;			/* aug */
    reg_year[ii] = reg_year[ii-1] + t31;
    leap_year[ii] = leap_year[ii-1] + t31;
    ii++;			/* sep */
    reg_year[ii] = reg_year[ii-1] + t30;
    leap_year[ii] = leap_year[ii-1] + t30;
    ii++;			/* oct */
    reg_year[ii] = reg_year[ii-1] + t31;
    leap_year[ii] = leap_year[ii-1] + t31;
    ii++;			/* nov */
    reg_year[ii] = reg_year[ii-1] + t30;
    leap_year[ii] = leap_year[ii-1] + t30;
    ii++;			/* dec */
    reg_year[ii] = reg_year[ii-1] + t31;
    leap_year[ii] = leap_year[ii-1] + t31;
    
    int yy;
    accumulated_years[0] = 0;
    
    for(ii = 0, yy = Reference_Year; ii <= Max_Years; ii++, yy++ ) {
	accumulated_months[ii] = yy % 4 ? reg_year : leap_year;
	if(ii) {
	    accumulated_years[ii] = accumulated_years[ii-1] +
		  *(accumulated_months[ii-1] + 12);
	}
    }

 }

// c---------------------------------------------------------------------------

DTime DD_Time::stamp_the_time()
{
    if(the_year < Reference_Year || the_year >= Reference_Year + Max_Years)
	  return(Bad_Time);

    int ii = the_year - Reference_Year;

    int mm = the_month > 0 ? the_month - 1 : 0;
    int dd = the_day > 0 ? the_day - 1 : 0;

    the_time_stamp = accumulated_years[ii]
	  + (*(accumulated_months[ii] + mm)) + dd * the_Seconds_Per_Day
		+ the_additional_seconds;

    return(the_time_stamp);
}

// c---------------------------------------------------------------------------

void DD_Time::unstamp_the_time()
{
    DTime dt = the_time_stamp;

    if(dt < 0 || dt > accumulated_years[Max_Years]) {
	the_year= -1;       
	the_month= -1;      
	the_day= -1;        
	the_hour= -1;       
	the_minute= -1;     
	the_second= -1;     
	the_millisecond= -1;
	return;
    }
    DTime fudge = 0;
    int idt = (int)dt;

    if(!(dt - (double)idt)) {	// no milliseconds
	// if you're at exactly midnight
	// the hours will come out as 24...sigh

	if(!(idt % the_Seconds_Per_Day)) { // exactly midnight
	    fudge = 1.;		// or else the day numbers will be wrong
	}
    }
    dt += fudge;
    int ii = Wild_Guess - Reference_Year; // see if we can save some looping

    if(dt <= accumulated_years[ii]) { ii = 0; }

    for(; ii <= Max_Years && accumulated_years[ii] <= dt; ii++ );

    DTime *mon_ptr = accumulated_months[--ii];
    the_year = Reference_Year + ii;
    dt -= accumulated_years[ii];

    the_julian_day = (int)(dt * Seconds_To_Days + 1.);

    ii = *(mon_ptr + 5) < dt ? 5 : 0; // try to save some looping again
    for(; ii < 13; ii++) {
	if(*(mon_ptr + ii) > dt)
	      break;
    }
    the_month = ii--;
    dt -= *(mon_ptr + ii);
    the_day = (int)(dt * Seconds_To_Days);
    dt -= ((the_day++) * the_Seconds_Per_Day);

    dt -= fudge;
    the_hour = (int)(dt * Seconds_To_Hours);
    dt -= 3600. * the_hour;
    the_minute = (int)(dt * Seconds_To_Minutes);
    dt -= 60. * the_minute;
    the_second = (int)dt;
    dt -= the_second;
    the_millisecond = (int)(1000. * dt +.5);

}

// c---------------------------------------------------------------------------

char *DD_Time::ascii_time()
{
    this->unstamp_the_time();

    sprintf(the_ascii_time, "%02d/%02d/%02d %02d:%02d:%02d.%03d"
	   , the_month
	   , the_day
	   , the_year - 1900
	   , the_hour
	   , the_minute
	   , the_second
	   , the_millisecond
	   );
    return(the_ascii_time);
}

// c---------------------------------------------------------------------------

char *DD_Time::ascii_numeric_time()
{
    this->unstamp_the_time();

    sprintf(the_ascii_time, "%d%02d%02d%02d%02d%02d"
	   , the_year - 1900
	   , the_month
	   , the_day
	   , the_hour
	   , the_minute
	   , the_second
	   );
    return(the_ascii_time);
}

/* c------------------------------------------------------------------------ */

int
dd_CrackDatime(char * datime, int nch, DD_Time * ddt)
{
    int yy=0, mon=0, dd=0, hh=0, mm=0, ss=0, ms=0;
    int ii, nt, ns, nc, just_a_number = NO;
    char str[32], *str_ptrs[16], *aa, *bb, *cc;


    strncpy(str, datime, nch);
    str[nch] = '\0';
    for(aa=str; *aa && (*aa == ' ' || *aa == '\t'); aa++);
    if((*aa == '+' || *aa == '-')) {
	return(NO);		/* relative */
    }
    just_a_number = !strstr( str, "/" ) && !strstr( str, ":" );

    if((nt = dd_tokenz( str, str_ptrs, "." )) > 1 ) {
	// assume the presence of milliseconds
	ms = atoi( str_ptrs[1] );
    }
    bb = cc = str;

    if( ( ns = dd_tokenz( str, str_ptrs, "/" )) == 3 ) { // "mon/day/year"
	mon = atoi( str_ptrs[0] );
	dd = atoi( str_ptrs[1] );
	bb = str_ptrs[2];
    }
    else if( ns > 1 ) { // one slash or more than two
	return( NO );
    }

    if( ( nc = dd_tokenz( bb, str_ptrs, ":" )) > 1 ) {
	if( ns == 3 ) {	// assumes a colon seperater between date and time
	    yy = atoi( str_ptrs[0] );
	    hh = atoi( str_ptrs[1] );
	    if( nc == 2 ) {
		cc = str_ptrs[1];
		just_a_number = YES;
	    }
	    if( nc > 2 ) {
		mm = atoi( str_ptrs[2] );
	    }
	    if( nc > 3 ) {
		ss = atoi( str_ptrs[3] );
	    }
	}
	else {
	    hh = atoi( str_ptrs[0] );
	    mm = atoi( str_ptrs[1] );
	    if( nc > 2 ) {
		ss = atoi( str_ptrs[2] );
	    }
	}
    }
    else if( ns == 3 ) {	// just a date
	yy = atoi( str_ptrs[2] );
    }

    if( just_a_number &&
	sscanf(cc, "%d", &ii)) { // it's just a number like 2100 or even 0000

	if( !ii ) {
	}
	else if(!strncmp(cc, "0000", 4)) { /* has 4 leading zeroes */
	    ss = ii;
	    ii = hh = mm = 0;
	}
	else if(!strncmp(cc, "00", 2)) { /* has 2 leading zeroes */
	    hh = 0;
	    mm = ii > 100 ? ii/100 : ii;
	    ii /= 100;
	    ss = ii % 100;
	}
	else {
	    hh = ii % 100;
	    ii /= 100;

	    if(ii) {
		mm = hh;
		hh = ii % 100;
		ii /= 100;
	    }
	    if(ii) {
		ss = mm;
		mm = hh;
		hh = ii % 100;
		ii /= 100;
	    }
	}
    }
    if(yy > 0)
	ddt->set_year( yy );
    if(mon > 0)
	ddt->set_month( mon );
    if(dd > 0)
	ddt->set_day( dd );

    ddt->set_additional_seconds( (DTime) D_SECS(hh,mm,ss,ms) );

    return(YES);
}

// c---------------------------------------------------------------------------

void centeredAvrg(Xfloat * in, Xfloat * out, int count, AvrgList * avli)
{
    int size = avli->returnSize();
    Xfloat * inOffs = in;
    Xfloat * inEnd = in + count;
    Xfloat * outEnd = out + count;
    int ctr = size & 1 ? size/2 + 1 : size/2; // odd or even?
    int limit = ( count > size ) ? size : count;

    avli->reset();		// zeroes out the running average and sum

    // seed the list with the first N/2 values if possible

    for(; limit-- ; avli->replaceNext(*inOffs++));

    Xfloat avrg = avli->returnAvrg();

    // fill up to center point with this average

    limit = ( count > ctr ) ? ctr : count;

    for(; limit-- ; *out++ = avrg);

    // average the rest

    for(; inOffs < inEnd; *out++ = avli->replaceNext(*inOffs++));

    // finish up

    avrg = avli->returnAvrg();

    for(; out < outEnd; *out++ = avrg);
}

// c---------------------------------------------------------------------------

void taperUpAvrg(Xfloat * in, Xfloat * out, int count, AvrgList * avli)
{
    Xfloat * inEnd = in + count;
    avli->reset();		// zeroes out the running average

				// leave the first value alone
				// the second is the average of the first two
				// the third is the average of the first three
				// etc. up to "sizeofList"

    for(; in < inEnd; *out++ = avli->replaceNext(*in++) );
}

// c---------------------------------------------------------------------------

void dd_StdDev( Xfloat * in, Xfloat * avg, Xfloat * out, int count
    , AvrgList * avliDf, AvrgList * avliDfSq)
{
    // Standard deviation of "in" put in "out" using AvrgList(s).

    int size = avliDf->returnSize();
    if( size < 2 )
	{ return; }
    Xfloat rcp_size = 1./size;
    Xfloat * avgOffs = avg;
    Xfloat * inOffs = in;
    Xfloat * inEnd = in + count;
    Xfloat * outEnd = out + count;
    Xfloat d = 0, dSq = 0, diff, val;
    int ctr = size & 1 ? size/2 + 1 : size/2; // odd or even?
    int limit = ( count > size ) ? size : count;

    avliDf->reset();		// for the running sum of the differences
    avliDfSq->reset();		// for the square of the differences

    // seed the running sums with size points

    for(; limit-- ; ) {
	diff = *(inOffs++) - (*avgOffs++);
	d = avliDf->updateSum(diff);
	dSq = avliDfSq->updateSum(diff * diff);
    }
    Xfloat nw = size;
    d *= rcp_size;
    dSq *= rcp_size;
    val = sqrt( (double)( (nw/(nw - 1)) * (dSq - d * d) ));

    // fill up to the center point with this std. dev.

    limit = ( count > ctr ) ? ctr : count;

    for(; limit-- ; *out++ = val);

    // now finish the stdDev array almost to the end

    for(; inOffs < inEnd ;) {
	diff = *(inOffs++) - (*avgOffs++);
	d = avliDf->updateSum(diff);
	dSq = avliDfSq->updateSum(diff * diff);

	d *= rcp_size;
	dSq *= rcp_size;
	*out++ = sqrt( (double)( (nw/(nw - 1)) * (dSq - d * d) ));
    }

    // really finish it

    for(; out < outEnd; *out = *(out -1), out++);
}

// c---------------------------------------------------------------------------

void dd_Variance( Xfloat * in, Xfloat * avg, Xfloat * out, int ngates
    , AvrgList * avliDf, AvrgList * avliDfSq)
{
    dd_StdDev( in, avg, out, ngates, avliDf, avliDfSq);

    Xfloat * outEnd = out + ngates;

    for(; out < outEnd; out++ ) {
	*out = (*out) * (*out);
    }
}

// c---------------------------------------------------------------------------

int
extra_parameters::there_is_a(char *id) {
    XTRA_PARAM_PAIR *pair = first_pair;
    for(; pair; pair = pair->next) {
	if(!strcmp(pair->id, id)) {
	    return 1;
	}
    }
    return(0);
}

// c---------------------------------------------------------------------------

Xfloat
extra_parameters::return_val_for(char *id) {
    XTRA_PARAM_PAIR *pair = first_pair;
    for(; pair; pair = pair->next) {
	if(!strcmp(pair->id, id)) {
	    return pair->val;
	}
    }
    return(0);
}

/* c------------------------------------------------------------------------ */

void
special_sectors::reset_hits()
{
  if( !az_limits->return_num_links() )
    { return; }
	
  limits_node * azn = az_limits->loop_reset();

  for(; azn; ) {
    azn->reset_hit_count();
    azn = az_limits->next_node();
  }
}

/* c------------------------------------------------------------------------ */

int
special_sectors::return_hits( char * sector_id )
{
  if( !az_limits->return_num_links() )
    { return 1; }
	
  limits_node * azn = az_limits->loop_reset();
  const char * id;

  for(; azn; ) {
    id = azn->return_id();
    if( !strcmp( id, sector_id )) {
      return azn->return_hit_count();
    }
    azn = az_limits->next_node();
  }
  return 0;
}

/* c------------------------------------------------------------------------ */

int
special_sectors::return_hits( int sector_num )
{
  if( !az_limits->return_num_links() )
    { return 1; }
	
  limits_node * azn = az_limits->loop_reset();
  int ii;

  for(ii = 0; azn; ii++ ) {
    if( ii == sector_num ) {
      return azn->return_hit_count();
    }
    azn = az_limits->next_node();
  }
  return 0;
}

/* c------------------------------------------------------------------------ */

const char *
special_sectors::max_hits_id()
  {
    if( !az_limits->return_num_links() )
      { return NULL; }
    limits_node * azn = az_limits->loop_reset();
    const char * id = NULL;
    int ii, max = 0, kk;

    for(ii = 0; azn; ii++ ) {
      if( (kk = azn->return_hit_count()) > max ) {
	max = kk;
	id = azn->return_id();
      }
      azn = az_limits->next_node();
    }
    return id;
  }

/* c------------------------------------------------------------------------ */

const char *
special_sectors::return_sector_id( int sector_num )
  {
    if( !az_limits->return_num_links() )
      { return NULL; }
    limits_node * azn = az_limits->loop_reset();
    const char * id;
    int ii;

    for(ii = 0; azn; ii++ ) {
      if( ii == sector_num ) {
	id = azn->return_id();
	return id;
      }
      azn = az_limits->next_node();
    }
    return 0;
  }

/* c------------------------------------------------------------------------ */

int
special_sectors::
inOneSector( float az )
{
  if( !az_limits->return_num_links() )
    { return 1; }
	
  limits_node * azn = az_limits->loop_reset();
	
  for(; azn; ) {
    if( azn->inThisSector( (double)az ) )
      { return 1; }
    azn = az_limits->next_node();
  }
  return 0;
}

/* c------------------------------------------------------------------------ */

int
special_sectors::
inOneSector( float fxd, float az )
{
  if( !az_limits->return_num_links() )
    { return 1; }
	
  limits_node * azn = az_limits->loop_reset();
	
  limits_node * fxn = fxd_limits->loop_reset();
	
  for(; azn; ) {
    if( azn->inThisSector( (double)az ) &&
	fxn->inThisSector( (double)fxd ) )
      { return 1; }
    azn = az_limits->next_node();
    fxn = fxd_limits->next_node();
  }
  return 0;
}

/* c------------------------------------------------------------------------ */

void
special_sectors::add_a_sector( float az1, float az2, char * id )
{
  limits_node * node;
  node = az_limits->add_node();
  node->set_limit1( (double)az1 );
  node->set_limit2( (double)az2 );
  node->set_id( id );
}

/* c------------------------------------------------------------------------ */

void
special_sectors::add_a_sector( float fx1, float fx2
			       , float az1, float az2 )
{
  limits_node * node;
  node = az_limits->add_node();
  node->set_limit1( (double)az1 );
  node->set_limit2( (double)az2 );
  node = fxd_limits->add_node();
  node->set_limit1( (double)fx1 );
  node->set_limit2( (double)fx2 );
}

/* c------------------------------------------------------------------------ */

int dd_mkdir( char * dir, int mode )
{
    DIR * dirp;
    
    if(( dirp = opendir( dir ))) {
	closedir( dirp );	// directory already exists
	return YES;
    }
    return NO;
}

// c---------------------------------------------------------------------------

double time_from_file_name( char * fname, char * prefix )
{
  int ii;
  const char * dot = ".";
  const char * under_score = "_";
  char str[256], *sptrs[32];
  struct tm tm;
  char *aa = strrchr( fname, '/' ), *bb;

  aa = aa ? aa+1 : fname;

  strcpy( str, aa );

  if( !strcmp( "ddop", prefix )) {
    // "ddop.19990919.192953.cdf"

    dd_tokenz( str, sptrs, dot );
    sscanf( sptrs[1], "%4d%2d%2d", &tm.tm_year, &tm.tm_mon, &tm.tm_mday );
    tm.tm_year -= 1900;
    tm.tm_mon -= 1;
    sscanf( sptrs[2], "%2d%2d%2d", &tm.tm_hour, &tm.tm_min, &tm.tm_sec );
  }
  else if( !strcmp( "20", prefix )) { // no mnemonic
    // 20000815.150000.nc

    dd_tokenz( str, sptrs, dot );
    sscanf( sptrs[0], "%4d%2d%2d", &tm.tm_year, &tm.tm_mon, &tm.tm_mday );
    tm.tm_year -= 1900;
    tm.tm_mon -= 1;
    sscanf( sptrs[1], "%2d%2d%2d", &tm.tm_hour, &tm.tm_min, &tm.tm_sec );
  }
  else if( !strcmp( "swp", prefix )) {

    dd_tokenz( str, sptrs, dot );
    bb = sptrs[1] + strlen( sptrs[1] ) -10; // point to month
    sscanf( bb, "%2d%2d%2d%2d%2d", &tm.tm_mon, &tm.tm_mday
	    , &tm.tm_hour, &tm.tm_min, &tm.tm_sec );
    tm.tm_mon -= 1;
    *bb = '\0';
    tm.tm_year = atoi( sptrs[1] );
    if( tm.tm_year > 1900 ) { tm.tm_year -= 1900; }
  }
  else if( strcmp( "pqswp", prefix ) == 0 ||
	   strcmp( "ncswp", prefix ) == 0  ||
	   strcmp( "ced", prefix ) == 0  ||
	   strcmp( "mdv", prefix ) == 0 ) {

    // "pqswp_SPOL_20000711_232252.141_v149_s1053_0.5_PPI"
    // "ncswp_SPOL_20000711_232252.141_v149_s1053_0.5_PPI"
    // "ncswp__20000711_232252.141_v149_s1053_0.5_PPI"  // bistatic

    ii = strstr( str, "ncswp__" ) ? 1 : 2; // bistatic
    dd_tokenz( str, sptrs, under_score );
    sscanf( sptrs[ii], "%4d%2d%2d", &tm.tm_year, &tm.tm_mon, &tm.tm_mday );
    tm.tm_year -= 1900;
    tm.tm_mon -= 1;
    sscanf( sptrs[ii+1], "%2d%2d%2d", &tm.tm_hour, &tm.tm_min, &tm.tm_sec );
  }

  char tz[16];
  strcpy (tz, "TZ=GMT");
  putenv (tz);
  timezone = 0;
  daylight = 0;
  tm.tm_wday = tm.tm_yday = 0;
  tm.tm_isdst = -1;
  time_t tt = mktime( &tm );
  return (double)tt;
}

// c---------------------------------------------------------------------------

int CleanDir2( char * ref_dir, struct solo_list_mgmt * rlist, int rcount
	, int time_span, char * prefix )
{
    char x_link[ 256 ];
    char *bb, *fname;
    int jj, zapped = 0;

    if( rcount < 1 || time_span < 0 )
	{ return zapped; }

    strcpy( x_link, ref_dir );
    strcat( x_link, "/" );
    bb = x_link + strlen( x_link );

    int nr = generic_sweepfiles( ref_dir, rlist, prefix, "", NO );
    int keep = nr;
    if( nr < 1 )
      { return zapped; }

    // get the time for the latest file
    double t0, t1, t2;

    if( time_span > 0 ) {
	fname = solo_list_entry( rlist, nr-1 );
	t2 = time_from_file_name( fname, prefix );
	t1 = t2 - time_span;

	for( jj=0; keep > rcount && jj < nr; jj++ ) {
	    fname = solo_list_entry( rlist, jj );
	    t0 = time_from_file_name( fname, prefix );
            if( t0 < t1 ) {
                strcpy( bb, fname );
                unlink( x_link );
                keep--;
                zapped++;
            }
	}
	return zapped;
    }
    int nz = nr - rcount;

    for( jj=0; jj < nz; jj++ ) {
	fname = solo_list_entry( rlist, jj );
	strcpy( bb, fname );
	unlink( x_link );
	zapped++;
    }
    return zapped;
}

// c---------------------------------------------------------------------------

int CleanDir3( char * ref_dir, struct solo_list_mgmt * rlist, int rcount
	, int time_span, char * prefix, char * suffix
	      , int not_this_suffix )
{
    char x_link[ 256 ];
    char *bb, *fname;
    int jj, zapped = 0;

    if( rcount < 1 || time_span < 0 )
	{ return zapped; }

    strcpy( x_link, ref_dir );
    strcat( x_link, "/" );
    bb = x_link + strlen( x_link );

    int nr = generic_sweepfiles( ref_dir, rlist, prefix, suffix
				 , not_this_suffix );
    if (nr < 1)
      { return zapped; }

    int keep = nr;

    // get the time for the latest file
    double t0, t1, t2;

    if( time_span > 0 ) {
	fname = solo_list_entry( rlist, nr-1 );
	t2 = time_from_file_name( fname, prefix );
	t1 = t2 - time_span;

	for( jj=0; keep > rcount && jj < nr; jj++ ) {
	    fname = solo_list_entry( rlist, jj );
	    t0 = time_from_file_name( fname, prefix );
            if( t0 < t1 ) {
                strcpy( bb, fname );
                unlink( x_link );
                keep--;
                zapped++;
            }
	}
	return zapped;
    }
    int nz = nr - rcount;

    for( jj=0; jj < nz; jj++ ) {
	fname = solo_list_entry( rlist, jj );
	strcpy( bb, fname );
	unlink( x_link );
	zapped++;
    }
    return zapped;
}

// c---------------------------------------------------------------------------


// c---------------------------------------------------------------------------







