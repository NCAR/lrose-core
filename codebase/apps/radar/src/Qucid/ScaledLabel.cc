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
// ScaledLabel.cpp: implementation of the ScaledLabel class.
//
//////////////////////////////////////////////////////////////////////

#include "ScaledLabel.hh"
#include <iomanip>
#include <math.h>

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

ScaledLabel::ScaledLabel(ScalingType t)
{
    m_scalingType = t;
    m_stringStr << std::setiosflags(std::ios_base::fixed);
    
}

ScaledLabel::~ScaledLabel()
{
    
}

//////////////////////////////////////////////////////////////////////
std::string
ScaledLabel::scale(double value) {
    
    // value is in km. 
    
    double exp = floor(log10(value));
    std::string units;
    
    double scaleFactor;
    if (exp < -3) {
        units = "mm";
        scaleFactor = 0.000001;
    } else {
        if (exp < 0) {
            units = "m";
            scaleFactor = 0.001;
        } else {
            units = "km";
            scaleFactor = 1;
        }
    }
    
    double m = value/scaleFactor;
    if (m >=100) {
        m_stringStr << std::setprecision(0);
    } else { 
        if (m >= 10.0) {
            m_stringStr << std::setprecision(1);
        } else {
            m_stringStr << std::setprecision(2);
        }
    }

    m_stringStr.str("");
    m_stringStr << m << units;
    return m_stringStr.str();
    
}

