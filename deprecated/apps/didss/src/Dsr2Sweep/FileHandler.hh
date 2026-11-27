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
/////////////////////////////////////////////////////////////
// FileHandler - Base class for classes that handle the sweep files.
//
// Nancy Rehak, RAP, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// April 2006
//
///////////////////////////////////////////////////////////////

#ifndef FileHandler_H
#define FileHandler_H

#include <vector>

#include <RayFile.h>
#include <RaycTime.h>

#include <rapformats/DsRadarMsg.hh>

#include "EosDetector.hh"
#include "ScanStrategyEosDetector.hh"
#include "SweepNumEosDetector.hh"

#include "FieldInfo.hh"
#include "Beam.hh"
#include "Params.hh"
class DsRadarParams;

using namespace std;


class FileHandler
{
  
public:

  /*********************************************************************
   * Constructor
   */
  
  FileHandler(const Params &params);

  /*********************************************************************
   * Destructor
   */
  
  virtual ~FileHandler();


  ////////////////////
  // Access methods //
  ////////////////////

  /*********************************************************************
   * addField() - Add a field to the list of fields to process
   */
  
  void addField(const FieldInfo &field_info)
  {
    _fields.push_back(field_info);
  }
  

  ////////////////////////
  // Processing methods //
  ////////////////////////

  /*********************************************************************
   * processMsg() - Process the given radar message.
   */
  
  virtual void processMsg(DsRadarMsg &radar_msg,
			  const int contents);


  /*********************************************************************
   * generateFileName() - Generate the appropriate name for the sweep file.
   */
  
  virtual string generateFileName(const ForayUtility::RaycTime &start_time,
				  const int scan_type,
				  const double fixed_angle,
				  const int volume_number,
				  const int sweep_number) = 0;


protected:
  
  /////////////////////////
  // Protected constants //
  /////////////////////////

  static const int BAD_DATA_VALUE;
  

  ///////////////////////
  // Protected members //
  ///////////////////////

  const Params &_params;

  bool _debug;
  bool _verbose;
  int _maxNbeamsInFile;

  string _outputDir;
  string _dataType;
  bool _writeToDatedDir;
  bool _writeLatestDataInfoFile;
  
  string _projectName;
  string _producerName;
  
  int _tiltNum;
  int _volNum;
  
  vector< FieldInfo > _fields;
  
  int _minBeamsInSweep;
  
  bool _constrainElevation;
  double _minElevation;
  double _maxElevation;

  EosDetector *_eosDetector;
  
  DsRadarParams _radarParams;
  bool _radarParamsSet;
  vector< Beam* > _beams;
  double _nyquist;

  DsRadarCalib _radarCalib;
  bool _radarCalibSet;
  
  // Data override information

  bool _overrideRadarLocationFlag;
  double _radarLatOverride;
  double _radarLonOverride;
  double _radarAltOverride;
  
  bool _overrideBeamWidthFlag;
  double _beamWidthOverride;
  
  bool _overrideNyquistFlag;
  double _nyquistOverride;
  
  bool _useTargetElev;
  bool _removeTestPulse;
  int _ngatesTestPulse;

  bool _checkAntennaMoving;
  double _minAngleChange;
  double _prevElev;
  double _prevAz;
  
  ///////////////////////
  // Protected methods //
  ///////////////////////

  /*********************************************************************
   * _addFieldData() - Add the field data to the given beam
   */

  void _addFieldData(Beam &beam, const DsRadarMsg &radar_msg);
  

  /*********************************************************************
   * _buildHeader() - Build the ray file header.
   */

  void _buildHeader(RayFile &ray_file,
		    const ForayUtility::RaycTime &startTime,
		    const ForayUtility::RaycTime &endTime,
		    const double target_angle,
		    const int vol_num, const int tilt_num);
  

  /*********************************************************************
   * _clearBeams() - Clear the beams from the beam queue
   */

  inline void _clearBeams()
  {
    vector< Beam* >::iterator beam;
    for (beam = _beams.begin(); beam != _beams.end(); ++beam)
      delete *beam;
    
    _beams.clear();
  }


  /*********************************************************************
   * _createFileObj() - Create the appropriate file object.
   */

  virtual RayFile *_createFileObj() = 0;


  /*********************************************************************
   * _getForayPolarization() - Convert the given DsRadar polarization to
   *                           a foray polarization.
   */

  static int _getForayPolarization(const int ds_radar_polar);
  

  /*********************************************************************
   * _getForayScanMode() - Convert the given DsRadar scan mode to a foray
   *                       scan mode.
   */

  static int _getForayScanMode(const int ds_radar_scan_mode);
  

  /*********************************************************************
   * _loadFieldParams() - load field params
   */

  void _loadFieldParams(const DsRadarMsg &radar_msg);
  

  /*********************************************************************
   * _loadRadarParams() - load radar params
   */

  void _loadRadarParams(const DsRadarMsg &radar_msg);
  
  /*********************************************************************
   * _loadRadarCalib() - load radar calib
   */

  void _loadRadarCalib(const DsRadarMsg &radar_msg);
  
  /*********************************************************************
   * _writeFile() - Write the current file to disk.
   */

  void _writeFile();
  

  /*********************************************************************
   * _radarParamsChanged() -  check if radar params have changed
   *
   * returns true if params have changed
   */
  
  bool _radarParamsChanged(const DsRadarParams &oldParams,
                           const DsRadarParams &newParams);

  ////////////////////////////////////////////////////////////////
  // is antenna moving?

  bool _isAntennaMoving(const DsBeamHdr_t *beamHdr);

  ////////////////////////////////////////////////////////////////
  // compute mean angles
  
  double _computeMeanEl();
  double _computeMeanAz();

  ////////////////////////////////////////////////////////////////
  // get target angles
  
  double _getTargetEl();
  double _getTargetAz();

};

#endif

