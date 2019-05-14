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
/*
 *   Module: RadxDealias.hh
 *
 *   Author: Sue Dettling
 *
 *   Date:   10/5/01
 *   Updated: 02/14/19 Brenda Javornik
 *
 *   Description: Class RadxDealias handles dealiasing DsRadarBeams using
 *                the Curtis James 4DD ( Four Dimensional Dealiser ) algorithm.
 *                Data are read from a NetCDF file, reformatted into
 *                an RSL (TRMM Radar Software Library) like structures.
 *                The radar volume is processed by 4DD. ( The C functions which make up
 *                the 4DD algorithm have been put in the FourDD class).
 *                Finally, the volume is written to a file in NetCDF format.
 *
 */

#ifndef RADXD_HH
#define RADXD_HH

#include <string>
#include <vector>
#include <sys/stat.h>
#include "Args.hh"
#include "Params.hh"
#include <Radx/RadxVol.hh>
#include "Rsl.hh"
#include "FourDD.hh"
using namespace std;

class RadxDealias {

public:

  RadxDealias (int argc, char **argv);

  ~RadxDealias();

  int Run();

  bool isOK;

private:

  int _run(vector<string> fileList);

  vector<string> _useCommandLineStartEndTimes();
  vector<string> _useCommandLineFileList();
  void _readFile(const string &filePath, RadxVol &vol);

  int _processOne(string filePath);
  int _runWithCompleteFileList(vector<string> fileList);
  int _runRealtimeWithLdata();
  int _runRealtimeNoLdata();

  bool tdrp_bool_t_to_bool(tdrp_bool_t value);

  //
  // Dealias _currRadarVol if possible
  //
  void _processVol(Volume *_prevVelVol, Volume *_currVelVol,
		   Volume *currDbzVol, time_t volTime);
  //
  // Write beams in _currRadarVol to fmq
  //
  void _writeVol(RadxVol &vol);

  void statusReport(int nError, int nGood);


  //
  // Reset the Dsr2Radar _currRadarVol and _prevRadarVol in
  // preparation for start of new volume.
  //
  void _reset();

  inline bool file_exists (const std::string& name) {
    struct stat buffer;
    return (stat (name.c_str(), &buffer) == 0);
  }

  //
  // Data members
  //
  string _progName;

  char *_paramsPath;

  Args _args;

  Params _params;

  Volume *_extractFieldData(const RadxVol &radxVol, string fieldName);
  Volume *_extractVelocityFieldData(const RadxVol &radxVol, string velocityFieldName,
				    float override_nyquist_vel,
				    bool override_missing_field_values,
				    float velocity_field_missing_value);

  void _insertFieldData(RadxVol *radxVol, string fieldName, Volume *volume);


  //
  // Dealiser methods
  //
  FourDD    *_fourDD;

  //
  // Copyright inforamtion for the 4DD algorithm
  //
  void jamesCopyright();

};

#endif




