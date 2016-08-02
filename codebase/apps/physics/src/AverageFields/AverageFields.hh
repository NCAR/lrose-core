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
 *   $Date: 2016/03/06 23:15:37 $
 *   $Id: AverageFields.hh,v 1.3 2016/03/06 23:15:37 dixon Exp $
 *   $Revision: 1.3 $
 *   $State: Exp $
 */
 
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/

/************************************************************************
 * AverageFields: AverageFields program object.
 *
 * RAP, NCAR, Boulder CO
 *
 * July 2002
 *
 * Nancy Rehak
 *
 ************************************************************************/

#ifndef AverageFields_HH
#define AverageFields_HH

/*
 **************************** includes **********************************
 */

#include <string>
#include <sys/time.h>

#include <dsdata/DsTrigger.hh>
#include <Mdv/DsMdvx.hh>
#include <Mdv/MdvxField.hh>

#include "Args.hh"
#include "Params.hh"

#include "GridCalculator.hh"

using namespace std;


class AverageFields
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

  ~AverageFields(void);
  

  /*********************************************************************
   * Inst() - Retrieve the singleton instance of this class.
   */

  static AverageFields *Inst(int argc, char **argv);
  static AverageFields *Inst();
  

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

  static AverageFields *_instance;
  
  // Program parameters.

  char *_progName;
  Args *_args;
  Params *_params;
  
  // Triggering object

  DsTrigger *_dataTrigger;
  
  // Object that calculates the averages

  GridCalculator *_gridCalc;
  

  /////////////////////
  // Private methods //
  /////////////////////

  /*********************************************************************
   * Constructor -- private because this is a singleton object
   */

  AverageFields(int argc, char **argv);
  

  /*********************************************************************
   * _createAverageField() - Create the average field using the given
   *                         information.
   *
   * Returns a pointer to the created field, or 0 if the field could
   * not be created for some reason.
   */

  MdvxField *_createAverageField(const Mdvx::field_header_t input_field_hdr,
				 const Mdvx::vlevel_header_t input_vlevel_hdr) const;
  

  /*********************************************************************
   * _initTrigger() - Initialize the data trigger.
   *
   * Returns true on success, false on failure.
   */

  bool _initTrigger(void);
  

  /*********************************************************************
   * _processData() - Process data for the given trigger time.
   *
   * Returns true on success, false on failure.
   */

  bool _processData(const time_t trigger_time);
  

  /*********************************************************************
   * _readField() - Read in the indicated input field
   *
   * Returns a pointer to the input field if successful, 0 otherwise.
   * Also returns the master header for the input file in the master_hdr
   * field.
   */

  MdvxField *_readField(const time_t data_time,
			const string &url,
			const string &field_name,
			const int field_num,
			Mdvx::master_header_t &master_hdr) const;
  

  /*********************************************************************
   * _updateOutputMasterHeader() - Update the master header values for
   *                               the output file.
   */

  static void _updateOutputMasterHeader(DsMdvx &output_file,
					const Mdvx::master_header_t &input_master_hdr);
  

};


#endif
