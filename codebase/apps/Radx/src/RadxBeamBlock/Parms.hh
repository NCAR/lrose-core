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
 * @file Parms.hh
 * @brief Parms object, derived class for parameter handling
 * @class Parms
 * @brief Parms object, derived class for parameter handling
 *
 * Dave Albo, RAP, NCAR
 *
 * P.O.Box 3000, Boulder, CO, 80307-3000, USA
 *
 * March 2014
 *
 * Parms has Params as a base class, and provides additional methods/members
 */

#ifndef PARMS_HH
#define PARMS_HH

#include "Params.hh"
#include <string>
#include <vector>

class Parms : public Params
{
public:
  /**
   * Constructor
   * @param[in] p  Params base class
   * @param[in] progName  Name of program
   */
  Parms (const Params &p, const std::string &progName);

  /**
   * Destructor
   */
  virtual ~Parms(void);

  inline int ngates(void) const {return Params::gates.count; }
  inline int nazimuth(void) const {return Params::azimuths.count; }
  inline int nelev(void) const {return Params::elevations.count;}

  inline double gate0(void) const {return Params::gates.start;}
  inline double azimuth0(void) const {return Params::azimuths.start;}
  inline double elev0(void) const {return Params::elevations.start;}

  inline double ithGate(int i) const {return gate0() + i*Params::gates.delta;}
  inline double ithAzimuth(int i) const {return azimuth0() + 
      i*Params::azimuths.delta;}
  inline double ithElev(int i) const {return elev0() + 
      i*Params::elevations.delta;}

  inline int elevToIndex(double elev) const
  {
    return (elev - elev0())/Params::elevations.delta;
  }

  /**
   * Figure out the min/max lat/lon from parameterized volume specification
   * @param[out] sw  The southwest lat/lon, lat=first
   * @param[out] ne  The northeast lat/lon, lat=first
   */
  void latlonExtrema(std::pair<double,double> &sw,
		     std::pair<double,double> &ne) const;

  /**
   * @return field name for a particular data type
   * @param[in] t  Type
   */
  std::string fieldName(Params::output_data_t t) const;


  std::string _progName;  /**< The program name */

  time_t _utime;

protected:
private:

};

#endif

