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
 *   $Date: 2016/03/07 01:23:02 $
 *   $Id: LtgStrike.hh,v 1.2 2016/03/07 01:23:02 dixon Exp $
 *   $Revision: 1.2 $
 *   $State: Exp $
 */
 
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/

/************************************************************************
 * LtgStrike: Class representing a lightning strike.
 *
 * RAP, NCAR, Boulder CO
 *
 * June 2006
 *
 * Nancy Rehak
 *
 ************************************************************************/

#ifndef LtgStrike_HH
#define LtgStrike_HH

#include <toolsa/DateTime.hh>

using namespace std;


class LtgStrike
{
 public:

  //////////////////
  // Public types //
  //////////////////

  typedef enum
  {
    CLOUD_DISCHARGE,
    CLOUD_TO_GROUND_DISCHARGE
  } discharge_type_t;

  
  ////////////////////
  // Public methods //
  ////////////////////

  //////////////////////////////
  // Constructors/Destructors //
  //////////////////////////////

  /*********************************************************************
   * Constructors
   */

  LtgStrike(const DateTime &strike_time = DateTime::NEVER,
	    const int strike_nanoseconds = 0,
	    const double lat = 0.0, const double lon = 0.0,
	    const double amplitude = 0.0,
	    const double altitude = 0.0,
	    const int multiplicity = 0,
	    const int num_sensors = 0,
	    const int deg_freedom = 0,
	    const double ellipse_angle = 0.0,
	    const double ellipse_major_axis = 0.0,
	    const double ellipse_minor_axis = 0.0,
	    const double chi_square_value = 0.0,
	    const double rise_time = 0.0,
	    const double peak_to_zero_time = 0.0,
	    const double max_rate_of_rise = 0.0,
	    const discharge_type_t discharge_type = CLOUD_DISCHARGE,
	    const int angle_indicator = 0,
	    const int signal_indicator = 0,
	    const int timing_indicator = 0);
  

  /*********************************************************************
   * Destructor
   */

  virtual ~LtgStrike(void);
  

  ////////////////////
  // Access methods //
  ////////////////////

  void setAmplitude(const double amplitude)
  {
    _amplitude = amplitude;
  }
  
  
  void setDischargeType(const discharge_type_t discharge_type)
  {
    _dischargeType = discharge_type;
  }
  
  
  void setLocation(const double latitude, const double longitude,
		   const double altitude = -1.0)
  {
    _latitude = latitude;
    _longitude = longitude;
    _altitude = altitude;
  }
  
  
  void setMultiplicity(const int multiplicity)
  {
    _multiplicity = multiplicity;
  }
  
  
  DateTime getTime() const
  {
    return _strikeTime;
  }
  
  
  void setTime(const DateTime &strike_time, const int nanoseconds = 0)
  {
    _strikeTime = strike_time;
    _nanoseconds = nanoseconds;
  }
  
  
  //////////////////////////
  // Input/Output methods //
  //////////////////////////

  /*********************************************************************
   * print() - Print the strike information to the indicated stream
   */

  virtual void print(ostream &stream) const;
  

  /*********************************************************************
   * writeUalf() - Write the strike in Universal ASCII Lightning Format
   *               to the given file.
   */

  void writeUalf(FILE *stream) const;
  

 private:

  /////////////////////
  // Private members //
  /////////////////////

  DateTime _strikeTime;
  int _nanoseconds;
  
  double _latitude;
  double _longitude;
  double _altitude;    // in m

  double _amplitude;   // in kA
  int _multiplicity;
  int _numSensors;
  int _degFreedom;
  double _ellipseAngle;      // clockwise bearing from 0 degN
  double _ellipseMajorAxis;  // in km
  double _ellipseMinorAxis;  // in km
  double _chiSquareValue;
  double _riseTime;          // in microseconds
  double _peakToZeroTime;    // in microseconds
  double _maxRateOfRise;     // in kA/usec
  
  discharge_type_t _dischargeType;
  
  int _angleIndicator;       // Sensor angle data used to compute position?
                             //    1 = yes, 0 = no
  int _signalIndicator;      // Sensor signal data used to compute position?
                             //    1 = yes, 0 = no
  int _timingIndicator;      // Sensor timing data used to compute position?
                             //    1 = yes, 0 = no
  
  
  /////////////////////
  // Private methods //
  /////////////////////

};


#endif
