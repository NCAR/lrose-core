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
 *   $Date: 2016/03/07 01:51:41 $
 *   $Id: CombineLtgRadar.hh,v 1.9 2016/03/07 01:51:41 dixon Exp $
 *   $Revision: 1.9 $
 *   $State: Exp $
 */
 
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/

/************************************************************************
 * CombineLtgRadar.hh : header file for the CombineLtgRadar program class.
 *
 * RAP, NCAR, Boulder CO
 *
 * April 1999
 *
 * Nancy Rehak
 *
 ************************************************************************/

#ifndef CombineLtgRadar_HH
#define CombineLtgRadar_HH

/*
 **************************** includes **********************************
 */

#include <sys/time.h>

#include <dsdata/DsTrigger.hh>
#include <Mdv/DsMdvx.hh>
#include <Mdv/MdvxField.hh>
#include <Mdv/MdvxProj.hh>

#include "Args.hh"
#include "Params.hh"

/*
 ******************************* defines ********************************
 */


/*
 ******************************* structures *****************************
 */

/*
 ************************* global variables *****************************
 */

/*
 ***************************** function prototypes **********************
 */

/*
 ************************* class definitions ****************************
 */

class CombineLtgRadar
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

  ~CombineLtgRadar(void);
  

  /*********************************************************************
   * Inst() - Retrieve the singleton instance of this class.
   */

  static CombineLtgRadar *Inst(int argc, char **argv);
  static CombineLtgRadar *Inst();
  

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

  static CombineLtgRadar *_instance;
  
  // Program parameters.

  char *_progName;
  Args *_args;
  Params *_params;
  
  // Processing trigger

  DsTrigger *_dataTrigger;
  

  /////////////////////
  // Private methods //
  /////////////////////

  /*********************************************************************
   * Constructor -- private because this is a singleton object
   */

  CombineLtgRadar(int argc, char **argv);
  
  /*********************************************************************
   * _combineFields() - Process the matching radar data field and lightning
   *                    data field.
   */

  MdvxField *_combineFields(const time_t data_time,
			    const MdvxField &radar_field,
			    const MdvxField &ltg_field) const;
  

  /*********************************************************************
   * _combineFromRadarOnly() - Create 'combined' field from radar
   *                           data only because ltg is missing
   */
  
  MdvxField *_combineFromRadarOnly(const time_t data_time,
                                   const MdvxField &radar_field) const;
  
  /*********************************************************************
   * _createCombinedField() - Create a blank for for storing the combined
   *                          data.
   *
   * Returns a pointer to the created field on success, 0 on failure.
   */

  MdvxField *_createCombinedField(const time_t data_time,
                                  const Mdvx::field_header_t &input_field_hdr,
                                  const Mdvx::vlevel_header_t &input_vlevel_hdr) const;
  

  /***************************************************************************
   * _getLtgDataValue() - Determines the lightning data value to use for
   *                      the given number of strikes.  Returns 0.0 if the
   *                      given number of strikes isn't found in the table.
   */

  double _getLtgDataValue(const int num_strikes) const;
  

  /*********************************************************************
   * _processData() - Process the data for the given time.
   */

  bool _processData(const time_t trigger_time) const;
  

  /*********************************************************************
   * _readField() - Read the indicated field data.
   */

  MdvxField *_readField(const time_t data_time,
			const string &url,
			const string &field_name,
			const int field_num,
			const int max_input_valid_secs,
			Mdvx::master_header_t &master_hdr,
			MdvxProj *remap_proj = 0) const;
  

  /*********************************************************************
   * _updateMasterHeader() - Update the master header for the output file.
   */

  void _updateMasterHeader(DsMdvx &output_file,
			   const time_t start_time,
			   const time_t centroid_time,
			   const time_t end_time,
			   const time_t expire_time,
			   const Mdvx::master_header_t &input_master_hdr) const;
  

};


#endif
