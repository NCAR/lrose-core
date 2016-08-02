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


  /**
   * @param[in] name  output field name
   */
  bool matchingOutput(Params::output_data_t type, std::string &name) const;
  /**
   * @param[in] name  output field name
   */
  const Params::output_field_t *matchingOutput(const std::string &name) const;
  

  /**
   * @param[in] name  Rainrate output field name
   */
  const Params::rainrate_field_t *matchingRainrate(const std::string &name) const;

  /**
   * @return true if input named field is a mask field
   */
  bool isMask(const std::string &name) const;

  bool hasSnr(void) const;

  int numRainRate(void) const;
  std::string ithOutputRateName(int i) const;
  std::string ithInputPrecipName(int i) const;

  std::string _progName;  /**< The program name */

protected:
private:

};

#endif

