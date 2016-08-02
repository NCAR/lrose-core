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
 *   $Date: 2016/03/07 01:50:09 $
 *   $Id: LiftedIndexInterpField.hh,v 1.4 2016/03/07 01:50:09 dixon Exp $
 *   $Revision: 1.4 $
 *   $State: Exp $
 */
 
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/

/************************************************************************
 * LiftedIndexInterpField: Class for controlling the creation of the lifted
 *                         index interpolated field.
 *
 * RAP, NCAR, Boulder CO
 *
 * September 2007
 *
 * Nancy Rehak
 *
 ************************************************************************/

#ifndef LiftedIndexInterpField_H
#define LiftedIndexInterpField_H

#include <iostream>
#include <string>

#include "DataMgr.hh"
#include "StnInterpField.hh"

using namespace std;


class LiftedIndexInterpField : public StnInterpField
{
  
public:

  ////////////////////
  // Public methods //
  ////////////////////

  /*********************************************************************
   * Constructors
   *
   * Note that this object takes control of the Interpolater pointer and
   * deletes the pointer in the destructor.
   *
   * The DataMgr pointer, however, is used but not owned by this object.
   * The calling method should delete the DataMgr object after deleting
   * this object.
   */

  LiftedIndexInterpField(Interpolater *interpolater,
			 DataMgr *data_mgr,
			 const double sndg_max_dist_km,
			 const double lifted_index_press,
			 const bool output_field_flag = true,
			 const bool debug_flag = false,
			 const bool tryOtherPressure = false,
			 const bool adjustStationPressure = false);
  

  /*********************************************************************
   * Destructor
   */

  virtual ~LiftedIndexInterpField();


protected:
  
  /////////////////////////
  // Protected constants //
  /////////////////////////

  static const string FIELD_NAME;
  static const string UNITS;
  

  ///////////////////////
  // Protected members //
  ///////////////////////

  DataMgr *_dataMgr;
  
  double _soundingMaxDistKm;
  double _liftedIndexPressure;
  bool _tryOtherPressure;
  bool _adjustStationPressure;
  bool _debug;

  ///////////////////////
  // Protected methods //
  ///////////////////////

  /*********************************************************************
   * _calcValue() - Calculates the appropriate value for this field.
   *
   * Returns the calculated value on success, STATION_NAN on failure.
   */

  virtual float _calcValue(const station_report_t &report);



};

#endif
