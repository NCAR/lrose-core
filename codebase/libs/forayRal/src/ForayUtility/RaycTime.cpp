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
//
//
//
//
//

#include <sys/time.h>
#include <time.h>
#include <stdio.h>

#include <cmath>

#include "RaycTime.h"
using namespace std;
using namespace ForayUtility;

//////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////
RaycTime::RaycTime(){

    seconds_     = 0;
    nanoSeconds_ = 0;
}

//////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////
RaycTime::RaycTime(int seconds) throw (Fault){

    try{
	set(seconds);
    }catch(Fault re){
	re.add_msg("RaycTime::RaycTime(int): Caught Fault \n");
	throw re;
    }
}

//////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////
RaycTime::RaycTime(double seconds) throw (Fault){

    try{
	set(seconds);
    }catch(Fault re){
	re.add_msg("RaycTime::RaycTime(double): Caught Fault \n");
	throw re;
    }

}

//////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////
RaycTime::~RaycTime(){


}

//////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////
RaycTime::RaycTime(const RaycTime &src){

    seconds_     = src.seconds_;
    nanoSeconds_ = src.nanoSeconds_;
}

//////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////
RaycTime &RaycTime::operator=(const RaycTime &src){

    if(this == &src){
	return *this;
    }

    seconds_     = src.seconds_;
    nanoSeconds_ = src.nanoSeconds_;

    return *this;
}

//////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////
int RaycTime::seconds() const {
    return seconds_;
}

//////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////
int RaycTime::nanoSeconds() const{
    return nanoSeconds_;
}

//////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////
double RaycTime::value() const{

    return (double)seconds_ + ((double)nanoSeconds_/1000000000.0);
}


//////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////
void RaycTime::set_current_time() throw (Fault){

    struct timeval currentTimeval;

    if(gettimeofday(&currentTimeval,NULL) != 0) {
    	seconds_ = 0;
	nanoSeconds_ = 0;
	throw Fault("RaycTime::set_current_time: gettimeofday failed.\n");
	
    } else {

    	seconds_     = currentTimeval.tv_sec;
    	nanoSeconds_ = currentTimeval.tv_usec * 1000;
    }
}

//////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////
void RaycTime::set(int seconds, int nanoSeconds) throw (Fault){

    if((seconds < 0) || (nanoSeconds < 0)){
	seconds_ = 0;
	nanoSeconds_ = 0;
	char msg[2048];
	sprintf(msg,
		"RaycTime::set: Invalid parameters: secs: %d, nanoSecs: %d .\n",
		seconds,nanoSeconds);
	throw Fault(msg);
	
    } else {

    	int tmp = nanoSeconds / 1000000000;
	
    	seconds_     = seconds + tmp;
	
    	nanoSeconds_ = nanoSeconds - (tmp * 1000000000);
    
    }
}

//////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////
void RaycTime::set(int seconds) throw (Fault){

    if(seconds < 0){
    	seconds_ = 0;
	nanoSeconds_ = 0;
	char msg[2048];
	sprintf(msg,
		"RaycTime::set(int): Invalid parameters: secs: %d .\n",
		seconds);
	throw Fault(msg);
	
    } else {

    	seconds_     = seconds;
    	nanoSeconds_ = 0;
    }
}

//////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////
void RaycTime::set(double seconds) throw (Fault){

  if(std::isnan(seconds) || std::isinf(seconds) || seconds < 0.0){
    	seconds_ = 0;
	nanoSeconds_ = 0;
	char msg[1024];
	sprintf(msg,
		"RaycTime::set(double): parameter not valid: %f .\n",
		seconds);
	throw Fault(msg);
	
    } else {

    	makeInt(seconds);
    
    }
}

//////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////
void RaycTime::set(int year, int month, int day, 
		   int hour, int minute, int second,
		   int nanosecond) throw (Fault){

    struct tm tv;

    tv.tm_year = year -1900;
    tv.tm_mon  = month - 1;
    tv.tm_mday = day;
    tv.tm_hour = hour;
    tv.tm_min  = minute;
    tv.tm_sec  = second;

    time_t seconds = timegm(&tv);

    seconds_     = (int)seconds;
    nanoSeconds_ = nanosecond;

}




//////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////
string RaycTime::list_entry(){

    struct tm tv;
    
    const time_t ttSeconds = (time_t)seconds_;

    gmtime_r(&ttSeconds,&tv);

    char entry[1024];

    sprintf(entry,"%4d%02d%02d %02d%02d%02d %09d",
	    tv.tm_year + 1900,
	    tv.tm_mon + 1,
	    tv.tm_mday,
	    tv.tm_hour,
	    tv.tm_min,
	    tv.tm_sec,
	    nanoSeconds_);

    return string(entry);
}

