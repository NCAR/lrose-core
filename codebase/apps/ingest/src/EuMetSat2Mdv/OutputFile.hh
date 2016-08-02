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
// OutputFile.hh
//
// Class for output file.
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Oct 2005
//
/////////////////////////////////////////////////////////////

#ifndef OutputFile_HH
#define OutputFile_HH

#include <iostream>
#include <string>
#include <map>
#include <Mdv/DsMdvx.hh>
#include <Mdv/MdvxProj.hh>
#include "ChannelSet.hh"
#include "FieldSet.hh"
#include "ChannelFiles.hh"
#include "Params.hh"
#include "Hrit.hh"
#include "SunAngle.hh"

using namespace std;

class OutputFile {
  
public:

  // constructor
  
  OutputFile(const string &prog_name,
             const Params &params,
             const Params::output_file_t &output_file,
             ChannelSet &channelSet,
             const FieldSet &fieldSet);
  
  // Destructor
  
  virtual ~OutputFile();
  
  // did the object construct OK

  bool OK() { return  _OK; }

  // process an input file
  // returns 0 on success, -1 if already in the set
  
  int processFile(const string &path,
                  const Hrit &hrit);

  // write the file set to an output file
  // Returns 0 on success, -1 on failure

  int write();

  // Check to see whether all sets are complete
  // Returns true if all file sets are complete
  
  bool complete();

private:

  typedef map<int, const ChannelFiles*, less<int> > ChannelFilesMap;
  typedef pair<int, const ChannelFiles* > ChannelFilesEntry;

  bool _OK;

  const string &_progName;
  const Params &_params;

  vector<const Field *> _fields;

  ChannelSet &_channelSet;
  vector<ChannelFiles *> _channelFiles;
  ChannelFilesMap _channelFilesMap;

  time_t _dataStartTime; // time of start of data in sat files
  time_t _dataEndTime; // time of latest data found in file

  double _sensorLon;

  string _outputUrl;

  MdvxProj _proj;
  Mdvx::projection_type_t _projection;
  double _originLat;
  double _originLon;
  double _lambertLat1;
  double _lambertLat2;
  int _nx;
  int _ny;
  double _minx;
  double _miny;
  double _dx;
  double _dy;
  int _nPointsGrid;
  
  double *_xLookup, *_yLookup;
  fl32 *_lat, *_lon;
  bool _needSunAngle;

  SunAngle _sunAngle;
  
  void _clear();
  void _setUpProj();
  void _computeLookups();
  void _computeSunAngle(time_t data_time);
  void _prepareForAdd(const Hrit &hrit);
  void _handleAdd(const Hrit &hrit);

};

#endif



