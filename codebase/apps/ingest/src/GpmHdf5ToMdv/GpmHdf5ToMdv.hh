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
// GpmHdf5ToMdv.hh
//
// Mike Dixon, EOL, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// May 2021
//
///////////////////////////////////////////////////////////////
//
// GpmHdf5ToMdv reads GPM data in HDF5 format, and
// converts to MDV.
// Originally based on NcGeneric2Mdv.
//
////////////////////////////////////////////////////////////////

#ifndef GpmHdf5ToMdv_H
#define GpmHdf5ToMdv_H

#include <string>
#include <toolsa/TaArray.hh>
#include <toolsa/DateTime.hh>
#include <didss/DsInputPath.hh>
#include <Mdv/DsMdvx.hh>
#include <Mdv/MdvxProj.hh>
#include <Mdv/MdvxRemapLut.hh>
#include <Ncxx/Hdf5xx.hh>
#include <euclid/point.h>
#include "Args.hh"
#include "Params.hh"
using namespace std;

////////////////////////
// This class

class GpmHdf5ToMdv {
  
public:

  // constructor

  GpmHdf5ToMdv (int argc, char **argv);

  // destructor
  
  ~GpmHdf5ToMdv();

  // run 

  int Run();

  // data members

  bool isOK;

protected:
  
private:

  static const fl32 _missingFloat;

  string _progName;
  char *_paramsPath;
  Args _args;
  Params _params;
  DsInputPath *_input;

  // data set members

  vector<DateTime> _times;
  DateTime _startTime, _endTime;
  size_t _nScans, _nRays, _nGates;

  string _fileHeader, _fileInfo;
  string _inputRecord, _jaxaInfo;
  string _navigationRecord, _history;

  vector<NcxxPort::si32> _dataQuality, _dataWarning;
  vector<NcxxPort::si32> _geoError, _geoWarning;
  vector<NcxxPort::si32> _limitErrorFlag, _missingScan;
  
  vector<NcxxPort::fl64> _lats;
  vector<NcxxPort::fl64> _lons;
  NcxxPort::fl64 _missingLat;
  NcxxPort::fl64 _missingLon;

  double _minLat, _maxLat;
  double _minLon, _maxLon;
  vector<vector<Point_d> > _latLons;
  
  vector<NcxxPort::fl64> _scLat, _scLon, _scAlt;

  vector<NcxxPort::fl32> _dbzInput;
  vector<NcxxPort::fl32> _dbzInterp;
  vector<NcxxPort::fl32> _dbzOutput;
  NcxxPort::fl32 _missingDbz;
  string _dbzUnits;

  bool _qualAvailable;
  vector<NcxxPort::si16> _qualInput;
  vector<NcxxPort::si16> _qualInterp;
  NcxxPort::si16 _missingQual;
  string _qualUnits;

  size_t _nx, _ny, _nz;
  double _minxDeg, _minyDeg, _minzKm;
  double _maxxDeg, _maxyDeg, _maxzKm;
  double _dxDeg, _dyDeg, _dzKm;
  vector<double> _zLevels;

  // hdf5 utilities

  Hdf5xx _utils;

  // private methods

  int _processFile(const char *input_path);

  string _readStringAttribute(Group &group,
                              const string &attrName,
                              const string &context);
  
  int _readMetadata(Group &ns);
  int _readTimes(Group &ns);
  int _readLatLon(Group &ns);
  int _readSpaceCraftPos(Group &ns);
  int _readQcFlags(Group &ns);

  int _readFields(Group &ns);

  int _readField3D(Group &ns,
                   const string &groupName,
                   const string &fieldName,
                   vector<NcxxPort::fl32> &vals,
                   NcxxPort::fl32 &missingVal,
                   string &units);
  
  int _readField3D(Group &ns,
                   const string &groupName,
                   const string &fieldName,
                   vector<NcxxPort::si32> &vals,
                   NcxxPort::si32 &missingVal,
                   string &units);
  
