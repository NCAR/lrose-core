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

// RCS info
//   $Author: dixon $
//   $Locker:  $
//   $Date: 2016/03/07 01:50:09 $
//   $Id: LiftedIndexInterpField.cc,v 1.5 2016/03/07 01:50:09 dixon Exp $
//   $Revision: 1.5 $
//   $State: Exp $
//
 
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/
/*********************************************************************
 * LiftedIndexInterpField: Class for controlling the creation of the lifted
 *                         index interpolated field.
 *
 * RAP, NCAR, Boulder CO
 *
 * September 2007
 *
 * Nancy Rehak
 *
 *********************************************************************/

#include <physics/physics.h>

#include "LiftedIndexInterpField.hh"

using namespace std;


const string LiftedIndexInterpField::FIELD_NAME = "LiftedIndex";
const string LiftedIndexInterpField::UNITS = "C";


/*********************************************************************
 * Constructors
 */

LiftedIndexInterpField::LiftedIndexInterpField(Interpolater *interpolater,
					       DataMgr *data_mgr,
					       const double sndg_max_dist_km,
					       const double lifted_index_press,
					       const bool output_field_flag,
					       const bool debug_flag,
					       const bool tryOtherPressure,
					       const bool adjustStationPressure) :
  StnInterpField(FIELD_NAME, UNITS, interpolater,
		 output_field_flag, debug_flag),
  _dataMgr(data_mgr),
  _soundingMaxDistKm(sndg_max_dist_km),
  _liftedIndexPressure(lifted_index_press),
  _tryOtherPressure(tryOtherPressure),
  _adjustStationPressure(adjustStationPressure),
  _debug(debug_flag)

{
}

  
/*********************************************************************
 * Destructor
 */

LiftedIndexInterpField::~LiftedIndexInterpField()
{
}


/**********************************************************************
 *              Protected/Private Member Functions                    *
 **********************************************************************/

/*********************************************************************
 * _calcValue() - Calculates the appropriate value for this field.
 *
 * Returns the calculated value on success, STATION_NAN on failure.
 */

float LiftedIndexInterpField::_calcValue(const station_report_t &report)
{
  // Get the sounding data

  Sndg *sndg_ptr;

  if ((sndg_ptr = _dataMgr->findClosestSounding(report.lat, report.lon,
						report.time,
						_soundingMaxDistKm)) == 0)
    return STATION_NAN;
  
  Sndg::header_t header = sndg_ptr->getHeader();
  int n_points = header.nPoints;
  vector< Sndg::point_t > pts = sndg_ptr->getPoints();

  // Initialize tli value.

  float tli = STATION_NAN;

  // Loop through sounding data points looking for correct pressure level.
  // If we find it, calculate tli.

  for (int i = 0; i < n_points - 1; ++i)
  {
    Sndg::point_t pt = pts[i];

    Sndg::point_t next_pt = pts[i+1];

    // See if we have exactly the pressure we need, either
    // at this point or the next one.

    if (fabs(pt.pressure - _liftedIndexPressure) < 0.1){
      tli = pt.temp;
      break;
    }

    if (fabs(next_pt.pressure - _liftedIndexPressure) < 0.1){
      tli = next_pt.temp;
      break;
    }

    // If we got here we don't have exactly the pressure we nned - have to interpolate.
    // Need both points to have non-missing pressure (greater than 0) for interpolation to go ahead.
    if ((pt.pressure > 0) && (next_pt.pressure > 0)){
      
      if ((pt.pressure >= _liftedIndexPressure &&
	   next_pt.pressure <= _liftedIndexPressure) ||
	  (pt.pressure <= _liftedIndexPressure &&
	   next_pt.pressure >= _liftedIndexPressure))
	{
	  tli = pt.temp + (next_pt.temp - pt.temp) *
	    (pt.pressure - _liftedIndexPressure)/
	    ( pt.pressure - next_pt.pressure);
	  break;
	}
    } // End of if we have non-missing pressure points
  } /* endfor - i */

  // Calculate the lifted index, if we have the appropriate data


  // Get the best pressure we can get
  double pressureToUse = report.shared.pressure_station.stn_pres;
  if ((pressureToUse == STATION_NAN) && (report.pres != STATION_NAN) && (_tryOtherPressure)){
    // I'm not 100% sure aout this so print a warning that the failover is being used - Niles.
    cerr << "WARNING : Station pressure is missing, using pressure from report.pres field of " << report.pres << endl;
    pressureToUse = report.pres;
  }

  if (tli == STATION_NAN ||
      pressureToUse == STATION_NAN ||
      report.temp == STATION_NAN ||
      report.dew_point == STATION_NAN)
    return STATION_NAN;
  

  if (_adjustStationPressure){
    if (_debug) cerr << "Station pressure adjusted from " <<  pressureToUse << " to ";
    double h = report.alt/1000.0;
    pressureToUse *= exp(-.119*h-.0013*h*h);
    if (_debug) cerr << pressureToUse << endl;
  }

  return PHYli(pressureToUse,
	       report.temp, report.dew_point, tli + 273.15,
	       _liftedIndexPressure, STATION_NAN);
}
