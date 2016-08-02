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
/*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*/

/* RCS info
 *   $Author: dixon $
 *   $Locker:  $
 *   $Date: 2016/03/04 01:28:11 $
 *   $Id: MdvFilter.hh,v 1.5 2016/03/04 01:28:11 dixon Exp $
 *   $Revision: 1.5 $
 *   $State: Exp $
 */
 
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/

/************************************************************************
 * MdvFilter: Base class for filtering Tstorms based on a 2-dimensional
 *            MDV field.
 *
 * RAP, NCAR, Boulder CO
 *
 * February 2003
 *
 * Nancy Rehak
 *
 ************************************************************************/

#ifndef MdvFilter_HH
#define MdvFilter_HH

#include <Mdv/MdvxField.hh>

#include "Filter.hh"
using namespace std;

class MdvFilter : public Filter
{
 public:

  //////////////////
  // Public types //
  //////////////////

  typedef enum
  {
    MISSING_NO_OUTPUT,
    MISSING_PASSING,
    MISSING_FAILING
  } missing_input_action_t;
  
  
  ////////////////////
  // Public methods //
  ////////////////////

  //////////////////////////////
  // Constructors/Destructors //
  //////////////////////////////

  /*********************************************************************
   * Constructor -- private because this is a singleton object
   */

  MdvFilter(const string& url,
	    const string& field_name,
	    const int level_num,
	    const int max_valid_secs,
	    const double storm_growth_km,
	    const missing_input_action_t missing_input_action = MISSING_NO_OUTPUT,
	    const bool debug = false);
  

  /*********************************************************************
   * Destructor
   */

  virtual ~MdvFilter(void);


  ////////////////////////////
  // Initialization Methods //
  ////////////////////////////

  /*********************************************************************
   * initFilter() - Initialize the filter based on the data time of the
   *                storms.  This method must be called before the first
   *                call to isPassing() and must be called again when
   *                processing a storm with a new data time.
   *
   * Returns true if the initialization was successful, false otherwise.
   */

  virtual bool initFilter(const DateTime& data_time);


  ////////////////////
  // Access Methods //
  ////////////////////

  /*********************************************************************
   * isPassing() - Returns true if the storm passes the filter criteria,
   *               false otherwise.
   */

  virtual bool isPassing(const Tstorm& storm,
			 double& algorithm_value);


protected:

  ///////////////////////
  // Protected members //
  ///////////////////////

  string _url;
  string _fieldName;
  int _levelNum;
  int _maxValidSecs;
  missing_input_action_t _missingInputAction;
  
  double _stormGrowthKm;
  
  bool _filterInitialized;
  bool _mdvDataAvailable;
  
  MdvxField *_field;
  
  
  ///////////////////////
  // Protected methods //
  ///////////////////////

  /*********************************************************************
   * _isPassing() - Determines if the storm passes the criteria.  The
   *                derrived class must define this method.
   */

  virtual bool _isPassing(const WorldPolygon2D *polygon,
			  double &algorithm_value) = 0;
  

  /*********************************************************************
   * _readGrid() - Read in the appropriate grid.
   */

  bool _readGrid(const DateTime& data_time);
  

};


#endif
