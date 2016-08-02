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
 *   $Date: 2016/03/04 01:28:14 $
 *   $Id: TstormGrow.hh,v 1.2 2016/03/04 01:28:14 dixon Exp $
 *   $Revision: 1.2 $
 *   $State: Exp $
 */
 
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/

/************************************************************************
 * TstormGrow: TstormGrow program object.
 *
 * RAP, NCAR, Boulder CO
 *
 * Nov 2007
 *
 * Nancy Rehak
 *
 ************************************************************************/

#ifndef TstormGrow_HH
#define TstormGrow_HH

#include <string>
#include <vector>

#include <dsdata/DsTrigger.hh>
#include <dsdata/TstormMgr.hh>
#include <Mdv/DsMdvx.hh>
#include <toolsa/DateTime.hh>

#include "Args.hh"
#include "Params.hh"

using namespace std;


class TstormGrow
{
 public:

  ////////////////////
  // Public members //
  ////////////////////

  // Flag indicating whether the program status is currently okay.

  bool okay;


  ////////////////////
  // Public methods //
  ////////////////////

  //////////////////////////////
  // Constructors/Destructors //
  //////////////////////////////

  /*********************************************************************
   * Destructor
   */

  ~TstormGrow(void);
  

  /*********************************************************************
   * Inst() - Retrieve the singleton instance of this class.
   */

  static TstormGrow *Inst(int argc, char **argv);
  static TstormGrow *Inst();
  

  /*********************************************************************
   * init() - Initialize the local data.
   *
   * Returns true if the initialization was successful, false otherwise.
   */

  bool init();
  

  /////////////////////
  // Running methods //
  /////////////////////

  /*********************************************************************
   * run() - run the program.
   */

  void run();
  

 private:

  /////////////////////
  // Private members //
  /////////////////////

  // Singleton instance pointer

  static TstormGrow *_instance;
  
  // Program parameters.

  char *_progName;
  Args *_args;
  Params *_params;
  
  // Forecast lead time in hours

  double _leadTimeHrs;
  
  // Triggering object

  DsTrigger *_dataTrigger;
  
  // Object for managing the tstorm data

  TstormMgr _tstormMgr;
  unsigned char *_polygonGrid;
  int _polygonGridSize;
  

  /////////////////////
  // Private methods //
  /////////////////////

  /*********************************************************************
   * Constructor -- private because this is a singleton object
   */

  TstormGrow(int argc, char **argv);
  

  /*********************************************************************
   * _applyGrowthToStorm() - Apply the growth and decay to the given
   *                         storm in the grid.
   *
   * Returns true on success, false on failure.
   */

  bool _applyGrowthToStorm(MdvxField &growth_field,
			   fl32 *orig_grid,
			   const Tstorm &storm) const;
  

  /*********************************************************************
   * _applyGrowthToStorms() - Apply the growth and decay to the storms
   *                          in the grid.
   *
   * Returns true on success, false on failure.
   */

  bool _applyGrowthToStorms(DsMdvx &mdvx);
  

  /*********************************************************************
   * _decayStorm() - Apply the decay to the given storm in the grid.
   *
   * Returns true on success, false on failure.
   */

  bool _decayStorm(MdvxField &growth_field,
		   fl32 *orig_grid,
		   const Tstorm &storm) const;
  

  /*********************************************************************
   * _growStorm() - Apply the growth to the given storm in the grid.
   *
   * Returns true on success, false on failure.
   */

  bool _growStorm(MdvxField &growth_field,
		  fl32 *orig_grid,
		  const Tstorm &storm) const;
  

  /*********************************************************************
   * _initTrigger() - Initialize the data trigger.
   *
   * Returns true on success, false on failure.
   */

  bool _initTrigger(void);
  

  /*********************************************************************
   * _performRead() - Perform the read.  This method just consolidates
   *                  the parts of the reading of the input fields that
   *                  is common between fields.
   *
   * Returns true on success, false on failure.
   */

  bool _performRead(DsMdvx &input_file,
		    const string &url,
		    const DateTime &trigger_time,
		    const int max_input_secs) const;
  

  /*********************************************************************
   * _processData() - Process data for the given trigger time.
   *
   * Returns true on success, false on failure.
   */

  bool _processData(const DateTime &trigger_time);
  

  /*********************************************************************
   * _readInput() - Read the indicated input.
   *
   * Returns true on success, false on failure.
   */

  bool _readInput(DsMdvx &mdvx,
		  const string &url,
		  const string &field_name,
		  const DateTime &trigger_time,
		  const int max_input_secs) const;
  
  bool _readInput(DsMdvx &mdvx,
		  const string &url,
		  const int field_num,
		  const DateTime &trigger_time,
		  const int max_input_secs) const;
  

};


#endif