  int _readField2D(Group &ns,
                   const string &groupName,
                   const string &fieldName,
                   vector<NcxxPort::fl32> &vals,
                   NcxxPort::fl32 &missingVal,
                   string &units);
  
  int _readField2D(Group &ns,
                   const string &groupName,
                   const string &fieldName,
                   vector<NcxxPort::si32> &vals,
                   NcxxPort::si32 &missingVal,
                   string &units);
  
  int _readField2D(Group &ns,
                   const string &groupName,
                   const string &fieldName,
                   vector<NcxxPort::si16> &vals,
                   NcxxPort::si16 &missingVal,
                   string &units);
  
  // interpolation

  void _interpField(vector<NcxxPort::fl32> &valsInput,
                    NcxxPort::fl32 missingVal,
                    vector<NcxxPort::fl32> &valsInterp,
                    bool nearestNeighbor);
  
  void _interpField(vector<NcxxPort::si32> &valsInput,
                    NcxxPort::si32 missingVal,
                    vector<NcxxPort::si32> &valsInterp);
  
  void _interpField(vector<NcxxPort::si16> &valsInput,
                    NcxxPort::si16 missingVal,
                    vector<NcxxPort::si16> &valsInterp);
  
  void _interpInsidePolygon(const Point_d *corners,
                            const NcxxPort::fl32 *vals,
                            NcxxPort::fl32 missingVal,
                            size_t iz,
                            vector<NcxxPort::fl32> &valsInterp,
                            bool nearestNeighbor);

  void _interpInsidePolygon(const Point_d *corners,
                            const NcxxPort::si32 *vals,
                            NcxxPort::si32 missingVal,
                            size_t iz,
                            vector<NcxxPort::si32> &valsInterp);

  void _interpInsidePolygon(const Point_d *corners,
                            const NcxxPort::si16 *vals,
                            NcxxPort::si16 missingVal,
                            size_t iz,
                            vector<NcxxPort::si16> &valsInterp);

  void _computeMinMaxIndices(const Point_d *corners,
                             int &minIx, int &maxIx,
                             int &minIy, int &maxIy);

  NcxxPort::fl32 _interpPt(const Point_d &pt,
                           const Point_d *corners,
                           const NcxxPort::fl32 *vals,
                           NcxxPort::fl32 missingVal,
                           bool nearestNeighbor);
  
  NcxxPort::si32 _interpPt(const Point_d &pt,
                           const Point_d *corners,
                           const NcxxPort::si32 *vals,
                           NcxxPort::si32 missingVal);

  NcxxPort::si32 _interpPt(const Point_d &pt,
                           const Point_d *corners,
                           const NcxxPort::si16 *vals,
                           NcxxPort::si32 missingVal);

  Point_d _getCornerLatLon(int iscan,
                           int iray,
                           double zM);

  Point_d _getCornerLatLon(int iscan,
                           int iray);

  // invert the height levels for DBZ because the data is stored
  // with the top first and decreasing in height 

  void _invertDbzGateLevels();

  // remap the gates onto specified vertical levels
  // compute the max for the remapping

  void _remapVertLevels();

  /// set MDV headers and data

  int _setMasterHeader(DsMdvx &mdvx);
  
  MdvxField *_createMdvxField(const string &fieldName,
                              const string &longName,
                              const string &units,
                              size_t nx, size_t ny, size_t nz,
                              double minx, double miny, double minz,
                              double dx, double dy, double dz,
                              NcxxPort::fl32 missingVal,
                              NcxxPort::fl32 *vals);


  MdvxField *_createMdvxField(const string &fieldName,
                              const string &longName,
                              const string &units,
                              size_t nx, size_t ny, size_t nz,
                              double minx, double miny, double minz,
                              double dx, double dy, double dz,
                              NcxxPort::si16 missingVal,
                              NcxxPort::si16 *vals);


};

#endif

