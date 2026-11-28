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
 *   $Author: cunning $
 *   $Locker:  $
 *   $Date: 2016/11/28 16:54:31 $
 *   $Id: FieldProcessor.hh,v 1.7 2016/11/28 16:54:31 cunning Exp $
 *   $Revision: 1.7 $
 *   $State: Exp $
 */
 
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/

/************************************************************************
 * FieldProcessor : Base class for objects that convert the data in
 *                  a TeraScan field into an Mdv field.
 *
 * RAP, NCAR, Boulder CO
 *
 * October 2001
 *
 * Nancy Rehak
 *
 ************************************************************************/

#ifndef FieldProcessor_HH
#define FieldProcessor_HH


#include <string>

// Include the needed Terascan include files.  Note that they were
// written assuming that people would only write in C, not in C++.

#ifdef __cplusplus
extern "C" {
#endif

#include <terrno.h>
#include <gp.h>
#include <uif.h>
#include <etx.h>
#include <cdfnames.h>

#ifdef __cplusplus
}
#endif


#include "Params.hh"
#include <dataport/port_types.h>
#include <Mdv/MdvxField.hh>

using namespace std;

class FieldProcessor
{
 public:

  ////////////////////
  // Public members //
  ////////////////////

  ////////////////////
  // Public methods //
  ////////////////////

  //////////////////////////////
  // Constructors/Destructors //
  //////////////////////////////

  /*********************************************************************
   * Constructor
   */

  FieldProcessor(SETP input_dataset);
  

  /*********************************************************************
   * Destructor
   */

  virtual ~FieldProcessor(void);
  

  /*********************************************************************
   * createField() - Create the MDV field from the TeraScan satellite data.
   */

  MdvxField *createField(const string &sat_field_name,
			 const string &mdv_field_name,
			 const string &field_units,
			 const int field_code,
			 const Params::scaling_type_t scaling_type,
			 const double scale,
			 const double bias);
  

  ////////////////////
  // Access Methods //
  ////////////////////

  /*********************************************************************
   * getNumSamples() - Retrieve the number of samples in the field.
   */

  int getNumSamples() const
  {
    return _numSamples;
  }
  

  /*********************************************************************
   * getNumLines() - Retrieve the number of lines in the field.
   */

  int getNumLines() const
  {
    return _numLines;
  }
  

  /*********************************************************************
   * getCenterLat() - Get the latitude of the center point of the field
   */

  double getCenterLat() const
  {
    return _centerLat;
  }
  

  /*********************************************************************
   * getCenterLon() - Get the longitude of the center point of the field
   */

  double getCenterLon() const
  {
    return _centerLon;
  }
  

protected:

  // The satellite input dataset

  SETP _inputDataset;
  
  // Flag indicating whether the object has been initialized

  bool _initialized;
  
  // The satellite field dimensions

  int _numSamples;
  int _numLines;

  long _projection;

  double _projParam;
  
  double _centerLat;
  double _centerLon;
  
  double _lowerLeftLat;
  double _lowerLeftLon;
  

  /*********************************************************************
   * _init() - Initialize the internal variables.
   */

  bool _init();
  

  //////////////////////////
  // Pure virtual methods //
  //////////////////////////

  /*********************************************************************
   * _calcIndex() - Calculate the MDV field index for the given satellite
   *                field location.
   */

  virtual int _calcIndex(const int x_index, const int y_index) = 0;


  /*********************************************************************
   * _initLocal() - Do any initialization needed locally by the derived
   *                class.  This method is called at the end of the
   *                FieldProcessor::_init() method.
   */

  virtual bool _initLocal(ETXFORM mxfm) = 0;


  /*********************************************************************
   * _setProjectionInfo() - Set the projection information in the given
   *                        field header.
   */

  virtual void _setProjectionInfo(Mdvx::field_header_t &fld_hdr) = 0;

};


#endif
