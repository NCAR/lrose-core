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
 *   $Author: jcraig $
 *   $Locker:  $
 *   $Date: 2018/01/26 20:33:40 $
 *   $Id: NexradA1ToRefract.hh,v 1.7 2018/01/26 20:33:40 jcraig Exp $
 *   $Revision: 1.7 $
 *   $State: Exp $
 */
 
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/

/************************************************************************
 * NexradA1ToRefract: NexradA1ToRefract program object.
 *
 * RAP, NCAR, Boulder CO
 *
 * May 2005
 *
 * Nancy Rehak
 *
 ************************************************************************/

#ifndef NexradA1ToRefract_HH
#define NexradA1ToRefract_HH

#include <string>
#include <sys/time.h>
#include <vector>

#include <Ncxx/Nc3File.hh>
#include <dsdata/DsTrigger.hh>

#include "Args.hh"
#include "Params.hh"

using namespace std;

class NexradA1ToRefract
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

  ~NexradA1ToRefract(void);
  

  /*********************************************************************
   * Inst() - Retrieve the singleton instance of this class.
   */

  static NexradA1ToRefract *Inst(int argc, char **argv);
  static NexradA1ToRefract *Inst();
  

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

  // Names of the dimension fields in the netCDF file.

  static const string TIME_DIM_NAME;
  static const string GATES_DIM_NAME;

  // Names of the variable fields in the netCDF file.

  static const string I_VAR_NAME;
  static const string Q_VAR_NAME;
  static const string SAMPLE_NUM_VAR_NAME;
  static const string AZIMUTH_VAR_NAME;
  static const string ELEVATION_VAR_NAME;
  static const string PRT_VAR_NAME;
  static const string TIME_VAR_NAME;
  static const string UNIX_TIME_VAR_NAME;
  static const string NANO_SEC_VAR_NAME;
  static const string FLAGS_VAR_NAME;

  // Constant radar values

  static const double RADAR_LAT;
  static const double RADAR_LON;
  static const double RADAR_ALT;

  static const int NUM_AZIMUTHS;
  static const double WAVELENGTH;
  static const int SAMPLES_PER_BEAM;
  static const double RADAR_CONSTANT;
  static const double POWER_CONSTANT;
  static const double START_RANGE;
  static const double GATE_SPACING;


  /////////////////////
  // Private members //
  /////////////////////

  // Singleton instance pointer

  static NexradA1ToRefract *_instance;
  
  // Program parameters.

  char *_progName;
  Args *_args;
  Params *_params;
  
  // Data trigger object

  DsTrigger* _dataTrigger;
  
  // Arrays of calculated values

  int _numGates;

  float *_azimuth;
  float *_elevation;
  double *_beamDuration;    /* in fractions of a second */

  float *_power;
  float *_velocity;
  float *_velA;
  float *_velB;
  float *_avgI;
  float *_avgQ;
  float *_niq;
  float *_aiq;
  float *_z;
  float *_ncp;
  float *_sw;

  // Other sweep information

  bool _waitingForSweepStart;
  int _sweepStartAzimuth;
  time_t _sweepStartTime;
  time_t _sweepEndTime;
  int _sweepPrt;
  time_t _prevSweepEndTime;

  // Information about the current beam

  int _currOutputAzimuth;
  time_t _beamStartTime;
  int _beamStartNanoseconds;
  time_t _beamEndTime;
  int _beamEndNanoseconds;
  double _elevationTotal;
  int _numSamples;
  double _prf;
  double _nyquistVel;
  
  vector< float > _iValuesBeam;
  vector< float > _qValuesBeam;


  /////////////////////
  // Private methods //
  /////////////////////

  /*********************************************************************
   * Constructor -- private because this is a singleton object
   */

  NexradA1ToRefract(int argc, char **argv);
  

  /*********************************************************************
   * _extractValues() - Extract the values for the specified variable
   *                    from the netCDF file.
   *
   * Returns a pointer to the values on success, 0 on failure.
   */

  Nc3Values *_extractValues(const string &var_name,
			   Nc3File &input_file,
			   const string &input_file_path) const;


  /*********************************************************************
   * _initializeArrays() - Initialize the arrays of calculated fields.
   */

  void _initializeArrays(const int num_gates);


  /*********************************************************************
   * _processFile() - Process the given input file
   */

  bool _processFile(const string &input_file_name);


  /*********************************************************************
   * _processBeam() - Process the given data, all of which fall within
   *                  the same beam.
   */

  void _processBeam(const vector< float > &i_values_beam,
		    const vector< float > &q_values_beam,
		    const size_t num_gates,
		    const int azimuth, const float elevation,
		    const double prf,
		    const double beam_duration);

  /*********************************************************************
   * _writeSweepFile() - Write the saved data to a sweep file.
   */

  bool _writeSweepFile();


};


#endif
