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

#include "PlotInfo.hh"

////////////////////////////////////////////////////////
PlotInfo::PlotInfo():
_id(0),
_displayType(0),
_shortName(""),
_longName(""),
_gainMin(-1),
_gainMax(1),
_gainCurrent(0),
_offsetMin(-1),
_offsetMax(1),
_offsetCurrent(0),
_autoscale(true)
{
}
////////////////////////////////////////////////////////
PlotInfo::PlotInfo(int id, int displayType, std::string shortName, std::string longName,
		double gainMin, double gainMax, double gainCurrent, 
		double offsetMin, double offsetMax, double offsetCurrent):
_id(id),
_displayType(displayType),
_shortName(shortName),
_longName(longName),
_gainMin(gainMin),
_gainMax(gainMax),
_gainCurrent(gainCurrent),
_offsetMin(offsetMin),
_offsetMax(offsetMax),
_offsetCurrent(offsetCurrent),
_autoscale(true)
{
}

////////////////////////////////////////////////////////

PlotInfo::~PlotInfo()
{
}

////////////////////////////////////////////////////////
void
PlotInfo::setGain(double min, double max, double current)
{
	_gainMin = min;
	_gainMax = max;
	_gainCurrent = current;
}

////////////////////////////////////////////////////////
void
PlotInfo::setOffset(double min, double max, double current)
{
	_offsetMin = min;
	_offsetMax = max;
	_offsetCurrent = current;
}

////////////////////////////////////////////////////////
double 
PlotInfo::getGainMin()
{
	return _gainMin;
}

////////////////////////////////////////////////////////
double 
PlotInfo::getGainMax()
{
	return _gainMax;
}

////////////////////////////////////////////////////////
double 
PlotInfo::getGainCurrent()
{
	return _gainCurrent;
}

////////////////////////////////////////////////////////
double 
PlotInfo::getOffsetMin()
{
	return _offsetMin;
}

////////////////////////////////////////////////////////
double 
PlotInfo::getOffsetMax()
{
	return _offsetMax;
}

////////////////////////////////////////////////////////
double 
PlotInfo::getOffsetCurrent()
{
	return _offsetCurrent;
}

////////////////////////////////////////////////////////
std::string 
PlotInfo::getShortName()
{
	return _shortName;
}

////////////////////////////////////////////////////////
std::string 
PlotInfo::getLongName()
{
	return _longName;
}
////////////////////////////////////////////////////////
int 
PlotInfo::getId()
{
	return _id;
}
////////////////////////////////////////////////////////
int 
PlotInfo::getDisplayType()
{
	return _displayType;
}
////////////////////////////////////////////////////////
void
PlotInfo::autoscale(bool b) {
    _autoscale = b;
}
////////////////////////////////////////////////////////
bool
PlotInfo::autoscale() {
    return _autoscale;
}



