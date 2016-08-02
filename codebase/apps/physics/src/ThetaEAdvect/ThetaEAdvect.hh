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
 *   $Id: ThetaEAdvect.hh,v 1.4 2016/03/06 23:15:37 dixon Exp $
 *   $Revision: 1.4 $
 *   $State: Exp $
 */
 
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/

/************************************************************************
 * ThetaEAdvect: ThetaEAdvect program object.
 *
 * RAP, NCAR, Boulder CO
 *
 * March 2002
 *
 * Nancy Rehak
 *
 ************************************************************************/

#ifndef ThetaEAdvect_HH
#define ThetaEAdvect_HH

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

class ThetaEAdvect
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

  ~ThetaEAdvect(void);
  

  /*********************************************************************
   * Inst() - Retrieve the singleton instance of this class.
   */

  static ThetaEAdvect *Inst(int argc, char **argv);
  static ThetaEAdvect *Inst();
  

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

  static const fl32 THETA_E_MISSING_DATA_VALUE;
  

  /////////////////////
  // Private members //
  /////////////////////

  // Singleton instance pointer

  static ThetaEAdvect *_instance;
  
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

  ThetaEAdvect(int argc, char **argv);
  

  /*********************************************************************
   * _calc3DThetaEField() - Calculate the 3D theta-e field.
   *
   * Returns a pointer to the created field, or 0 if the field could
   * not be created for some reason.
   */

  MdvxField *_calc3DThetaEField(const MdvxField &mixing_ratio_field,
				const MdvxField &temperature_field,
				const MdvxField &pressure_field) const;
  

  /*********************************************************************
   * _calcThetaEAdvectField() - Calculate the theta-e advection field.
   *
   * Returns a pointer to the created field, or 0 if the field could
   * not be created for some reason.
   */

  MdvxField *_calcThetaEAdvectField(const MdvxField &u_field,
				    const MdvxField &v_field,
				    const MdvxField &theta_e_field) const;
  

  /*********************************************************************
   * _calcVertDeriveField() - Calculate the vertical derivative field.
   *
   * Returns a pointer to the created field, or 0 if the field could
   * not be created for some reason.
   */

  MdvxField *_calcVertDeriveField(const MdvxField &theta_e_adv_field) const;
  

  /*********************************************************************
   * _create3DThetaEField() - Create the blank 3D theta-e field.  Upon
   *                          return, the field values will be set to the
   *                          missing data value.
   *
   * Returns a pointer to the created field, or 0 if the field could
   * not be created for some reason.
   */

  MdvxField *_create3DThetaEField(const MdvxField &base_field) const;
  

  /*********************************************************************
   * _createThetaEAdvectField() - Create the blank theta-e advection field.
   *                              Upon return, the field values will be
   *                              set to the missing data value.
   *
   * Returns a pointer to the created field, or 0 if the field could
   * not be created for some reason.
   */

  MdvxField *_createThetaEAdvectField(const MdvxField &base_field) const;
  

  /*********************************************************************
   * _createVertDeriveField() - Create the blank vertical derivative field.
   *                            Upon return, the field values will be
   *                            set to the missing data value.
   *
   * Returns a pointer to the created field, or 0 if the field could
   * not be created for some reason.
   */

  MdvxField *_createVertDeriveField(const MdvxField &base_field) const;
  

  /*********************************************************************
   * _processData() - Process data for the given trigger time.
   *
   * Returns true on success, false on failure.
   */

  bool _processData(const time_t trigger_time, const int lead_time);
  

  /*********************************************************************
   * _readFieldData() - Read the indicated field data.
   */

  MdvxField *_readFieldData(const string &url,
			    const string &field_name,
			    const int field_num,
			    const double lower_level,
			    const double upper_level,
			    const time_t data_time,
			    const int lead_time,
			    const int max_input_valid_secs,
			    Mdvx::master_header_t *master_hdr = 0);
  

  /*********************************************************************
   * _updateOutputMasterHeader() - Update the master header values for
   *                               the output file.
   */

  void _updateOutputMasterHeader(DsMdvx &output_file,
				 const Mdvx::master_header_t &input_master_hdr);
  
};


#endif
