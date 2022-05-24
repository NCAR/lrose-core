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
/////////////////////////////////////////////////////////////
// Filter.hh
//
// Mike Dixon, RAP, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// March 2004
//
///////////////////////////////////////////////////////////////

#ifndef Filter_H
#define Filter_H

#include <string>
using namespace std;

class ClutFilter;

////////////////////////
// This class

class Filter {
  
public:

  // constructor

  Filter(ClutFilter &parent, bool debug);

  // destructor
  
  ~Filter();

  // perform filtering
  
  void run(const double *rawMag, 
	   int nSamples,
	   double max_clutter_vel,
	   double init_notch_width,
	   double nyquist,
	   double *filteredMag,
	   int &clutterFound,
	   int &notchStart,
	   int &notchEnd,
	   double &powerRemoved,
	   double &vel,
	   double &width);

protected:
  
private:
  
  ClutFilter &_parent;
  bool _debug;

  void _locateWxAndClutter(const double *power,
			   int nSamples,
			   double max_clutter_vel,
			   double init_notch_width,
			   double nyquist,
			   int &notchWidth,
			   int &clutterFound,
			   int &weatherPos,
			   int &clutterPos);

  void _fitGaussian(const double *magnitude,
		    int nSamples, 
		    int weatherPos,
		    double minMagnitude,
		    double nyquist,
		    double &vel,
		    double &width,
		    double *gaussian);
  
};

#endif

