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
 *   $Id: CalcHumidity.hh,v 1.2 2016/03/06 23:15:37 dixon Exp $
 *   $Revision: 1.2 $
 *   $State: Exp $
 */
 
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/

/************************************************************************
 * CalcHumidity: CalcHumidity program object.
 *
 * RAP, NCAR, Boulder CO
 *
 * June 2003
 *
 * Nancy Rehak
 *
 ************************************************************************/

#ifndef CalcHumidity_HH
#define CalcHumidity_HH


#include <string>
#include <sys/time.h>

#include <dsdata/DsTrigger.hh>
#include <Mdv/DsMdvx.hh>
#include <Mdv/MdvxField.hh>

#include "Args.hh"
#include "Params.hh"


class CalcHumidity
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

  ~CalcHumidity(void);
  

  /*********************************************************************
   * Inst() - Retrieve the singleton instance of this class.
   */

  static CalcHumidity *Inst(int argc, char **argv);
  static CalcHumidity *Inst();
  

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

  static const fl32 HUMIDITY_MISSING_DATA_VALUE;
  

  /////////////////////
  // Private members //
  /////////////////////

  // Singleton instance pointer

  static CalcHumidity *_instance;
  
  // Program parameters.

  char *_progName;
  Args *_args;
  Params *_params;
  
  DsTrigger *_dataTrigger;
  

  /////////////////////
  // Private methods //
  /////////////////////

  /*********************************************************************
   * Constructor -- private because this is a singleton object
   */

  CalcHumidity(int argc, char **argv);
  

  /*********************************************************************
   * _createHumidityField() - Create the empty humidity field.  The data
   *                          volume will be filled with missing data
   *                          values when returned.
   *
   * Returns a pointer to the newly created field on success, 0 on failure.
   */

  MdvxField *_createHumidityField(const Mdvx::field_header_t base_field_hdr,
				  const Mdvx::vlevel_header_t base_vlevel_hdr);
  

  /*********************************************************************
   * _processData() - Process the data for the given time.
   */

  bool _processData(const time_t trigger_time);
  

  /*********************************************************************
   * _readField() - Read the indicated field data.
   */

  static MdvxField *_readField(const time_t data_time,
			       const string &url,
			       const string &field_name,
			       const int field_num,
			       const int max_input_valid_secs);
  
  /*********************************************************************
   * _updateMasterheader() - Update the master header for the output file.
   */
  
  void _updateMasterHeader(DsMdvx &output_file,
			   const time_t data_time,
			   const Mdvx::field_header_t &field_hdr);
  

};


#endif
