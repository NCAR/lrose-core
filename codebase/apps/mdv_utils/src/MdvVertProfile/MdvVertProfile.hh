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
 *   $Date: 2016/03/04 02:22:13 $
 *   $Id: MdvVertProfile.hh,v 1.3 2016/03/04 02:22:13 dixon Exp $
 *   $Revision: 1.3 $
 *   $State: Exp $
 */
 
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/

/************************************************************************
 * MdvVertProfile: MdvVertProfile program object.
 *
 * RAP, NCAR, Boulder CO
 *
 * May 2008
 *
 * Nancy Rehak
 *
 ************************************************************************/

#ifndef MdvVertProfile_HH
#define MdvVertProfile_HH


#include <string>
#include <sys/time.h>
#include <vector>

#include <dsdata/DsTrigger.hh>
#include <Mdv/DsMdvx.hh>
#include <toolsa/DateTime.hh>

#include "Args.hh"
#include "Output.hh"
#include "Params.hh"
#include "ProfileSet.hh"
#include "VertProfile.hh"

using namespace std;

class MdvVertProfile
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

  ~MdvVertProfile(void);
  

  /*********************************************************************
   * Inst() - Retrieve the singleton instance of this class.
   */

  static MdvVertProfile *Inst(int argc, char **argv);
  static MdvVertProfile *Inst();
  

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

  static MdvVertProfile *_instance;
  
  // Program parameters.

  char *_progName;
  Args *_args;
  Params *_params;
  
  DsTrigger *_dataTrigger;
  
  Output *_output;
  

  /////////////////////
  // Private methods //
  /////////////////////

  /*********************************************************************
   * Constructor -- private because this is a singleton object
   */

  MdvVertProfile(int argc, char **argv);
  

  /*********************************************************************
   * _extractHtProfile() - Extract the vertical profile for the height field.
   */

  void _extractHtProfile(const Mdvx::field_header_t field_hdr,
			 const Mdvx::vlevel_header_t &vlevel_hdr,
			 VertProfile &profile) const;
  

  /*********************************************************************
   * _extractVertProfile() - Extract a vertical profile from the given field
   *                         for the given point.
   */

  bool _extractVertProfile(const MdvxField &field,
			   const double lat, const double lon,
			   const double output_missing_value,
			   VertProfile &profile) const;
  

  /*********************************************************************
   * _extractVertProfiles() - Extract the vertical profiles for the given
   *                          point.
   *
   * Returns TRUE on success, FALSE on failure.
   */

  bool _extractVertProfiles(const DsMdvx &mdvx,
			    const double lat, const double lon,
			    vector< ProfileSet > &profile_sets);
  

  /*********************************************************************
   * _initOutput() - Initialize the output object.
   *
   * Returns TRUE on success, FALSE on failure.
   */

  bool _initOutput();
  

  /*********************************************************************
   * _initTrigger() - Initialize the data trigger object.
   *
   * Returns TRUE on success, FALSE on failure.
   */

  bool _initTrigger();
  

  /*********************************************************************
   * _processData() - Process the data for the given time.
   *
   * Returns TRUE on success, FALSE on failure.
   */

  bool _processData(const DateTime &trigger_time);
  

  /*********************************************************************
   * _updateFieldNames() - Update the field names in the output file.
   */
  
  void _updateFieldNames(Mdvx &fcst_mdvx);
  

  /*********************************************************************
   * _readData() - Read the data for the given time.
   *
   * Returns TRUE on success, FALSE on failure.
   */

  bool _readData(DsMdvx &mdvx,
		 const DateTime &read_time) const;
  

};


#endif
