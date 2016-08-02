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
 *   $Date: 2016/03/04 02:22:10 $
 *   $Id: Mdv3Dto2D.hh,v 1.2 2016/03/04 02:22:10 dixon Exp $
 *   $Revision: 1.2 $
 *   $State: Exp $
 */
 
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/

/************************************************************************
 * Mdv3Dto2D: Mdv3Dto2D program object.
 *
 * RAP, NCAR, Boulder CO
 *
 * March 2008
 *
 * Nancy Rehak
 *
 ************************************************************************/

#ifndef Mdv3Dto2D_HH
#define Mdv3Dto2D_HH

#include <string>

#include <dsdata/DsTrigger.hh>
#include <Mdv/DsMdvx.hh>
#include <Mdv/MdvxField.hh>
#include <Mdv/MdvxPjg.hh>
#include <toolsa/DateTime.hh>

#include "Args.hh"
#include "Params.hh"

#include "Computer.hh"

using namespace std;


class Mdv3Dto2D
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

  ~Mdv3Dto2D(void);
  

  /*********************************************************************
   * Inst() - Retrieve the singleton instance of this class.
   */

  static Mdv3Dto2D *Inst(int argc, char **argv);
  static Mdv3Dto2D *Inst();
  

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

  static Mdv3Dto2D *_instance;
  
  // Program parameters.

  char *_progName;
  Args *_args;
  Params *_params;
  
  // Triggering object

  DsTrigger *_dataTrigger;
  
  // Object for computing the 2D field from the 3D field

  Computer *_computer;
  

  /////////////////////
  // Private methods //
  /////////////////////

  /*********************************************************************
   * Constructor -- private because this is a singleton object
   */

  Mdv3Dto2D(int argc, char **argv);
  

  /*********************************************************************
   * _initComputer() - Initialize the value computer.
   *
   * Returns true on success, false on failure.
   */

  bool _initComputer(void);
  

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
   * _readInputFile() - Read the indicated input field.
   *
   * Returns true on success, false on failure.
   */

  bool _readInputFile(DsMdvx &input_file,
		      const string &url,
		      const string &field_name,
		      const DateTime &trigger_time,
		      const int max_input_secs) const;
  

  bool _readInputFile(DsMdvx &input_file,
		      const string &url,
		      const int field_num,
		      const DateTime &trigger_time,
		      const int max_input_secs) const;
  

  /*********************************************************************
   * _writeOutputFile() - Create and write the output file.
   *
   * Returns true on success, false on failure.
   */

  bool _writeOutputFile(const Mdvx::master_header_t input_master_hdr,
			MdvxField *field_2d) const;
  

};


#endif
