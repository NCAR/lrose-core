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
 *   $Id: MdvAccumulate.hh,v 1.5 2016/03/04 02:22:10 dixon Exp $
 *   $Revision: 1.5 $
 *   $State: Exp $
 */
 
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/

/************************************************************************
 * MdvAccumulate: MdvAccumulate program object.
 *
 * RAP, NCAR, Boulder CO
 *
 * December 2006
 *
 * Nancy Rehak
 *
 ************************************************************************/

#ifndef MdvAccumulate_HH
#define MdvAccumulate_HH

#include <map>
#include <string>
#include <sys/time.h>

#include <dsdata/DsTrigger.hh>
#include <Mdv/Mdvx.hh>
#include <Mdv/MdvxPjg.hh>
#include <toolsa/DateTime.hh>

#include "Args.hh"
#include "Params.hh"
#include "BackgroundField.hh"
#include "QualityControl.hh"

using namespace std;


class MdvAccumulate
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

  ~MdvAccumulate(void);
  

  /*********************************************************************
   * Inst() - Retrieve the singleton instance of this class.
   */

  static MdvAccumulate *Inst(int argc, char **argv);
  static MdvAccumulate *Inst();
  

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

  static MdvAccumulate *_instance;
  
  // Program parameters.

  char *_progName;
  Args *_args;
  Params *_params;
  
  // Triggering object

  DsTrigger *_dataTrigger;
  
  // Input projection

  mutable MdvxPjg _inputProj;
  
  // Background field information

  map< string, BackgroundField > _backgroundFieldList;
  
  // Quality control objects for the accumulation fields.  Each quality
  // control object is stored in the map using the associated field name.

  map< string, QualityControl* > _qualityControllers;
  

  /////////////////////
  // Private methods //
  /////////////////////

  /*********************************************************************
   * Constructor -- private because this is a singleton object
   */

  MdvAccumulate(int argc, char **argv);
  

  /*********************************************************************
   * _createAccumTimeField() - Create the initial accumulation data time
   *                           field.  This field will have the given time
   *                           where the given field has a data value, and
   *                           will have a missing data value where the
   *                           given field data is missing.
   *
   * Returns a pointer to the created field on success, 0 on failure.
   */

  MdvxField *_createAccumTimeField(const MdvxField &input_field,
				   const DateTime &data_time,
				   const DateTime &bkgnd_time) const;
  

  /*********************************************************************
   * _createAccumulation() - Create a new accumulation file based on the
   *                         given input file.
   *
   * Returns true on success, false on failure.
   */

  bool _createAccumulation(const Mdvx &input_file,
			   Mdvx &accum_file) const;
  

  /*********************************************************************
   * _initBackgroundFields() - Initialize the background fields.
   *
   * Returns true on success, false on failure.
   */

  bool _initBackgroundFields(void);
  

  /*********************************************************************
   * _initInputProj() - Initialize the input projection.
   *
   * Returns true on success, false on failure.
   */

  bool _initInputProj(void);
  

  /*********************************************************************
   * _initQualityControl() - Initialize the quality control objects
   *
   * Returns true on success, false on failure.
   */

  bool _initQualityControl(void);
  

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
   * _readAccumFile() - Read the indicated accumulation file.
   *
   * Returns true on success, false on failure.
   */

  bool _readAccumFile(Mdvx &accum_file,
		      const DateTime &trigger_time) const;
  

  /*********************************************************************
   * _readBackgroundField() - Read the indicated background field.
   *
   * Returns a pointer to the read field on success, 0 on failure.
   */

  MdvxField *_readBackgroundField(const DateTime &data_end_time,
				  const DateTime &data_start_time,
				  const BackgroundField &background_field_info) const;
  
  MdvxField *_readBackgroundField(const DateTime &data_end_time,
				  const int search_margin,
				  const BackgroundField &background_field_info) const;
  

  /*********************************************************************
   * _readInputFile() - Read the indicated input file.
   *
   * Returns true on success, false on failure.
   */

  bool _readInputFile(Mdvx &input_file,
		      const DateTime &trigger_time) const;
  

  /*********************************************************************
   * _updateAccumBackground() - Update the given accumulation file with
   *                            any new background data.
   *
   * Returns true on success, false on failure.
   */

  bool _updateAccumBackground(const DateTime &data_time,
			      Mdvx &accum_file) const;
  

  /*********************************************************************
   * _updateAccumFieldBackground() - Update the given accumulation field
   *                                 with the specified background field.
   *                                 If the background field is updated,
   *                                 the background field time is sent back
   *                                 in the bkgnd_time parameter.  If it is
   *                                 not updated, DateTime::NEVER is returned.
   *
   * Returns true on success, false on failure.
   */

  bool _updateAccumFieldBackground(const DateTime &data_time,
				   const DateTime &accum_time,
				   MdvxField &accum_field,
				   const BackgroundField &bkgnd_field_info,
				   DateTime &bkgnd_time) const;
  

  /*********************************************************************
   * _updateAccumTimeField() - Update the accumulation time field with
   *                           the new input time.  The time values will
   *                           only be updated where the input field has
   *                           valid data.
   *
   * Returns true on success, false on failure.
   */

  bool _updateAccumTimeField(MdvxField &time_field,
			     const MdvxField &input_field,
			     const DateTime &data_time) const;

  bool _updateAccumTimeField(MdvxField &time_field,
			     const DateTime &data_time) const;
  

  /*********************************************************************
   * _updateAccumulation() - Update the given accumulation file based on the
   *                         given input file.
   *
   * Returns true on success, false on failure.
   */

  bool _updateAccumulation(const Mdvx &input_file,
			   Mdvx &accum_file) const;
  

  /*********************************************************************
   * __updateAccumMasterHeader() - Update the accumulation file master
   *                               header based on the information in
   *                               the given master header.
   */

  void _updateAccumMasterHeader(Mdvx &accum_file,
				const Mdvx::master_header_t &master_hdr,
				const string &input_file_path) const;
  

};


#endif
