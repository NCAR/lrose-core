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
 *   $Date: 2016/03/07 18:28:25 $
 *   $Id: ShapeGridPt2Mdv.hh,v 1.3 2016/03/07 18:28:25 dixon Exp $
 *   $Revision: 1.3 $
 *   $State: Exp $
 */
 
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/

/************************************************************************
 * ShapeGridPt2Mdv : ShapeGridPt2Mdv program class.
 *
 * RAP, NCAR, Boulder CO
 *
 * Sept 2006
 *
 * Nancy Rehak
 *
 ************************************************************************/

#ifndef ShapeGridPt2Mdv_HH
#define ShapeGridPt2Mdv_HH

#include <Mdv/MdvxField.hh>
#include <Mdv/MdvxPjg.hh>
#include <toolsa/str.h>

#include "Args.hh"
#include "Params.hh"

using namespace std;

class ShapeGridPt2Mdv
{
 public:

  // Destructor

  ~ShapeGridPt2Mdv(void);
  
  // Get ShapeGridPt2Mdv singleton instance

  static ShapeGridPt2Mdv *Inst(int argc, char **argv);
  static ShapeGridPt2Mdv *Inst();
  
  // Initialize the program.  Must be called before run().

  bool init();
  
  // Run the program.

  void run();
  
  // Flag indicating whether the program status is currently okay.

  bool okay;
  
 private:

  // Singleton instance pointer

  static ShapeGridPt2Mdv *_instance;
  
  // Program parameters.

  char *_progName;
  Args *_args;
  Params *_params;
  
  char *_inputLine;
  
  MdvxPjg _outputProj;
  

  /////////////////////
  // Private methods //
  /////////////////////

  // Constructor -- private because this is a singleton object

  ShapeGridPt2Mdv(int argc, char **argv);
  
  /*********************************************************************
   * _addData() - Add the data from this line to the given field data.
   */

  void _addData(fl32 *field_data,
		const char *input_line) const;
  

  /*********************************************************************
   * _createField() - Create the output field.  Initialize the field with
   *                  missing data values which will be overwritten with
   *                  data values as they are processed.
   *
   * Returns a pointer to the created field on success, 0 on failure.
   */

  MdvxField *_createField(const DateTime &file_time) const;
  

  /*********************************************************************
   * _setMasterHeader() - Set the master header in the output file.
   */

  void _setMasterHeader(Mdvx &mdvx,
			const DateTime &file_time) const;
  

  /*********************************************************************
   * _updateField() - Update the given field with the data from the
   *                  input file.
   *
   * Returns true on success, false on failure.
   */

  bool _updateField(MdvxField &field) const;
  

};


#endif
