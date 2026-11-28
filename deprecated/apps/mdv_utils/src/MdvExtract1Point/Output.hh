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
 * @file Output.hh
 *
 * @class Output
 *
 * Base class for output handlers.
 *  
 * @date 8/5/2009
 *
 */

#ifndef Output_HH
#define Output_HH

#include <toolsa/DateTime.hh>

using namespace std;

/** 
 * @class Output
 */

class Output
{
 public:

  //////////////////////
  // Public constants //
  //////////////////////

  /**
   * @brief Value indicating a missing data value.
   */

  static const double MISSING_VALUE;
  

  ////////////////////
  // Public methods //
  ////////////////////

  //////////////////////////////
  // Constructors/Destructors //
  //////////////////////////////

  /**
   * @brief Constructor
   */

  Output(const bool debug_flag = false,
	 const bool verbose_flag = false);
  

  /**
   * @brief Destructor
   */

  virtual ~Output(void);
  

  ////////////////////////
  // Processing methods //
  ////////////////////////

  /**
   * @brief Initialize the output for this data volume.  Must be
   *        called before processing each new input volume.
   *
   * @param[in] data_time Data time for this volume.
   *
   * @return Returns true on success, false on failure.
   */

  virtual bool init(const DateTime &data_time);
  

  /**
   * @brief Add the given point to the output.
   *
   * @param[in] location_id Location identifier.
   * @param[in] lat Latitude of point.
   * @param[in] lon Longitude of point.
   * @param[in] alt Altitude of point.
   * @param[in] grid_size Size of grid squares in km^2.
   * @param[in] value Point value.
   * @param[in] value_nw Value of point "northwest" of this point in grid.
   * @param[in] value_n Value of point "north" of this point in grid.
   * @param[in] value_ne Value of point "northeast" of this point in grid.
   * @param[in] value_e Value of point "east" of this point in grid.
   * @param[in] value_se Value of point "southeast" of this point in grid.
   * @param[in] value_s Value of point "south" of this point in grid.
   * @param[in] value_sw Value of point "southwest" of this point in grid.
   * @param[in] value_w Value of point "west" of this point in grid.
   *
   * @return Returns true on success, false on failure.
   */

  virtual bool addPoint(const int location_id,
			const double lat, const double lon,
			const double alt, const double grid_size,
			const double value, const double value_nw,
			const double value_n, const double value_ne,
			const double value_e, const double value_se,
			const double value_s, const double value_sw,
			const double value_w) = 0;
  

  /**
   * @brief Close the output for this data volume.
   *
   * @return Returns true on success, false on failure.
   */

  virtual bool close() = 0;
  
protected:

  ///////////////////////
  // Protected members //
  ///////////////////////

  /**
   * @brief Debug flag.
   */

  bool _debug;
  

  /**
   * @brief Verbose debug flag.
   */

  bool _verbose;
  

  /**
   * @brief Data time for the current data volume.
   */

  DateTime _dataTime;
  

  ///////////////////////
  // Protected methods //
  ///////////////////////

  /**
   * @brief Internal initialization method.  Each subclass of this class must
   *        define its initialization here.
   *
   * @return Returns true on success, false on failure.
   */

  virtual bool _init() = 0;
  

};


#endif
