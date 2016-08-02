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
 *   $Date: 2016/03/07 18:17:27 $
 *   $Id: RefrMerge.hh,v 1.3 2016/03/07 18:17:27 dixon Exp $
 *   $Revision: 1.3 $
 *   $State: Exp $
 */
 
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/

/************************************************************************
 * RefrMerge: RefrMerge program object.
 *
 * RAP, NCAR, Boulder CO
 *
 * June 2006
 *
 * Nancy Rehak
 *
 ************************************************************************/

#ifndef RefrMerge_HH
#define RefrMerge_HH


#include <string>
#include <sys/time.h>

#include <dsdata/DsTrigger.hh>
#include <Mdv/DsMdvx.hh>
#include <Mdv/MdvxField.hh>
#include <Mdv/MdvxPjg.hh>

#include "Args.hh"
#include "Params.hh"

using namespace std;


class RefrMerge
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

  ~RefrMerge(void);
  

  /*********************************************************************
   * Inst() - Retrieve the singleton instance of this class.
   */

  static RefrMerge *Inst(int argc, char **argv);
  static RefrMerge *Inst();
  

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

  static RefrMerge *_instance;
  
  // Program parameters.

  char *_progName;
  Args *_args;
  Params *_params;
  
  // Triggering object

  DsTrigger *_dataTrigger;
  
  // Output information

  MdvxPjg _projection;
  

  /////////////////////
  // Private methods //
  /////////////////////

  /*********************************************************************
   * Constructor -- private because this is a singleton object
   */

  RefrMerge(int argc, char **argv);
  

  /*********************************************************************
   * _addMergeField() - Add the given field information to the merge field.
   */

  void _addMergeField(fl32 *mean_n, fl32 *weight,
		      const MdvxField &n_field,
		      const MdvxField &sigma_n_field) const;
  

  /*********************************************************************
   * _createMergeField() - Create the MdvxField object with the given merge
   *                       data.
   *
   * Returns a pointer to the new field on success, 0 on failure.
   */

  MdvxField *_createMergeField(const fl32 *mean_n_data,
			       const DateTime &data_time) const;
  

  /*********************************************************************
   * _initProjection() - Initialize the projection.
   *
   * Returns true on success, false on failure.
   */

  bool _initProjection(void);
  

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
   * _readFields() - Read in the indicated merge field
   *
   * Returns a pointer to the merge field information if successful, 0 otherwise.
   * The N field will be field 0; the Sigma N field will be field 1.
   */

  Mdvx *_readFields(const DateTime &data_time,
		    const string &url,
		    const string &n_field_name,
		    const string &sigma_n_field_name) const;
  

  /*********************************************************************
   * _updateOutputMasterHeader() - Update the master header values for
   *                               the output file.
   */

  void _updateOutputMasterHeader(DsMdvx &output_file,
				 const DateTime &data_time) const;
  

};


#endif
