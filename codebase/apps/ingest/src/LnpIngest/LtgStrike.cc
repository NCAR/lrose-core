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
//   $Date: 2016/03/07 01:23:02 $
//   $Id: LtgStrike.cc,v 1.2 2016/03/07 01:23:02 dixon Exp $
//   $Revision: 1.2 $
//   $State: Exp $
//
 
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/
/*********************************************************************
 * LtgStrike: Class representing a lightning strike.
 *
 * RAP, NCAR, Boulder CO
 *
 * June 2006
 *
 * Nancy Rehak
 *
 *********************************************************************/

#include <iostream>
#include <string>

#include "LtgStrike.hh"

using namespace std;

/*********************************************************************
 * Constructor
 */

LtgStrike::LtgStrike(const DateTime &strike_time,
		     const int nanoseconds,
		     const double lat, const double lon,
		     const double amplitude,
		     const double altitude,
		     const int multiplicity,
		     const int num_sensors,
		     const int deg_freedom,
		     const double ellipse_angle,
		     const double ellipse_major_axis,
		     const double ellipse_minor_axis,
		     const double chi_square_value,
		     const double rise_time,
		     const double peak_to_zero_time,
		     const double max_rate_of_rise,
		     const discharge_type_t discharge_type,
		     const int angle_indicator,
		     const int signal_indicator,
		     const int timing_indicator) :
  _strikeTime(strike_time),
  _nanoseconds(nanoseconds),
  _latitude(lat),
  _longitude(lon),
  _altitude(altitude),
  _amplitude(amplitude),
  _multiplicity(multiplicity),
  _numSensors(num_sensors),
  _degFreedom(deg_freedom),
  _ellipseAngle(ellipse_angle),
  _ellipseMajorAxis(ellipse_major_axis),
  _ellipseMinorAxis(ellipse_minor_axis),
  _chiSquareValue(chi_square_value),
  _riseTime(rise_time),
  _peakToZeroTime(peak_to_zero_time),
  _maxRateOfRise(max_rate_of_rise),
  _dischargeType(discharge_type),
  _angleIndicator(angle_indicator),
  _signalIndicator(signal_indicator),
  _timingIndicator(timing_indicator)
{
}


/*********************************************************************
 * Destructor
 */

LtgStrike::~LtgStrike()
{
}


/*********************************************************************
 * print() - Print the strike information to the indicated stream
 */

void LtgStrike::print(ostream &stream) const
{
  stream << "Strike info:" << endl;
  stream << "   strike time: " << _strikeTime << endl;
  stream << "   nanoseconds: " << _nanoseconds << endl;
  stream << "   latitude: " << _latitude << endl;
  stream << "   longitude: " << _longitude << endl;
  stream << "   amplitude: " << _amplitude << endl;
  stream << "   discharge type: ";
  switch (_dischargeType)
  {
  case CLOUD_DISCHARGE :
    stream << "CLOUD_DISCHARGE";
    break;
  case CLOUD_TO_GROUND_DISCHARGE :
    stream << "CLOUD_TO_GROUND_DISCHARGE";
    break;
  }
  cerr << endl;
  stream << "   multiplicity: " << _multiplicity << endl;
}


/*********************************************************************
 * writeUalf() - Write the strike in Universal ASCII Lightning Format
 *               to the given file.
 */

void LtgStrike::writeUalf(FILE *stream) const
{
  fprintf(stream, "0");
  fprintf(stream, "\t%d\t%d\t%d\t%d\t%d\t%d\t%d",
	  _strikeTime.getYear(), _strikeTime.getMonth(), _strikeTime.getDay(),
	  _strikeTime.getHour(), _strikeTime.getMin(), _strikeTime.getSec(),
	  _nanoseconds);
  fprintf(stream, "\t%.4f\t%.4f\t%d",
	  _latitude, _longitude, (int)_amplitude);
  fprintf(stream, "\t%d\t%d\t%d",
	  _multiplicity, _numSensors, _degFreedom);
  fprintf(stream,
	  "\t%.1f\t%.1f\t%.1f",
	  _ellipseAngle, _ellipseMajorAxis, _ellipseMinorAxis);
  fprintf(stream, "\t%.2f\t%.1f\t%.1f\t%.1f\t",
	  _chiSquareValue,
	  _riseTime, _peakToZeroTime, _maxRateOfRise);
  switch (_dischargeType)
  {
  case CLOUD_DISCHARGE :
    fprintf(stream, "1");
    break;
  case CLOUD_TO_GROUND_DISCHARGE :
    fprintf(stream, "0");
    break;
  }
  fprintf(stream, "\t%d\t%d\t%d\n",
	  _angleIndicator, _signalIndicator, _timingIndicator);
}

/**********************************************************************
 *              Private Member Functions                              *
 **********************************************************************/
