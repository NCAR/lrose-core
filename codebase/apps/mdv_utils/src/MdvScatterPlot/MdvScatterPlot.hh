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
 *   $Date: 2016/03/04 02:22:12 $
 *   $Id: MdvScatterPlot.hh,v 1.5 2016/03/04 02:22:12 dixon Exp $
 *   $Revision: 1.5 $
 *   $State: Exp $
 */
 
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/

/************************************************************************
 * MdvScatterPlot: MdvScatterPlot program object.
 *
 * RAP, NCAR, Boulder CO
 *
 * April 2007
 *
 * Nancy Rehak
 *
 ************************************************************************/

#ifndef MdvScatterPlot_HH
#define MdvScatterPlot_HH

#include <string>

#include <dsdata/DsTrigger.hh>
#include <Mdv/DsMdvx.hh>
#include <Mdv/MdvxField.hh>
#include <Mdv/MdvxPjg.hh>
#include <toolsa/DateTime.hh>

#include "Args.hh"
#include "Params.hh"
#include "Plotter.hh"

using namespace std;


class MdvScatterPlot
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

  ~MdvScatterPlot(void);
  

  /*********************************************************************
   * Inst() - Retrieve the singleton instance of this class.
   */

  static MdvScatterPlot *Inst(int argc, char **argv);
  static MdvScatterPlot *Inst();
  

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

  ///////////////////////
  // Private constants //
  ///////////////////////


  /////////////////////
  // Private members //
  /////////////////////

  // Singleton instance pointer

  static MdvScatterPlot *_instance;
  
  // Program parameters.

  char *_progName;
  Args *_args;
  Params *_params;
  
  // Triggering object

  DsTrigger *_dataTrigger;
  
  // Input projection

  mutable MdvxPjg _inputProj;
  
  // Plotter object

  Plotter *_plotter;
  

  /////////////////////
  // Private methods //
  /////////////////////

  /*********************************************************************
   * Constructor -- private because this is a singleton object
   */

  MdvScatterPlot(int argc, char **argv);
  

  /*********************************************************************
   * _applyThresholds() - Apply the given thresholds to the given data
   *                      field.
   */
  
  void _applyThresholds(MdvxField *field,
			const bool apply_min_thresh,
			const double min_thresh,
			const bool apply_max_thresh,
			const double max_thresh) const;
  

  /*********************************************************************
   * _getMdvReadSearchMode() - Convert the read search mode given in the
   *                           parameter file to the matching MDV value.
   *
   * Returns the matching MDV read search mode,
   */

  Mdvx::read_search_mode_t _getMdvReadSearchMode(const Params::read_search_mode_t search_mode) const;
  

  /*********************************************************************
   * _initInputProj() - Initialize the input projection.
   *
   * Returns true on success, false on failure.
   */

  bool _initInputProj(void);
  

  /*********************************************************************
   * _initPlotter() - Initialize the object that creates the scatter plots.
   *
   * Returns true on success, false on failure.
   */

  bool _initPlotter(void);
  

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
			  const int max_input_secs,
			  const Mdvx::read_search_mode_t search_mode,
			  const int forecast_lead_secs,
			  const bool process_single_level,
			  const int level_num) const;
  

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
			     const int max_input_secs,
			     const Mdvx::read_search_mode_t search_mode,
			     const int forecast_lead_secs,
			     const bool apply_min_thresh,
			     const double min_thresh,
			     const bool apply_max_thresh,
			     const double max_thresh,
			     const bool process_single_level,
			     const int level_num) const;
  
  MdvxField *_readInputField(const string &url,
			     const int field_num,
			     const DateTime &trigger_time,
			     const int max_input_secs,
			     const Mdvx::read_search_mode_t search_mode,
			     const int forecast_lead_secs,
			     const bool apply_min_thresh,
			     const double min_thresh,
			     const bool apply_max_thresh,
			     const double max_thresh,
			     const bool process_single_level,
			     const int level_num) const;
  

};


#endif
