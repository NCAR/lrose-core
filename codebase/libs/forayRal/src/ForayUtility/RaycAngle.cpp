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

#include <stdio.h>

#include <cmath>

#include "RaycAngle.h"
using namespace std;
using namespace ForayUtility;

const int RaycAngle::scale_    = 10000;
const int RaycAngle::scale360_ = 360 * RaycAngle::scale_;
const int RaycAngle::scale180_ = 180 * RaycAngle::scale_;

//////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////
RaycAngle::RaycAngle(){

    scaleAngle_ = -99;

}

//////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////
RaycAngle::RaycAngle(double initAngle){

    setAngle(initAngle);

}

//////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////
RaycAngle::~RaycAngle(){


}

//////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////
RaycAngle::RaycAngle(const RaycAngle &ra){

    scaleAngle_ = ra.scaleAngle_;

}

//////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////
RaycAngle &RaycAngle::operator=(const RaycAngle &ra){

    scaleAngle_ = ra.scaleAngle_;

    return *this;
}

//////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////
RaycAngle &RaycAngle::operator=(const double da){

    setAngle(da);

    return *this;
}


//////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////
RaycAngle &RaycAngle::operator+=(const double da){

    setAngle(value() + da);

    return *this;
}


//////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////
RaycAngle &RaycAngle::operator-=(const double da){

    setAngle(value() - da);

    return *this;
}



//////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////
bool RaycAngle::operator==(const RaycAngle &ra) const{

    return scaleAngle_ == ra.scaleAngle_;
}

//////////////////////////////////////////////////////////////////////
//
//
//
// angle1 < angle2 if angle1 is counter-clockwise to angle2.
// 
// angle1 > angle2 if angle1 is clockwise to angle2
//
//////////////////////////////////////////////////////////////////////
bool RaycAngle::operator<(const RaycAngle &ra) const{

    int ccw_scaleAngle = ra.scaleAngle_ - scaleAngle_;

    if (ccw_scaleAngle < 0){
	ccw_scaleAngle += scale360_;
    }

    if (ccw_scaleAngle == 0) return false;

    return ccw_scaleAngle <= scale180_;
}

bool RaycAngle::operator>(const RaycAngle &ra) const{

    int ccw_scaleAngle = ra.scaleAngle_ - scaleAngle_;

    if (ccw_scaleAngle < 0){
	ccw_scaleAngle += scale360_;
    }

    if (ccw_scaleAngle == 0) return false;

    return ccw_scaleAngle > scale180_;
}

//////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////
double RaycAngle::value() const{

    return (double)scaleAngle_ / (double)scale_;
}

//////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////
double RaycAngle::abs_delta(const RaycAngle &other) const{

    int diff = scaleAngle_ - other.scaleAngle_;

    if(diff < 0){
	diff += scale360_;
    }

    if(diff > scale180_){
	diff = scale360_ - diff;
    }

    return (double)diff / (double)scale_;
}

//////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////
void RaycAngle::setAngle(const double da){

    double ra = fmod(da,360.0);

    if(ra < 0.0){
	ra += 360.0;
    }

    scaleAngle_ = (int)((ra * scale_) + .5);
}

//////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////
string RaycAngle::csv_full_head(string head) {

    char msg[2048];

    sprintf(msg,"%7s",head.c_str());

    return string(msg);
}

//////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////
string RaycAngle::csv_full_line() {

    char msg[2048];
    
    sprintf(msg,"%7.2f",value());

    return string(msg);
}



