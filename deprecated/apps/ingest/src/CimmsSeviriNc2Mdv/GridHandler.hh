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
 * @file GridHandler.hh
 *
 * @class GridHandler
 *
 * Base class for classes that accumulate the data for the gridded fields.
 *  
 * @date 1/28/2010
 *
 */

#ifndef GridHandler_HH
#define GridHandler_HH

#include <dataport/port_types.h>
#include <euclid/Pjg.hh>

using namespace std;

/** 
 * @class GridHandler
 */

class GridHandler
{
 public:

  //////////////////////
  // Public constants //
  //////////////////////

  /**
   * @brief Missing value to use for data fields in the MDV file.
   */

  static const double MISSING_DATA_VALUE;


  ////////////////////
  // Public methods //
  ////////////////////

  /**
   * @brief Constructor
   *
   * @param[in] proj The data projection.
   * @param[in] debug_flag Debug flag.
   */

  GridHandler(const Pjg &proj, const bool debug_flag = false);
  

  /**
   * @brief Destructor
   */

  virtual ~GridHandler();
  

  /**
   * @brief Initialize the handler.
   *
   * @return Returns true on success, false on failure.
   */

  virtual bool init();
  

  /**
   * @brief Clear out the data in the grids.  This must be done before
   *        processing each file.
   */

  virtual void clearGridData() = 0;


  /**
   * @brief Add the given data element to the grid.
   *
   * @param[in] lat Data latitude.
   * @param[in] lon Data longitude.
   * @param[in] value Data value.
   */

  void addData(const double lat,
	       const double lon,
	       const double value);
  

  /**
   * @brief Get the final data grid.
   *
   * @return Returns a pointer to the data grid.  This pointer is owned by
   *         this object and must not be deleted by the calling method.
   */

  virtual fl32 *getGrid() = 0;
  

protected:

  ///////////////////////
  // Protected members //
  ///////////////////////

  /**
   * @brief Debug flag.
   */

  bool _debug;
  
  /**
   * @brief The data projection
   */

  Pjg _proj;
  

  ///////////////////////
  // Protected methods //
  ///////////////////////

  /**
   * @brief Add the given data element to the grid.
   *
   * @param[in] lat Data latitude.
   * @param[in] lon Data longitude.
   * @param[in] value Data value.
   */

  virtual void _addData(const double lat,
			const double lon,
			const double value) = 0;
  

  /**
   * @brief Initialize the internal object.
   *
   * @return Returns true on success, false on failure.
   */

  virtual bool _init() = 0;


};


#endif
