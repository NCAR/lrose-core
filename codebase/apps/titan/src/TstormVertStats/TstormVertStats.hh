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
 *   $Date: 2016/03/04 02:01:42 $
 *   $Id: TstormVertStats.hh,v 1.2 2016/03/04 02:01:42 dixon Exp $
 *   $Revision: 1.2 $
 *   $State: Exp $
 */
 
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/

/************************************************************************
 * TstormVertStats: TstormVertStats program object.
 *
 * RAP, NCAR, Boulder CO
 *
 * Sept 2007
 *
 * Nancy Rehak
 *
 ************************************************************************/

#ifndef TstormVertStats_HH
#define TstormVertStats_HH

#include <string>
#include <vector>

#include <dsdata/DsTrigger.hh>
#include <dsdata/TstormMgr.hh>
#include <Mdv/DsMdvx.hh>
#include <Mdv/MdvxField.hh>
#include <toolsa/DateTime.hh>

#include "Args.hh"
#include "Params.hh"
#include "Output.hh"
#include "Stat.hh"

using namespace std;


class TstormVertStats
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

  ~TstormVertStats(void);
  

  /*********************************************************************
   * Inst() - Retrieve the singleton instance of this class.
   */

  static TstormVertStats *Inst(int argc, char **argv);
  static TstormVertStats *Inst();
  

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

  static TstormVertStats *_instance;
  
  // Program parameters.

  char *_progName;
  Args *_args;
  Params *_params;
  
  // Triggering object

  DsTrigger *_dataTrigger;
  
  // Object for managing the tstorm data

  TstormMgr _tstormMgr;
  unsigned char *_polygonGrid;
  int _polygonGridSize;
  
  // List of statistics calculators

  vector< Stat* > _statistics;
  
  // Output object

  Output *_output;
  

  /////////////////////
  // Private methods //
  /////////////////////

  /*********************************************************************
   * Constructor -- private because this is a singleton object
   */

  TstormVertStats(int argc, char **argv);
  

  /*********************************************************************
   * _calcLevelStatistics() - Calculate the statistics for given vertical
   *                          level of data.
   *
   * Returns a vector of the calculated statistics.
   */

  vector< double > _calcLevelStatistics(const Pjg &proj,
					const fl32 *data_grid,
					const int min_x,
					const int min_y,
					const int max_x,
					const int max_y,
					const fl32 missing_data_value,
					const fl32 bad_data_value);
  

  /*********************************************************************
   * _calcStatistics() - Calculate the statistics for this field
   *
   * Returns true on success, false on failure.
   */

  bool _calcStatistics(const MdvxField &field);
  

  /*********************************************************************
   * _calcStormStatistics() - Calculate the statistics for this field and
   *                          storm.
   *
   * Returns true on success, false on failure.
   */

  bool _calcStormStatistics(const MdvxField &field,
			    const Tstorm &storm);
  

  /*********************************************************************
   * _initOutput() - Initialize the object that creates the output.
   *
   * Returns true on success, false on failure.
   */

  bool _initOutput(void);
  

  /*********************************************************************
   * _initStatistics() - Initialize the list of statistics.
   *
   * Returns true on success, false on failure.
   */

  bool _initStatistics(void);
  

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
   * Returns the newly read field on success, 0 on failure.
   */

  MdvxField *_performRead(DsMdvx &input_file,
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
   * _readInputField() - Read the indicated input field.
   *
   * Returns the newly read field on success, 0 on failure.
   */

  MdvxField *_readInputField(const string &url,
			     const string &field_name,
			     const DateTime &trigger_time,
			     const int max_input_secs) const;
  
  MdvxField *_readInputField(const string &url,
			     const int field_num,
			     const DateTime &trigger_time,
			     const int max_input_secs) const;
  

};


#endif
