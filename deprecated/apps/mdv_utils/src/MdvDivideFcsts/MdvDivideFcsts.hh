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
 *   $Date: 2016/03/04 02:22:11 $
 *   $Id: MdvDivideFcsts.hh,v 1.2 2016/03/04 02:22:11 dixon Exp $
 *   $Revision: 1.2 $
 *   $State: Exp $
 */
 
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/

/************************************************************************
 * MdvDivideFcsts: MdvDivideFcsts program object.
 *
 * RAP, NCAR, Boulder CO
 *
 * December 2007
 *
 * Nancy Rehak
 *
 ************************************************************************/

#ifndef MdvDivideFcsts_HH
#define MdvDivideFcsts_HH

#include <string>

#include <dsdata/DsTrigger.hh>
#include <Mdv/DsMdvx.hh>
#include <Mdv/MdvxField.hh>
#include <Mdv/MdvxPjg.hh>
#include <toolsa/DateTime.hh>

#include "Args.hh"
#include "Params.hh"

using namespace std;


class MdvDivideFcsts
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

  ~MdvDivideFcsts(void);
  

  /*********************************************************************
   * Inst() - Retrieve the singleton instance of this class.
   */

  static MdvDivideFcsts *Inst(int argc, char **argv);
  static MdvDivideFcsts *Inst();
  

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

  static MdvDivideFcsts *_instance;
  
  // Program parameters.

  char *_progName;
  Args *_args;
  Params *_params;
  
  // Triggering object

  DsTrigger *_dataTrigger;
  

  /////////////////////
  // Private methods //
  /////////////////////

  /*********************************************************************
   * Constructor -- private because this is a singleton object
   */

  MdvDivideFcsts(int argc, char **argv);
  

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

  bool _processData(const DateTime &trigger_time);
  

  /*********************************************************************
   * _readInputData() - Read the indicated input data.
   *
   * Returns true on success, false on failure.
   */

  bool _readInputData(DsMdvx &mdvx,
		      const string &url,
		      const DateTime &trigger_time) const;
  

  /*********************************************************************
   * _writeFcstField() - Write the given field to the indicated URL as a 
   *                     forecast field with the indicated lead time.
   */

  void _writeFcstField(const MdvxField &input_field,
		       const int fcst_lead_secs,
		       const string fcst_url,
		       const string output_field_name,
		       const Mdvx::master_header_t input_master_hdr) const;
  

};


#endif
