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

// RCS info
//   $Author: dixon $
//   $Locker:  $
//   $Date: 2016/03/07 01:47:03 $
//   $Id: BeamFilter.hh,v 1.6 2016/03/07 01:47:03 dixon Exp $
//   $Revision: 1.6 $
//   $State: Exp $
//
 
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/
/*********************************************************************
 * BeamFilter: Class for filtering the beam data in a DsRadarMsg.
 *
 * RAP, NCAR, Boulder CO
 *
 * July 2003
 *
 * Nancy Rehak
 *
 *********************************************************************/

#ifndef BeamFilter_hh
#define BeamFilter_hh

#include <string>
#include <vector>

#include <rapformats/DsFieldParams.hh>
#include <rapformats/DsRadarBeam.hh>
#include <rapformats/DsRadarParams.hh>


class BeamFilter
{

public:

  ////////////////////////////////
  // Constructors & Destructors //
  ////////////////////////////////

  /*********************************************************************
   * Constructor
   */

  BeamFilter(const bool debug = false);


  /*********************************************************************
   * Destructor
   */

  virtual ~BeamFilter();


  /*********************************************************************
   * init() - Initialize the beam filter.
   */

  bool init(const bool debug = false) ;


  /*********************************************************************
   * addFilterField() - Add a field to use as a filter.
   */

  void addFilterField(const string &filter_field_name,
		      const double filter_value,
		      const bool keep_data_above_filter_value);
  

  /////////////////////
  // Utility methods //
  /////////////////////

  /*********************************************************************
   * filterBeam() - Filter the data in the beam contained in the given
   *                radar message.  This method assumes that there is
   *                beam data in the given message and doesn't check
   *                for this.
   *
   * Returns true on success, false on failure.
   */

  bool filterBeam(DsRadarBeam &radar_beam) const;


  /*********************************************************************
   * updateFieldInfo() - Update the field information for the filter.
   *                     This method assumes that there is field information
   *                     in the given message and doesn't check for this.
   *                     This method should be called every time a message
   *                     is received that contains field parameters.
   *
   * Returns true on success, false on failure.
   */

  bool updateFieldInfo(const vector< DsFieldParams* > input_fields);


  /*********************************************************************
   * updateParamInfo() - Update the radar parameters for the filter.
   *                     This method assumes that there is param information
   *                     in the given message and doesn't check for this.
   *                     This method should be called every time a message
   *                     is received that contains radar parameters.
   *
   * Returns true on success, false on failure.
   */

  bool updateRadarInfo(const DsRadarParams &radar_params);


protected:

  /////////////////////
  // Protected types //
  /////////////////////

  typedef struct
  {
    string field_name;
    int field_index;
    double filter_value;
    bool keep_data_above_filter_value;
  } filter_field_t;
  
  ///////////////////////
  // Protected members //
  ///////////////////////

  bool _debug;

  vector< filter_field_t > _filterFields;
  
  unsigned int _numGates;
  unsigned int _numFields;

  bool _fieldsInitialized;

  vector< DsFieldParams* > _inputFields;


  ///////////////////////
  // Protected methods //
  ///////////////////////

  /*********************************************************************
   * _clearInputFields() - Clear out the current input field information.
   */

  void _clearInputFields();

};

#endif

   
