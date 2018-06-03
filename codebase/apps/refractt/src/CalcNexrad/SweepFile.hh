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

/* RCS info
 *   $Author: jcraig $
 *   $Locker:  $
 *   $Date: 2018/01/26 20:33:40 $
 *   $Id: SweepFile.hh,v 1.3 2018/01/26 20:33:40 jcraig Exp $
 *   $Revision: 1.3 $
 *   $State: Exp $
 */
 
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/

/************************************************************************
 * SweepFile: Class for controlling netCDF sweep files.
 *
 * RAP, NCAR, Boulder CO
 *
 * May 2005
 *
 * Nancy Rehak
 *
 ************************************************************************/

#ifndef SweepFile_H
#define SweepFile_H

#include <string>
#include <vector>

#include <Ncxx/Nc3File.hh>
#include <toolsa/DateTime.hh>

using namespace std;

class SweepFile
{
  
public:

  static const float MISSING_DATA_VALUE;

  /*********************************************************************
   * Constructors
   */

  SweepFile(const string &output_dir,
	    const double radar_lat,    /* deg */
	    const double radar_lon,    /* deg */
	    const double radar_alt,    /* km */
	    const time_t sweep_start_time,
	    const float elev0,
	    const int tilt_num,
	    const int vol_num,
	    const double start_range,  /* km */
	    const double gate_spacing, /* km */
	    const int n_az,
	    const int n_gates,
	    const int samples_per_beam,
	    const double nyquist_velocity,
	    const double radar_constant,
	    const double wave_length,   /* cm */
	    const double prf,
	    const bool debug = false);


  /*********************************************************************
   * Destructor
   */

  ~SweepFile();


  /*********************************************************************
   * write() - Write sweep file
   *
   * Returns true on success, false on failure
   */

  bool write(const double *beam_duration,
	     const float *azimuth,
	     const float *elevation,
	     const float *power,
	     const float *velocity,
	     const float *a, const float *b,
	     const float *avg_i, const float *avg_q,
	     const float *niq, const float *aiq,
	     const float *z,
	     const float *ncp,
	     const float *sw);


private:

  /////////////////////
  // Private members //
  /////////////////////

  bool _debug;

  string _outputDir;
  string _outPath;
  string _tmpPath;
  
  double _radarLat;
  double _radarLon;
  double _radarAlt;

  int _tiltNum;
  int _volNum;
  double _startRange;
  double _gateSpacing;
  int _nAz;
  int _nGates;
  int _nFields;
  double _samplesPerBeam;
  double _nyquistVelocity;
  double _radarConstant;
  double _waveLength;
  double _prf;

  time_t _sweepStartTime;
  

  /////////////////////
  // Private methods //
  /////////////////////

  /*********************************************************************
   * _putField() - Put the given data field into the netCDF file.
   */
  
  void _putField(Nc3File &out,
		 const string &field_name,
		 const string &field_name_long,
		 const string &units,
		 const float *data,
		 const Nc3Dim *TimeDim,
		 const Nc3Dim *maxCellsDim,
		 const float missing_data_value);


  /*********************************************************************
   * _writeTmp() - Write output to tmp file.
   *
   * Returns true on success, false on failure
   */

  bool _writeTmp(const double *beam_duration,
		 const float *azimuth,
		 const float *elevation,
		 const float *power,
		 const float *velocity,
		 const float *a, const float *b,
		 const float *avg_i, const float *avg_q,
		 const float *niq, const float *aiq,
		 const float *z,
		 const float *ncp,
		 const float *sw);


};

#endif

