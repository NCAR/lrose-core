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
/**
 *
 * @file CalibDayNight.hh
 *
 * @class CalibDayNight
 *
 * CalibDayNight program object.
 *  
 * @date 12/1/2008
 *
 */

#ifndef CalibDayNight_HH
#define CalibDayNight_HH

#include "Calib.hh"
#include <Mdv/DsMdvx.hh>

class FieldDataPair;
class FieldWithData;

/** 
 * @class CalibDayNight
 */

class CalibDayNight
{
 public:

  ////////////////////
  // Public members //
  ////////////////////

  /**
   * @brief Constructor
   *
   * @param[in] argc Number of command line arguments.
   * @param[in] argv List of command line arguments.
   *
   * @note Private because this is a singleton object.
   */

  CalibDayNight(void);
  

  /**
   * @brief Destructor
   */

  virtual ~CalibDayNight(void);
  
  bool initialize(const std::string &ref_file_name_day,
		  const std::string &ref_file_name_night,
		  const int *hms_night, const int *hms_day,
		  int day_night_transition_delta_seconds);

  FieldDataPair avIqPtr(const time_t &t) const;
  FieldWithData phaseErPtr(const time_t &t) const;
  inline double refNDay(void) const {return _calibDay.refN();}

 private:

  Calib _calibDay;
  Calib _calibNight;
  int _hms_day[3];
  int _hms_night[3];
  double _hourDay;
  double _hourNight;
  int _transition_delta_seconds;

  void _weights(const time_t &t, double &wDay, double &wNight) const;
};


#endif
