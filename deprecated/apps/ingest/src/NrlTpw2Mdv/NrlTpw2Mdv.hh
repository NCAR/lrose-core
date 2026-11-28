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
 *   $Author: jcraig $
 *   $Locker:  $
 *   $Date: 2018/01/22 19:54:16 $
 *   $Id: NrlTpw2Mdv.hh,v 1.4 2018/01/22 19:54:16 jcraig Exp $
 *   $Revision: 1.4 $
 *   $State: Exp $
 */
 
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/

/************************************************************************
 * NrlTpw2Mdv: NrlTpw2Mdv program object.
 *
 * RAP, NCAR, Boulder CO
 *
 * August 2007
 *
 * Nancy Rehak
 *
 ************************************************************************/

#ifndef NrlTpw2Mdv_HH
#define NrlTpw2Mdv_HH

#include <string>
#include <sys/time.h>
#include <vector>
#include <Ncxx/Nc3File.hh>
#include <dsdata/DsTrigger.hh>
#include <Mdv/MdvxPjg.hh>
#include <toolsa/DateTime.hh>

#include "Args.hh"
#include "Params.hh"

using namespace std;

class NrlTpw2Mdv
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

  ~NrlTpw2Mdv(void);
  

  /*********************************************************************
   * Inst() - Retrieve the singleton instance of this class.
   */

  static NrlTpw2Mdv *Inst(int argc, char **argv);
  static NrlTpw2Mdv *Inst();
  

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

  static const string LAT_ATT_NAME;
  static const string LON_ATT_NAME;
  
  static const string TPW_VAR_NAME;
  static const string TPW_UNITS_ATT_NAME;
  static const string TPW_MISSING_VALUE_ATT_NAME;
  
  static const double MISSING_DATA_VALUE;
  

  /////////////////////
  // Private members //
  /////////////////////

  // Singleton instance pointer

  static NrlTpw2Mdv *_instance;
  
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

  NrlTpw2Mdv(int argc, char **argv);
  

  /*********************************************************************
   * _getTpwField() - Get the TPW field from the given netCDF file and put
   *                  it into the given MDV file.
   *
   * Returns true on success, false on failure.
   */

  bool _getTpwField(Mdvx &mdvx,
		    const Nc3File &nc_file,
		    const string &input_file_path) const;
  

  /*********************************************************************
   * _initTrigger() - Initialize the data trigger.
   */

  bool _initTrigger();
  

  /*********************************************************************
   * _processFile() - Process the given file.
   */

  bool _processFile(const string &input_file_path);
  

  /*********************************************************************
   * _updateFieldHeader() - Update the field header for the TPW field 
   *                        based on information in the netCDF file.
   *
   * Returns true on success, false on failure.
   */

  bool _updateFieldHeader(Mdvx::field_header_t &field_hdr,
			  const Nc3File &nc_file,
			  const string &input_file_path,
			  const string &tpw_units) const;
  

  /*********************************************************************
   * _updateMasterHeader() - Update the master header in the given MDV
   *                         file using information from the given netCDF
   *                         file and file path.
   *
   * Returns true on success, false on failure.
   */

  bool _updateMasterHeader(Mdvx &mdvx,
			   const Nc3File &nc_file,
			   const string &input_file_path) const;
  

};


#endif