//////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////
string RaycTime::csv_full_head(){

    char msg[2048];

    sprintf(msg,
	    "%8s,%6s,%9s",
	    "Date",
	    "Time",
	    "NanoSecs");

    return string(msg);
}

//////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////
string RaycTime::csv_full_line(){

    struct tm tv;
    const time_t ttSeconds = (time_t)seconds_;
    gmtime_r(&ttSeconds,&tv);

    char msg[2048];
    sprintf(msg,
	    "%04d%02d%02d,%02d%02d%02d,%09d",
	    tv.tm_year + 1900,
	    tv.tm_mon + 1,
	    tv.tm_mday,
	    tv.tm_hour,
	    tv.tm_min,
	    tv.tm_sec,
	    nanoSeconds_);

    return string(msg);
}

//////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////
int RaycTime::get_year() const{

    struct tm tv;
    const time_t ttSeconds = (time_t)seconds_;
    gmtime_r(&ttSeconds,&tv);

    return tv.tm_year + 1900;
}

//////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////
int RaycTime::get_month() const{

    struct tm tv;
    const time_t ttSeconds = (time_t)seconds_;
    gmtime_r(&ttSeconds,&tv);

    return tv.tm_mon + 1;
}

//////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////
int RaycTime::get_day() const{

    struct tm tv;
    const time_t ttSeconds = (time_t)seconds_;
    gmtime_r(&ttSeconds,&tv);

    return tv.tm_mday;
}

//////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////
int RaycTime::get_hour() const{

    struct tm tv;
    const time_t ttSeconds = (time_t)seconds_;
    gmtime_r(&ttSeconds,&tv);

    return tv.tm_hour;
}

//////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////
int RaycTime::get_minute() const{

    struct tm tv;
    const time_t ttSeconds = (time_t)seconds_;
    gmtime_r(&ttSeconds,&tv);

    return tv.tm_min;
}

//////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////
int RaycTime::get_second() const{

    struct tm tv;
    const time_t ttSeconds = (time_t)seconds_;
    gmtime_r(&ttSeconds,&tv);

    return tv.tm_sec;
}

//////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////
int RaycTime::get_nanosecond() const{

    return nanoSeconds_;
}

//////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////
int RaycTime::get_julian_day() const{

    struct tm tv;
    const time_t ttSeconds = (time_t)seconds_;
    gmtime_r(&ttSeconds,&tv);

    return tv.tm_yday + 1;
}


//////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////
RaycTime &RaycTime::operator+=(const int &seconds) throw (Fault){

    seconds_ += seconds;

    if(seconds_ < 0){
    	
	seconds_ = 0;
	nanoSeconds_ = 0;
	
	char msg[1024];
	sprintf(msg,
		"RaycTime::operator+=(int): adding %d created negative time.\n",
		seconds);
	throw Fault(msg);
    }

    return *this;
}

//////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////
RaycTime &RaycTime::operator+=(const double &seconds) throw (Fault){

    double addSec, addNanoSec;

    addNanoSec = modf(seconds,&addSec);

    int intAddSec     = (int)addSec;
    
    seconds_     += intAddSec;
    
    if(seconds >= 0)
    {
    
    	int intAddNanoSec = (int)((addNanoSec * 1000000000.0) + 0.5); // Add 0.5 to prevent rounding errors
    
	nanoSeconds_ += intAddNanoSec;
	
	if(nanoSeconds_ >= 1000000000) {
		nanoSeconds_ -= 1000000000;
		++seconds_;
	} 
	
    } else {
    
    	int intAddNanoSec = (int)((addNanoSec * 1000000000.0) - 0.5); // Add 0.5 to prevent rounding errors
    
	nanoSeconds_ += intAddNanoSec;
	
	if(nanoSeconds_ < 0) {
		nanoSeconds_ = 1000000000 + nanoSeconds_;
		--seconds_;
	}
    
    }
    
    if(seconds_ < 0){
	seconds_ = 0;
	nanoSeconds_ = 0;
	char msg[1024];
	sprintf(msg,
		"RaycTime::operator+=(double): adding %g created negative time.\n",
		seconds);
	throw Fault(msg);
    }
    
    //double thisTime = makeDouble();

    //    thisTime += seconds;

    //    if(thisTime < 0.0){
    //	char msg[1024];
    //	sprintf(msg,
    //		"RaycTime::operator+=(double): adding %f created negative time.\n",
    //	seconds);
    //	throw Fault(msg);
    //}

    //makeInt(thisTime);

    return *this;
}

