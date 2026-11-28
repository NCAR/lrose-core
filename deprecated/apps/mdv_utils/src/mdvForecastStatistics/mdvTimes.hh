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
//
// Niles Oien, RAP, NCAR,
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// October 2006
//
/////////////////////////////////////////////////////////////

#ifndef MDV_TIMES_HH
#define MDV_TIMES_HH

#include <string>
#include <vector>
#include <ctime>


/**
 *
 * @file mdvTimes.hh
 *
 * @class mdvTimes
 *
 * Class to get input data times given an MDV URL. Operates in two modes,
 * forecast and non-forecast, depending on the nature of the MDV data
 * (forecast data being of the type that has the generate and lead
 * time in the MDV filename).
 * 
 * The general idea here is that you pass in a URL, a start and end time,
 * and indicate if the URL you are looking at has forecast data or not.
 * The class will then compile an internal vector of times, which are
 * stored in the structure mdvTimes::mdvTime_t.
 *
 * In forecast mode, the start and end times are taken as being
 * limits on the generation time. In this case the genTime and
 * leadTime fields in the mdvTime_t structure are filled out.
 *
 * In non-forecast mode, the start and end times are taken as
 * being limits on the data times. In this case the genTime
 * field in the mdvTime_t structure is filled out, and the
 * leadTime is set to mdvTimes::undefinedLeadTime.
 *
 * @author Niles Oien
 *
 */
using namespace std;

class mdvTimes {
  
public:
  
  typedef struct {
    time_t genTime;
    int leadTime;
  } mdvTime_t;

  const static int undefinedLeadTime = -1; // Indicates non-forecast data.
  

/**
 * The constructor. Generates the internal vector of data times.
 *
 * @param url The MDV URL to parse for data times.
 * @param isForecast A boolean indicating if the data are in forecast mode.
 * @param startTime The lower limit on the times, inclusive.
 * @param endTime The upper limit on the time, inclusive.
 *
 * @return  No return value.
 *
 *
 * @author Niles Oien oien@ucar.edu
 *
 */ 
  mdvTimes(string url,
	   bool isForecast,
	   time_t startTime,
	   time_t endTime);


/**
 * Get the number of data times. No input parameters.
 *
 * @return  The number of data times, an integer.
 *
 * @author Niles Oien oien@ucar.edu
 *
 */ 
  int getNumTimes();

/**
 * Get the next data time in an mdvTimes::mdvTime_t struct.
 *
 * @return  An mdvTimes::mdvTime_t struct. In forecast mode all
 * fields in the stuct are filled out, in non-forecast mode the
 * leadTime field is set to  mdvTimes::undefinedLeadTime.
 *
 * @author Niles Oien oien@ucar.edu
 *
 */ 
  mdvTime_t getNextTime();

/**
 * Destructor. Frees up memory as needed.
 *
 * @return  None.
 *
 * @author Niles Oien oien@ucar.edu
 *
 */ 
  ~mdvTimes();
  
protected:
  
private:

  vector< mdvTime_t > _dataTimes;
  vector< mdvTime_t >::iterator _currentPosition;
  int _numTimes;

};

#endif
