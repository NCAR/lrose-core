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
 *   $Id: MdvThreat.hh,v 1.4 2016/03/04 02:22:12 dixon Exp $
 *   $Revision: 1.4 $
 *   $State: Exp $
 */

/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/

/************************************************************************
 * MdvThreat: MdvThreat program object.
 *
 * RAL, NCAR, Boulder CO
 *
 * August 2008
 *
 * Jeff Copeland
 *
 ************************************************************************/

#ifndef MdvThreat_HH
#define MdvThreat_HH

#include <map>
#include <string>
#include <sys/time.h>

#include <dsdata/DsTrigger.hh>
#include <Mdv/Mdvx.hh>
#include <Mdv/MdvxPjg.hh>
#include <toolsa/DateTime.hh>
#include <Mdv/DsMdvx.hh>

#include "Args.hh"
#include "Params.hh"

using namespace std;


class MdvThreat
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

  ~MdvThreat(void);


  /*********************************************************************
   * Inst() - Retrieve the singleton instance of this class.
   */

  static MdvThreat *Inst(int argc, char **argv);
  static MdvThreat *Inst();


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

  static const string DATA_TIME_FIELD_NAME;
  static const ui32 DATA_TIME_MISSING_VALUE;


  /////////////////////
  // Private members //
  /////////////////////

  // Singleton instance pointer

  static MdvThreat *_instance;

  // Program parameters.

  char *_progName;
  Args *_args;
  Params *_params;

  // Triggering object

  DsTrigger *_dataTrigger;

  // Input projection

  mutable MdvxPjg _inputProj;

  /////////////////////
  // Private methods //
  /////////////////////

  /*********************************************************************
   * Constructor -- private because this is a singleton object
   */

  MdvThreat(int argc, char **argv);


  /*********************************************************************
   * _createThreatTimeField() - Create the threat time
   *                           field.  This field will have the given time
   *                           where the given field has a data value, and
   *                           will have a missing data value where the
   *                           given field data is missing.
   *
   * Returns a pointer to the created field on success, 0 on failure.
   */

  MdvxField *_createThreatTimeField(const MdvxField &input_field, float forecast_delta) const;


  /*********************************************************************
   * _createThreat() - Create a new threat file based on the
   *                         given input file.
   *
   * Returns true on success, false on failure.
   */

  bool _createThreat(const Mdvx &input_file,
			   Mdvx &threat_file) const;


  /*********************************************************************
   * __createThreatMasterHeader() - Create the threat file master
   *                               header based on the information in
   *                               the given master header.
   */

  void _createThreatMasterHeader(Mdvx &threat_file,
				const Mdvx::master_header_t &master_hdr,
				const string &input_file_path) const;

  /*********************************************************************
   * _initInputProj() - Initialize the input projection.
   *
   * Returns true on success, false on failure.
   */

  bool _initInputProj(void);


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

  bool _processThreat(const DateTime &trigger_time, DsMdvx &threat_file, bool *createThreatFile);


  /*********************************************************************
   * _readInputFile() - Read the indicated input file.
   *
   * Returns true on success, false on failure.
   */

  bool _readInputFile(Mdvx &input_file,
		      const DateTime &trigger_time) const;


  /*********************************************************************
   * _updateThreatTimeField() - Update the threat time field with
   *                           the new input time.  The time values will
   *                           only be updated where the input field has
   *                           valid data and the threat field is missing.
   *
   * Returns true on success, false on failure.
   */

  bool _updateThreatTimeField(MdvxField &threat_field, const MdvxField &input_field, float forecast_delta) const;

//  bool _updateThreatTimeField(MdvxField &time_field,
//			     const DateTime &data_time) const;


  /*********************************************************************
   * _updateThreat() - Update the given threat file based on the
   *                         given input file.
   *
   * Returns true on success, false on failure.
   */

  bool _updateThreat(const Mdvx &input_file,
			   Mdvx &threat_file) const;


  /*********************************************************************
   * __updateThreatMasterHeader() - Update the threat file master
   *                               header based on the information in
   *                               the given master header.
   */

  void _updateThreatMasterHeader(Mdvx &threat_file,
				const Mdvx::master_header_t &master_hdr,
				const string &input_file_path) const;

  /*********************************************************************
    * _smoothField() - Smooth the threat time field
    *
    * Returns true on success, false on failure.
    */

   bool _smoothField(Mdvx &threat_file, int filter_radius) const;

};


#endif