//////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////
RaycTime &RaycTime::operator-=(const int &seconds) throw (Fault){

    seconds_ -= seconds;

    if(seconds_ < 0){
    	seconds_ = 0;
	nanoSeconds_ = 0;
	char msg[1024];
	sprintf(msg,
		"RaycTime::operator-=(int): adding %d created negative time.\n",
		seconds);
	throw Fault(msg);
    }

    return *this;
}

//////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////
RaycTime &RaycTime::operator-=(const double &seconds) throw (Fault){

    double subSec, subNanoSec;

    subNanoSec = modf(seconds,&subSec);

    int intSubSec     = (int)subSec;
    
    seconds_     -= intSubSec;
    
    if(seconds >= 0)
    {
    
    	int intSubNanoSec = (int)((subNanoSec * 1000000000.0) + 0.5); // Add 0.5 to prevent rounding errors
    
	nanoSeconds_ -= intSubNanoSec;
	
	if(nanoSeconds_ < 0) {
		nanoSeconds_ = 1000000000 + nanoSeconds_;
		--seconds_;
	}
	
    } else {
    
    	int intSubNanoSec = (int)((subNanoSec * 1000000000.0) - 0.5); // Add 0.5 to prevent rounding errors
    
	nanoSeconds_ -= intSubNanoSec;
	
	if(nanoSeconds_ >= 1000000000) {
		nanoSeconds_ -= 1000000000;
		++seconds_;
	}
    
    }
    
    if(seconds_ < 0){
	seconds_ = 0;
	nanoSeconds_ = 0;
	char msg[1024];
	sprintf(msg,
		"RaycTime::operator-=(double): subtracting %g created negative time.\n",
		seconds);
	throw Fault(msg);
    }
    
    return *this;
}

//////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////
bool RaycTime::operator<(const RaycTime &rhs) const{

    const double leftTime = makeDouble();
    const double rightTime = rhs.makeDouble();

    return (leftTime < rightTime);

}

//////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////
bool RaycTime::operator>(const RaycTime &rhs) const{

    double leftTime = makeDouble();
    double rightTime = rhs.makeDouble();

    return (leftTime > rightTime);
}

//////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////
bool RaycTime::operator>=(const RaycTime &rhs) const{

    double leftTime = makeDouble();
    double rightTime = rhs.makeDouble();

    if (leftTime > rightTime) {
	return true;
    }

    if(seconds_ != rhs.seconds_){
	return false;
    }

    return (nanoSeconds_ == rhs.nanoSeconds_);
}


//////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////
bool RaycTime::operator==(const RaycTime &rhs) const{

    if(seconds_ != rhs.seconds_){
	return false;
    }

    return (nanoSeconds_ == rhs.nanoSeconds_);
}


//////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////
RaycTime operator+(const RaycTime &lhs,const int &rhs) throw(Fault){
    
    RaycTime rt(lhs);

    try {
	rt+= rhs;
    }catch(Fault re){
	re.add_msg("operator+(RaycTime,int): caught Fault \n");
	throw re;
    }

    return rt;
}

//////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////
RaycTime operator+(const RaycTime &lhs,const double &rhs) throw(Fault){
    
    RaycTime rt(lhs);

    try {
	rt+= rhs;
    }catch(Fault re){
	re.add_msg("operator+(RaycTime,double): caught Fault \n");
	throw re;
    }

    return rt;
}

//////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////
RaycTime operator-(const RaycTime &lhs,const int &rhs) throw(Fault){
    
    RaycTime rt(lhs);

    try {
	rt-= rhs;
    }catch(Fault re){
	re.add_msg("operator-(RaycTime,int): caught Fault \n");
	throw re;
    }

    return rt;
}

//////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////
RaycTime operator-(const RaycTime &lhs,const double &rhs) throw(Fault){
    
    RaycTime rt(lhs);

    try {
	rt-= rhs;
    }catch(Fault re){
	re.add_msg("operator-(RaycTime,double): caught Fault \n");
	throw re;
    }

    return rt;
}

//////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////
double operator-(const RaycTime &lhs,const RaycTime &rhs){
    

    double seconds = (double)(lhs.seconds() - rhs.seconds());

    double nanoSeconds = (double)(lhs.nanoSeconds() - rhs.nanoSeconds())/1000000000.0;
    return seconds + nanoSeconds;
}



//////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////
double RaycTime::makeDouble() const{

    double ds  = (double)seconds_;
    double dns = (double)nanoSeconds_;

    return (ds + (dns/1000000000.0));
}

//////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////
void RaycTime::makeInt(double seconds){

    double ds,dns;

    dns = modf(seconds,&ds);

    seconds_     = (int)ds;
    nanoSeconds_ = (int)((dns * 1000000000.0) + 0.5); // Add 0.5 to prevent rounding errors
}
