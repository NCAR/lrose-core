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

#ifndef _STATISTICIAN_INC_
#define _STATISTICIAN_INC_

#include <cstdio>
#include <string>


/**
 *
 * @file statistician.hh
 *
 * @class statistician
 *
 * Class to handle the accumulated statistics output. Taken from
 * app/didss/src/Validator and reformatted a little - the idea
 * being that we want to keep the output format the same as
 * it was for Validator.
 *
 * @author Niles Oien
 *
 */


using namespace std;


class statistician
{

public:

/**
 * The constructor. Sets up for the long road ahead.
 *
 * @param name The name of this statistician. Used in output file
 *             names and directory names.
 * @param startTime Time to start output data interval bins.
 * @param outDir Output directory - we append the name to this as a subdir.
 * @param outputInterval The output interval, seconds.
 *
 * @return None.
 *
 * @author Niles Oien oien@ucar.edu
 *
 */ 
  statistician(string name,
	       time_t startTime,
	       string outDir,
	       int outputInterval);


/**
 * Initialization routine. Opens output files.
 * 
 * @return None.
 *
 * @author Niles Oien oien@ucar.edu
 *
 */ 
  void init();

/**
 * Accumulate the results of another truth/forecast pair.
 *
 * @param numNon The number of non events.
 * @param numFail The number of failures (or misses).
 * @param numFalse The number of false alarms.
 * @param numHit The number of hits.
 * @param truthTime The truth time for this data pair.
 * @param leadTime The lead time, secs, for this forecast.
 *
 * @return None.
 *
 * @author Niles Oien oien@ucar.edu
 *
 */ 
  void accumulate(unsigned long numNon, 
		  unsigned long numFail, 
		  unsigned long numFalse, 
		  unsigned long numHit,
		  time_t truthTime,
		  int leadTime,
		  char *forecastDataSetSource);


/**
 * Reset the object to a certain time and set counts to 0.
 *
 * @param newTime The time to reset to.
 *
 * @return None.
 *
 * @author Niles Oien oien@ucar.edu
 *
 */ 
  void reset(time_t newTime);




/**
 * Destructor. Closes output files.
 *
 * @return None.
 *
 * @author Niles Oien oien@ucar.edu
 *
 */ 
  ~statistician();

private:

protected:

  double _totalNumNon, _totalNumFail, 
    _totalNumFalse, _totalNumHit;

  double _subtotalNumNon, _subtotalNumFail, 
    _subtotalNumFalse, _subtotalNumHit;

  unsigned long _num, _subnum;

  FILE *_sfp, *_fsfp;

  time_t _lastOutputTime;
  time_t _lastTruthTime;
  time_t _outputInterval;
  string _name;
  string _outDir;
  time_t _startTime;
  int _sourceIndicator;


  double _noNan(double val);


};

#endif































