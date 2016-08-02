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
 * @file VolHandler.hh
 * @brief Handles the data that is created and written out
 * @class VolHandler
 * @brief Handles the data that is created and written out
 */

#ifndef VOLHANDLER_HH
#define VOLHANDLER_HH

#include <string>
#include <vector>

#include "Parms.hh"
#include "ScanHandler.hh"
#include <Radx/RadxVol.hh>

class RayHandler;

class VolHandler
{
public:

  /**
   * @param[in] params
   */
  VolHandler (const Parms &params);

  /**
   * Destructor
   */
  ~VolHandler(void);

  /**
   * Apply Finishing steps to _scan data, then copy to _vol
   */
  void finish(void);

  /**
   * Write _vol data
   */
  int write(void);

  typedef vector<ScanHandler>::iterator iterator;
  typedef vector<ScanHandler>::const_iterator const_iterator;
  typedef vector<ScanHandler>::reverse_iterator reverse_iterator;
  typedef vector<ScanHandler>::const_reverse_iterator const_reverse_iterator;
  auto begin() -> iterator                       { return _scan.begin(); }
  auto begin() const -> const_iterator           { return _scan.begin(); }
  auto end() -> iterator                         { return _scan.end(); }
  auto end() const -> const_iterator             { return _scan.end(); }

  bool isOK;   /**< True if object is good to go */

protected:
private:

  RadxVol _vol;                    /**< Used for output */
  Parms _params;                   /**< App params */
  std::vector<ScanHandler> _scan;  /**< The data created, one per scan */

  void _addBeam(const RayHandler &beam, double elev, int ielev);
  void _addFieldToRay(const Params::output_field_t &ofield,
		      const RayHandler &beam, RadxRay *ray);
  int _writeVol(void);
  void _setupWrite(RadxFile &file);
  
};

#endif
