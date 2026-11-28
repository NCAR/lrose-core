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
 * GenPtWriter: Class that writes obs information to a GenPt
 *              SPDB database.
 *
 * RAP, NCAR, Boulder CO
 *
 * February 2006
 *
 * Kay Levesque
 *
 ************************************************************************/

#ifndef GenPtWriter_H
#define GenPtWriter_H

#include <rapformats/GenPt.hh>
#include "quikSCATObs.hh"
#include "SpdbWriter.hh"
#include <cstdio>

using namespace std;


class GenPtWriter : public SpdbWriter
{
  
public:

  ////////////////////
  // Public methods //
  ////////////////////

  /*********************************************************************
   * Constructors
   */

  GenPtWriter(const string &output_url,
	     const int expire_secs,
	     const bool debug_flag);
  

  /*********************************************************************
   * Destructor
   */

  virtual ~GenPtWriter();

  /*********************************************************************
   * addInfo() - Add observation information
   *
   * Returns true on success, false otherwise.
   */

  bool addInfo(const quikSCATObs &obs);

protected:

  static const string WIND_SPEED_FIELD_NAME;
  static const string WIND_SPEED_UNITS_NAME;

  static const string WIND_DIRECTION_FIELD_NAME;
  static const string WIND_DIRECTION_UNITS_NAME;

  static const string RAIN_FLAG_FIELD_NAME;
  static const string RAIN_FLAG_UNITS_NAME;

  static const string NSOL_FLAG_FIELD_NAME;
  static const string NSOL_FLAG_UNITS_NAME;

};

#endif
