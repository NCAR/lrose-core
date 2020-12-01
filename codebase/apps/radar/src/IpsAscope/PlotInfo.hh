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
#ifndef PLOTINFOINC_
#define PLOTINFOINC_

#include <string>

class PlotInfo {
public:
	PlotInfo();
	PlotInfo(int id, int displayType, std::string shortName, std::string longName,
		double gainMin, double gainMax, double gainCurrent, 
		double offsetMin, double offsetMax, double offsetCurrent);
	virtual ~PlotInfo();

	int getId();
	int getDisplayType();

	void setGain(double min, double max, double current);
	void setOffset(double min, double Max, double current);

	double getGainMin();
	double getGainMax();
	double getGainCurrent();

	double getOffsetMin();
	double getOffsetMax();
	double getOffsetCurrent();

	std::string getShortName();
	std::string getLongName();
	
	/// Set the autoscale flag
	/// @param flag True if autscale requested.
	void autoscale(bool flag);
	
	/// @return True if an autoscale is needed.
	bool autoscale();

protected:
	int _id;
	int _displayType;
	std::string _shortName;
	std::string _longName;
	double _gainMin;
	double _gainMax;
	double _gainCurrent;
	double _offsetMin;
	double _offsetMax;
	double _offsetCurrent;
    bool _autoscale;
};
#endif
