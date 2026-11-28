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

/************************************************************************
 * MdvWriter: Class that writes obs information to an MDV file.
 *
 * RAP, NCAR, Boulder CO
 *
 * April 2006
 *
 * Kay Levesque
 *
 ************************************************************************/

#ifndef MdvWriter_H
#define MdvWriter_H

#include <string>

#include <euclid/Pjg.hh>
#include <Mdv/DsMdvx.hh>
#include <Mdv/MdvxField.hh>
#include <toolsa/DateTime.hh>
#include "Writer.hh"
#include "quikSCATObs.hh"

#include <cstdio>

using namespace std;


class MdvWriter : public Writer
{
  
public:

  static const fl32 MISSING_DATA_VALUE;

  ////////////////////
  // Public methods //
  ////////////////////

  /*********************************************************************
   * Constructors
   */

  MdvWriter(const string &output_url,
	    const int expire_secs,
	    const MdvxPjg &projection,
	    const bool debug_flag);
  

  /*********************************************************************
   * Destructor
   */

  virtual ~MdvWriter();


  /*********************************************************************
   * init() - Initialize the local data.
   *
   * Returns true if the initialization was successful, false otherwise.
   */

  virtual bool init();


  /*********************************************************************
   * addInfo() - Initialize the local data.
   *
   * Returns true if the initialization was successful, false otherwise.
   */

  bool addInfo(const quikSCATObs &obs);


  /*********************************************************************
   * writeInfo() - Write the observation information.
   *
   */

  bool writeInfo();


protected:

  static const string WIND_SPEED_FIELD_NAME;
  static const string WIND_SPEED_UNITS_NAME;

  static const string WIND_DIRECTION_FIELD_NAME;
  static const string WIND_DIRECTION_UNITS_NAME;

  static const string U_FIELD_NAME;
  static const string U_UNITS_NAME;

  static const string V_FIELD_NAME;
  static const string V_UNITS_NAME;

  static const string RAIN_FLAG_FIELD_NAME;
  static const string RAIN_FLAG_UNITS_NAME;

  static const string NSOL_FLAG_FIELD_NAME;
  static const string NSOL_FLAG_UNITS_NAME;
  bool _remap;
  MdvxField *_windSpeedField;
  MdvxField *_windDirectionField;
  MdvxField *_uField;
  MdvxField *_vField;
  MdvxField *_rainFlagField;
  MdvxField *_nsolFlagField;
  MdvxPjg _projection;
  DateTime _startTime, _endTime;
  fl32 *_windSpeedDataPtr;
  fl32 *_windDirDataPtr;
  fl32 *_uDataPtr;
  fl32 *_vDataPtr;
  fl32 *_rainFlagDataPtr;
  fl32 *_nsolFlagDataPtr;

  /*********************************************************************
   * _createMdvField() - Create a blank MDV field from the given information.
   *
   * Returns a pointer to the new field on success, 0 on failure.
   */

  MdvxField *_createMdvField(const string &field_name,
                             const string &field_name_long,
                             const string &units) const;


  /*********************************************************************
   * _initializeMasterHeader() - Initialize the master header in the output
   *                             MDV file.
   */

  void _initializeMasterHeader(DsMdvx &output_mdv) const;


  /*********************************************************************
   * _createFields() - 
   *
   * Returns true if the fields were successfully created. False otherwise.
   */

  bool _createFields();

};

#endif
