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
 *   $Date: 2016/03/04 01:28:14 $
 *   $Id: TitanVectors2Mdv.hh,v 1.4 2016/03/04 01:28:14 dixon Exp $
 *   $Revision: 1.4 $
 *   $State: Exp $
 */
 
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/

/************************************************************************
 * TitanVectors2Mdv: TitanVectors2Mdv program object.
 *
 * RAP, NCAR, Boulder CO
 *
 * December 2003
 *
 * Nancy Rehak
 *
 ************************************************************************/

#ifndef TitanVectors2Mdv_HH
#define TitanVectors2Mdv_HH


#include <string>

#include <dsdata/DsTrigger.hh>
#include <dsdata/Tstorm.hh>
#include <dsdata/TstormGroup.hh>
#include <dsdata/TstormMgr.hh>
#include <Mdv/DsMdvx.hh>
#include <Mdv/MdvxField.hh>
#include <Mdv/MdvxPjg.hh>

#include "Args.hh"
#include "Params.hh"

using namespace std;


class TitanVectors2Mdv
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

  ~TitanVectors2Mdv(void);
  

  /*********************************************************************
   * Inst() - Retrieve the singleton instance of this class.
   */

  static TitanVectors2Mdv *Inst(int argc, char **argv);
  static TitanVectors2Mdv *Inst();
  

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

  static const float MISSING_DATA_VALUE;
  

  /////////////////////
  // Private members //
  /////////////////////

  // Singleton instance pointer

  static TitanVectors2Mdv *_instance;
  
  // Program parameters.

  char *_progName;
  Args *_args;
  Params *_params;
  
  // Triggering object

  DsTrigger *_dataTrigger;
  
  MdvxPjg _gridProjection;
  
  int _gridSize;
  
  fl32 *_uGrid;
  fl32 *_vGrid;
  
  // Intermediate grids -- used only in _calcGriddedStorms() but kept
  // in the class so that the memory isn't constantly being reallocated.

  float *_uSum;
  float *_vSum;
  int *_numVectors;
  
  unsigned char *_stormGrid;
  
  // Remapping look-up table.  Keep globally so it only has to be calculated
  // when the projection of the incoming MDV vectors changes, which shouldn't
  // be very often.

  MdvxRemapLut _remapLut;

  
  /////////////////////
  // Private methods //
  /////////////////////

  /*********************************************************************
   * Constructor -- private because this is a singleton object
   */

  TitanVectors2Mdv(int argc, char **argv);
  

  /*********************************************************************
   * _calcGriddedStorms() - Calculate the gridded storms for the given
   *                        data time.  Updates _uSum, _vSum and _numVectors
   *                        with the gridded information for this set of
   *                        storms.
   *
   * Returns true on success, false on failure.
   */

  bool _calcGriddedStorms(const DateTime &data_time);
  

  /*********************************************************************
   * _createField() - Create a blank field for the output file.
   *
   * Returns a pointer to the newly created field on success, 0 on
   * failure.  The calling method is responsible for deleting the
   * returned pointer.
   */

  MdvxField *_createField(const DateTime &data_time,
			  const string &field_name_long,
			  const string &field_name,
			  const fl32 *data) const;
  

  /*********************************************************************
   * _processData() - Process data for the given time.
   *
   * Returns true on success, false on failure.
   */

  bool _processData(const DateTime &data_time);
  

  /*********************************************************************
   * _updateMasterHeader() - Update the values in the output file master
   *                         header.
   */

  void _updateMasterHeader(DsMdvx &output_file,
			   const DateTime &data_time) const;
  

  /*********************************************************************
   * _updateWithMdvVectors() - Update the vectors in _uGrid and _vGrid
   *                           with the corresponding MDV vectors.  We
   *                           update with MDV vectors by simply replacing
   *                           missing vectors with the corresponding
   *                           MDV vectors.
   */

  void _updateWithMdvVectors(const DateTime &data_time);
  

  /*********************************************************************
   * _writeOutputGrid() - Write the _uGrid and _vGrid data to the output
   *                      file for the given time.
   *
   * Returns true on success, false on failure.
   */

  bool _writeOutputGrid(const DateTime &data_time);
  

};


#endif
